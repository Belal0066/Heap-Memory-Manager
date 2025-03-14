#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include "../src/hmm.h"

#define NUM_THREADS 4
#define ALLOCS_PER_THREAD 100
#define MAX_BLOCK_SIZE 1024

// Structure to pass data to threads
typedef struct {
    int thread_id;
    int success_count;
    int error_count;
} ThreadData;

// Thread function
void* thread_allocate_free(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    void* ptrs[ALLOCS_PER_THREAD];
    size_t sizes[ALLOCS_PER_THREAD];
    
    printf("Thread %d starting allocations\n", data->thread_id);
    
    // Allocate memory
    for (int i = 0; i < ALLOCS_PER_THREAD; i++) {
        // Generate a random size between 1 and MAX_BLOCK_SIZE
        sizes[i] = rand() % MAX_BLOCK_SIZE + 1;
        
        // Use thread-safe allocation
        ptrs[i] = hmmAlloc_mt(sizes[i]);
        
        if (ptrs[i] != NULL) {
            data->success_count++;
            
            // Write to the allocated memory to ensure it's usable
            memset(ptrs[i], data->thread_id, sizes[i]);
        } else {
            data->error_count++;
            printf("Thread %d: Allocation failed for size %zu\n", 
                  data->thread_id, sizes[i]);
        }
    }
    
    // Free half of the allocations
    for (int i = 0; i < ALLOCS_PER_THREAD / 2; i++) {
        if (ptrs[i] != NULL) {
            hmmFree_mt(ptrs[i]);
            ptrs[i] = NULL;
        }
    }
    
    // Allocate some more memory
    for (int i = 0; i < ALLOCS_PER_THREAD / 2; i++) {
        sizes[i] = rand() % MAX_BLOCK_SIZE + 1;
        ptrs[i] = hmmAlloc_mt(sizes[i]);
        
        if (ptrs[i] != NULL) {
            data->success_count++;
            memset(ptrs[i], data->thread_id, sizes[i]);
        } else {
            data->error_count++;
        }
    }
    
    // Free all remaining allocations
    for (int i = 0; i < ALLOCS_PER_THREAD; i++) {
        if (ptrs[i] != NULL) {
            hmmFree_mt(ptrs[i]);
            ptrs[i] = NULL;
        }
    }
    
    printf("Thread %d completed: %d successes, %d errors\n", 
          data->thread_id, data->success_count, data->error_count);
    
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    int total_success = 0;
    int total_errors = 0;
    
    printf("Starting multithreaded test with %d threads\n", NUM_THREADS);
    
    // Initialize random seed
    srand(time(NULL));
    
    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].success_count = 0;
        thread_data[i].error_count = 0;
        
        if (pthread_create(&threads[i], NULL, thread_allocate_free, &thread_data[i]) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        total_success += thread_data[i].success_count;
        total_errors += thread_data[i].error_count;
    }
    
    printf("\nMultithreaded test complete\n");
    printf("Total successful allocations: %d\n", total_success);
    printf("Total allocation errors: %d\n", total_errors);
    
    // Basic test assertion
    assert(total_errors == 0 && "All memory allocations should succeed");
    
    printf("Multithreaded test PASSED\n");
    return 0;
} 