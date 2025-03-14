#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include "../src/hmm.h"

#define NUM_THREADS 4
#define ALLOCS_PER_THREAD 100
#define MAX_ALLOC_SIZE 1024

typedef struct {
    int thread_id;
    int success_count;
    int failure_count;
} ThreadData;

void* thread_work(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    void* ptrs[ALLOCS_PER_THREAD] = {NULL};
    size_t sizes[ALLOCS_PER_THREAD] = {0};
    
    printf("Thread %d starting work\n", data->thread_id);
    
    // Allocate memory
    for (int i = 0; i < ALLOCS_PER_THREAD; i++) {
        sizes[i] = rand() % MAX_ALLOC_SIZE + 1;
        ptrs[i] = hmmAlloc(sizes[i]);
        
        if (ptrs[i] != NULL) {
            // Write to memory to ensure it's usable
            memset(ptrs[i], data->thread_id, sizes[i]);
            data->success_count++;
        } else {
            data->failure_count++;
            printf("Thread %d: Allocation %d failed\n", data->thread_id, i);
        }
    }
    
    // Free half of the allocations
    for (int i = 0; i < ALLOCS_PER_THREAD / 2; i++) {
        if (ptrs[i] != NULL) {
            hmmFree(ptrs[i]);
            ptrs[i] = NULL;
        }
    }
    
    // Allocate memory again
    for (int i = 0; i < ALLOCS_PER_THREAD / 2; i++) {
        sizes[i] = rand() % MAX_ALLOC_SIZE + 1;
        ptrs[i] = hmmAlloc(sizes[i]);
        
        if (ptrs[i] != NULL) {
            // Write to memory to ensure it's usable
            memset(ptrs[i], data->thread_id, sizes[i]);
            data->success_count++;
        } else {
            data->failure_count++;
            printf("Thread %d: Allocation %d failed (second round)\n", data->thread_id, i);
        }
    }
    
    // Free all remaining allocations
    for (int i = 0; i < ALLOCS_PER_THREAD; i++) {
        if (ptrs[i] != NULL) {
            hmmFree(ptrs[i]);
            ptrs[i] = NULL;
        }
    }
    
    printf("Thread %d completed: %d successful allocations, %d failures\n", 
           data->thread_id, data->success_count, data->failure_count);
    
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];
    ThreadData thread_data[NUM_THREADS];
    int total_success = 0;
    int total_failure = 0;
    
    printf("Starting multithreaded test with %d threads\n", NUM_THREADS);
    
    // Initialize random number generator
    srand(time(NULL));
    
    // Initialize thread data
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].success_count = 0;
        thread_data[i].failure_count = 0;
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
    }
    
    printf("\nTest complete!\n");
    printf("Total successful allocations: %d\n", total_success);
    printf("Total allocation failures: %d\n", total_failure);
    
    // Basic assertion to verify the test was successful
    assert(total_success > 0);
    assert(total_failure == 0);
    
    if (total_failure == 0) {
        printf("MULTITHREADED TEST PASSED\n");
        return 0;
    } else {
        printf("MULTITHREADED TEST FAILED\n");
        return 1;
    }
} 