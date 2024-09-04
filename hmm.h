#ifndef HMM_H
#define HMM_H

#include <stddef.h> 
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


#define MEMORY_SIZE 1024*1024*100  // 100 MB 
#define ALIGNMENT 8


typedef struct BlockHeader {
    size_t size;
    bool is_free;
    struct BlockHeader* next;
    struct BlockHeader* prev;
} BlockHeader;




void* hmmAlloc(size_t size);

void hmmFree(void* ptr);

void* get_program_break(void);


void print_heap_state();



#endif // HMM_H
