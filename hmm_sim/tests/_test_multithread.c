#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "../src/hmm.h"

#define NUM_THREADS 16
#define BASE_ALLOCS_PER_THREAD 200
#define MAX_ALLOC_SIZE 8192
#define MIN_ALLOC_SIZE 8
#define STRESS_ITERATIONS 10
#define VERIFY_PATTERN 1  // Enable memory pattern verification

// Different thread roles to simulate various workloads
typedef enum {
    ROLE_BALANCED,       // Equal alloc/dealloc
    ROLE_ALLOC_HEAVY,    // More allocs than deallocs
    ROLE_DEALLOC_HEAVY,  // More deallocs than allocs
    ROLE_BURSTY,         // Burst of allocs followed by deallocs
    ROLE_LONG_LIVED,     // Allocates and holds for a long time
    ROLE_TINY_ALLOCS,    // Many tiny allocations
    ROLE_LARGE_ALLOCS,   // Few large allocations
    ROLE_RANDOM          // Completely random behavior
} ThreadRole;

typedef struct {
    int thread_id;
    ThreadRole role;
    int success_count;
    int failure_count;
    size_t bytes_allocated;
    int allocs_per_thread;
} ThreadData;

// Memory block info to track allocations
typedef struct {
    void* ptr;
    size_t size;
    int pattern;
    int is_allocated;
} MemoryBlock;

// Global test tracking
int global_allocs = 0;
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

// Fill memory with a pattern based on thread ID and block index
void fill_memory(void* ptr, size_t size, int pattern) {
    if (!VERIFY_PATTERN) return;
    
    unsigned char* mem = (unsigned char*)ptr;
    for (size_t i = 0; i < size; i++) {
        mem[i] = (pattern + i) % 256;
    }
}

// Verify memory still contains the expected pattern
int verify_memory(void* ptr, size_t size, int pattern) {
    if (!VERIFY_PATTERN) return 1;
    
    unsigned char* mem = (unsigned char*)ptr;
    for (size_t i = 0; i < size; i++) {
        if (mem[i] != (unsigned char)((pattern + i) % 256)) {
            printf("Memory corruption detected at offset %zu. Expected %d, found %d\n",
                   i, (pattern + i) % 256, mem[i]);
            return 0;
        }
    }
    return 1;
}

// Determine allocation size based on thread role and random factors
size_t get_allocation_size(ThreadRole role) {
    switch (role) {
        case ROLE_TINY_ALLOCS:
            return (rand() % 64) + MIN_ALLOC_SIZE;
        case ROLE_LARGE_ALLOCS:
            return (rand() % (MAX_ALLOC_SIZE - 1024)) + 1024;
        case ROLE_BALANCED:
        case ROLE_ALLOC_HEAVY:
        case ROLE_DEALLOC_HEAVY:
        case ROLE_BURSTY:
        case ROLE_LONG_LIVED:
        case ROLE_RANDOM:
        default:
            // Full range
            return (rand() % (MAX_ALLOC_SIZE - MIN_ALLOC_SIZE)) + MIN_ALLOC_SIZE;
    }
}

