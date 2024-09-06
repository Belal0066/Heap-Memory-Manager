#include "hmm.h"

// Use the implementation
int main() {
    printf("Initial state:\n");
    print_heap_state();

    void* ptr1 = hmmAlloc(100);
    printf("After allocating 100 bytes:\n");
    print_heap_state();

    void* ptr2 = hmmAlloc(200);
    printf("After allocating 200 bytes:\n");
    print_heap_state();

    hmmFree(ptr1);
    printf("After freeing the first allocation:\n");
    print_heap_state();

    void* ptr3 = hmmAlloc(50);
    printf("After allocating 50 bytes:\n");
    print_heap_state();

    hmmFree(ptr2);
    printf("After freeing the second allocation:\n");
    print_heap_state();

    hmmFree(ptr3);
    printf("After freeing the third allocation:\n");
    print_heap_state();

    return 0;
}