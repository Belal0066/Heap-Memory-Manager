// DHHM.h
#ifndef DHMM_h
#define DHMM_h

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>


#define ALIGN_SIZE 8
#define INIT_SIZE 1024 * 1024
#define MIN_BLOCK_SIZE (ceil2nALIGN_SIZE(sizeof(struct BlkHdr)) + ceil2nALIGN_SIZE(sizeof(void *)))

#define DEBUG_MODE 1

typedef struct BlkHdr
{
	size_t size;
	bool free;
	struct BlkHdr *next;
} BlkHdr;

static BlkHdr *freeList = NULL;
static size_t hpBrk = 0;
static void *heap_start = NULL;
static size_t heap_size = 0;

static size_t ceil2nALIGN_SIZE(size_t memSize);

void *hmmAlloc(size_t size);
void hmmFree(void *ptr);
void hmmInit(void);

void init_hmm_s(size_t initial_size);
void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

void hmmCleanup(void);
#endif