// Thread worker function
void* thread_work(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    MemoryBlock* blocks = calloc(data->allocs_per_thread, sizeof(MemoryBlock));
    int allocated_count = 0;
    
    // Random seed per thread
    srand(time(NULL) + data->thread_id);
    
    printf("Thread %d starting with role: %d, allocs: %d\n", 
           data->thread_id, data->role, data->allocs_per_thread);
    
    // Main allocation loop
    for (int iter = 0; iter < STRESS_ITERATIONS; iter++) {
        // Allocation phase
        for (int i = 0; i < data->allocs_per_thread; i++) {
            if (blocks[i].is_allocated) {
                // For some roles, we may want to free more aggressively
                if (data->role == ROLE_DEALLOC_HEAVY || 
                    (data->role == ROLE_RANDOM && (rand() % 3 == 0)) ||
                    (data->role == ROLE_BURSTY && iter % 2 == 1)) {
                    
                    // Verify memory before freeing
                    if (VERIFY_PATTERN) {
                        assert(verify_memory(blocks[i].ptr, blocks[i].size, blocks[i].pattern));
                    }
                    
                    hmmFree(blocks[i].ptr);
                    blocks[i].is_allocated = 0;
                    allocated_count--;
                }
                continue;
            }
            
            // Decide whether to allocate based on role
            if ((data->role == ROLE_ALLOC_HEAVY) ||
                (data->role == ROLE_BALANCED) ||
                (data->role == ROLE_BURSTY && iter % 2 == 0) ||
                (data->role == ROLE_LONG_LIVED && allocated_count < data->allocs_per_thread / 2) ||
                (data->role == ROLE_RANDOM && (rand() % 2 == 0))) {
                
                // Determine size based on role
                size_t size = get_allocation_size(data->role);
                
                // Allocate memory
                blocks[i].ptr = hmmAlloc(size);
                blocks[i].size = size;
                blocks[i].pattern = (data->thread_id * 1000) + i;
                
                if (blocks[i].ptr != NULL) {
                    // Fill memory with a verifiable pattern
                    fill_memory(blocks[i].ptr, size, blocks[i].pattern);
                    blocks[i].is_allocated = 1;
                    allocated_count++;
                    
                    data->success_count++;
                    data->bytes_allocated += size;
                    
                    // Update global allocation count (thread-safe)
                    pthread_mutex_lock(&stats_mutex);
                    global_allocs++;
                    pthread_mutex_unlock(&stats_mutex);
                } else {
                    data->failure_count++;
                    printf("Thread %d: Allocation %d failed for size %zu\n", 
                           data->thread_id, i, size);
                }
            }
        }
        
        // For long-lived allocations, sleep a bit to hold memory longer
        if (data->role == ROLE_LONG_LIVED) {
            usleep(50000);  // 50ms
        }
        
        // Periodic verification phase - check all allocated blocks
        if (VERIFY_PATTERN) {
            for (int i = 0; i < data->allocs_per_thread; i++) {
                if (blocks[i].is_allocated) {
                    assert(verify_memory(blocks[i].ptr, blocks[i].size, blocks[i].pattern));
                }
            }
        }
        
        // Deallocation phase - free some or all blocks based on role
        int free_count = 0;
        switch (data->role) {
            case ROLE_BALANCED:
                free_count = data->allocs_per_thread / 2;  // Free half
                break;
            case ROLE_ALLOC_HEAVY:
                free_count = data->allocs_per_thread / 4;  // Free quarter
                break;
            case ROLE_DEALLOC_HEAVY:
                free_count = data->allocs_per_thread;      // Free all
                break;
            case ROLE_BURSTY:
                free_count = (iter % 2 == 1) ? data->allocs_per_thread : 0;
                break;
            case ROLE_LONG_LIVED:
                free_count = (iter == STRESS_ITERATIONS - 1) ? data->allocs_per_thread : 0;
                break;
            case ROLE_TINY_ALLOCS:
            case ROLE_LARGE_ALLOCS:
                free_count = data->allocs_per_thread / 2;  // Free half
                break;
            case ROLE_RANDOM:
                free_count = rand() % data->allocs_per_thread;
                break;
        }
        
        // Free the determined number of blocks
        for (int i = 0, freed = 0; i < data->allocs_per_thread && freed < free_count; i++) {
            if (blocks[i].is_allocated) {
                // Verify memory before freeing
                if (VERIFY_PATTERN) {
                    assert(verify_memory(blocks[i].ptr, blocks[i].size, blocks[i].pattern));
                }
                
                hmmFree(blocks[i].ptr);
                blocks[i].is_allocated = 0;
                allocated_count--;
                freed++;
            }
        }
    }
    
    // Final cleanup - free all remaining blocks
    for (int i = 0; i < data->allocs_per_thread; i++) {
        if (blocks[i].is_allocated) {
            // Verify memory before final free
            if (VERIFY_PATTERN) {
                assert(verify_memory(blocks[i].ptr, blocks[i].size, blocks[i].pattern));
            }
            
            hmmFree(blocks[i].ptr);
            blocks[i].is_allocated = 0;
        }
    }
    
    free(blocks);
    
    printf("Thread %d completed: %d successful, %d failed, %zu bytes allocated\n",
           data->thread_id, data->success_count, data->failure_count, data->bytes_allocated);
    
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    int total_success = 0;
    int total_failure = 0;
    size_t total_bytes = 0;
    void* initial_break;
    void* final_break;
    
    // Record initial heap state
    initial_break = get_program_break();
    printf("Starting multithreaded test with %d threads\n", NUM_THREADS);
    printf("Initial program break: %p\n", initial_break);
    
    // Initialize random number generator
    srand(time(NULL));
    
    // Initialize thread data with different roles and allocation counts
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].success_count = 0;
        thread_data[i].failure_count = 0;
        thread_data[i].bytes_allocated = 0;
        
        // Assign different roles to threads
        switch (i % 8) {
            case 0: thread_data[i].role = ROLE_BALANCED; break;
            case 1: thread_data[i].role = ROLE_ALLOC_HEAVY; break;
            case 2: thread_data[i].role = ROLE_DEALLOC_HEAVY; break;
            case 3: thread_data[i].role = ROLE_BURSTY; break;
            case 4: thread_data[i].role = ROLE_LONG_LIVED; break;
            case 5: thread_data[i].role = ROLE_TINY_ALLOCS; break;
            case 6: thread_data[i].role = ROLE_LARGE_ALLOCS; break;
            case 7: thread_data[i].role = ROLE_RANDOM; break;
        }
        
        // Vary the number of allocations based on role
        switch (thread_data[i].role) {
            case ROLE_TINY_ALLOCS:
                thread_data[i].allocs_per_thread = BASE_ALLOCS_PER_THREAD * 2;
                break;
            case ROLE_LARGE_ALLOCS:
                thread_data[i].allocs_per_thread = BASE_ALLOCS_PER_THREAD / 2;
                break;
            default:
                thread_data[i].allocs_per_thread = BASE_ALLOCS_PER_THREAD;
                break;
        }
    }
    
    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        int result = pthread_create(&threads[i], NULL, thread_work, &thread_data[i]);
        assert(result == 0);
    }
    
    // Join threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Summarize results
    for (int i = 0; i < NUM_THREADS; i++) {
        total_success += thread_data[i].success_count;
        total_failure += thread_data[i].failure_count;
        total_bytes += thread_data[i].bytes_allocated;
    }
    
    // Check final heap state
    final_break = get_program_break();
    
    printf("\nMultithreaded test complete!\n");
    printf("Total successful allocations: %d\n", total_success);
    printf("Total allocation failures: %d\n", total_failure);
    printf("Total bytes allocated: %zu\n", total_bytes);
    printf("Global allocation count: %d\n", global_allocs);
    printf("Initial program break: %p\n", initial_break);
    printf("Final program break: %p\n", final_break);
    
    // Assertions for test validation
    assert(total_success > 0);
    
    // Check if we returned to a similar heap state (allowing some overhead)
    size_t heap_overhead = (char*)final_break - (char*)initial_break;
    printf("Heap overhead after test: %zu bytes\n", heap_overhead);
    
    // Basic pass/fail reporting
    if (total_failure == 0) {
        printf("MULTITHREADED STRESS TEST PASSED\n");
        return 0;
    } else {
        printf("MULTITHREADED STRESS TEST FAILED: %d allocation failures\n", total_failure);
        return 1;
    }
} 