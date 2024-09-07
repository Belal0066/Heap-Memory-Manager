#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../src/hmm.h"

#define NUM_ALLOCS 10000
#define MAX_SIZE 10240
#define MAX_ITERATIONS 1000000

void random_alloc_free_test() {
    srand((unsigned int)time(NULL));
    
    void* pointers[NUM_ALLOCS] = {NULL};
    
    for (int i = 0; i < MAX_ITERATIONS; ++i) {
        int index = rand() % NUM_ALLOCS;
        if (pointers[index] == NULL) {
            // Allocate memory
            size_t size = (size_t)(rand() % MAX_SIZE) + 1;
            pointers[index] = hmmAlloc(size);
            if (pointers[index] != NULL) {
                printf("Allocated memory of size %zu at address %p\n", size, pointers[index]);
            } else {
                fprintf(stderr, "Allocation failed for size %zu\n", size);
            }
        } else {
            // Free memory
            printf("Freeing memory at address %p\n", pointers[index]);
            hmmFree(pointers[index]);
            pointers[index] = NULL;
        }
    }
    
    // Free remaining allocated memory
    void* beforeFree = get_program_break();

    printf("\nProgram break before freeing: %p\n", beforeFree);
    for (int i = 0; i < NUM_ALLOCS; ++i) {
        if (pointers[i] != NULL) {
            hmmFree(pointers[i]);
            pointers[i] = NULL;
        }
    }

    // printf("Freeing remaining memory at address %p\n", get_program_break());
    void* afterFree = get_program_break();
    printf("Program break after freeing: %p\n", afterFree);
    long kb = ((char*)beforeFree - (char*)afterFree)/1024;
    printf("Difference: %ld.%ld MB.\n", kb/1024, kb%1024);
}

int main() {
    printf("Starting random allocation and deallocation test...\n");
    random_alloc_free_test();
    printf("Test complete.\n");
    return 0;
}
