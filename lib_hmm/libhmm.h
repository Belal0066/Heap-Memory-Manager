#ifndef LIB_HMM_H
#define LIB_HMM_H

#include <stddef.h> 
#include <unistd.h> 
#include <string.h> 
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

//Config
#define ALIGNMENT 8
#define PAGE_SIZE 4096
// #define LOG_MODE



typedef struct BlockHeader
{
    size_t size;
    int is_free;
    struct BlockHeader *next;
    struct BlockHeader *prev;
} BlockHeader;


static BlockHeader *free_list_head = NULL;


void *hmmAlloc(size_t size);
void hmmFree(void *ptr);


void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

#endif 
