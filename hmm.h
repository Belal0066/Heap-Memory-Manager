//HHM.h
#ifndef HMM_h
#define HMM_h

#include<stddef.h> 
#include <stdio.h>

#define HEAP_SIZE 1024
#define ALIGN_SIZE 8


struct BlkHdr{
	int free; 
	size_t size; 
	struct BlkHdr *nxt; 
};



static unsigned int hpArr[HEAP_SIZE];
static size_t hpBrk = 0; 



static struct BlkHdr* freeList = NULL;
int ceil2nALIGN_SIZE(size_t memSize);

void* hmmAlloc(size_t size);
void hmmFree(void* ptr);

#endif
