#include "dhmm.h"

static size_t ceil2nALIGN_SIZE(size_t memSize)
{
    return (memSize + (ALIGN_SIZE - 1)) & ~(ALIGN_SIZE - 1);
}

void init_hmm_s(size_t initial_size)
{
    if (heap_start != NULL) return;

    initial_size = ceil2nALIGN_SIZE(initial_size);
    heap_start = sbrk(initial_size);
    if (heap_start == (void *)-1)
    {
        perror("sbrk failed to initialize heap");
        exit(EXIT_FAILURE);
    }
    heap_size = initial_size;
    hpBrk = 0;
    freeList = NULL;

    BlkHdr *initialBlock = (BlkHdr *)heap_start;
    initialBlock->free = true;
    initialBlock->size = heap_size - ceil2nALIGN_SIZE(sizeof(BlkHdr));
    initialBlock->next = NULL;
    freeList = initialBlock;

    if (DEBUG_MODE)
        printf("Heap initialized. size: %zu\n", heap_size);
}

void extend_hmm_heap(size_t additional_size)
{
    additional_size = ceil2nALIGN_SIZE(additional_size);
    void *new_brk = sbrk(additional_size);
    if (new_brk == (void *)-1)
    {
        perror("sbrk failed to extend heap");
        exit(EXIT_FAILURE);
    }

    BlkHdr *lastBlock = (BlkHdr *)((char *)heap_start + heap_size - ceil2nALIGN_SIZE(sizeof(BlkHdr)));
    if (lastBlock->free)
    {
        lastBlock->size += additional_size;
    }
    else
    {
        BlkHdr *newBlock = (BlkHdr *)((char *)heap_start + heap_size);
        newBlock->free = true;
        newBlock->size = additional_size - ceil2nALIGN_SIZE(sizeof(BlkHdr));
        newBlock->next = NULL;
        lastBlock->next = newBlock;
    }

    heap_size += additional_size;
}
static void printHeapState()
{
    if (!DEBUG_MODE)
        return;

    BlkHdr *current = (BlkHdr *)heap_start;
    printf("\n>>> Heap State <<<\n");
    while ((char *)current < (char *)heap_start + heap_size)
    {
        printf("Block at %p: size=%zu, free=%d, next=%p\n",
               (void *)current, current->size, current->free, (void *)current->next);
        current = (BlkHdr *)((char *)current + current->size);

        if (current == NULL || current->size == 0)
        {
            printf("Error: Invalid block or size detected!\n");
            break;
        }
    }
    printf(" === === === === === === \n");
}

static BlkHdr *findFreeBlock(size_t size)
{
    BlkHdr *current = freeList;
    BlkHdr *bestFit = NULL;
    size_t minSize = heap_size + 1;

    while (current != NULL)
    {
        if (current->free && current->size >= size)
        {
            if (current->size < minSize)
            {
                bestFit = current;
                minSize = current->size;
            }
        }
        current = current->next;

        if (current == freeList)
        {
            printf("Error in findFreeBlock\n");
            break;
        }
    }

    if (DEBUG_MODE)
    {
        if (bestFit)
        {
            printf("Found free block of size %zu at %p\n", bestFit->size, (void *)bestFit);
        }
        else
        {
            printf("No suitable free block found for size %zu\n", size);
        }
    }

    return bestFit;
}

static BlkHdr *allocateNewBlock(size_t size)
{
    if (hpBrk + size > heap_size)
    {
        size_t additional_size = size > 4096 ? size : 4096;
        extend_hmm_heap(additional_size);
    }

    BlkHdr *block = (BlkHdr *)((char *)heap_start + hpBrk);
    block->size = size;
    block->free = false;
    block->next = NULL;
    hpBrk += size;

    if (DEBUG_MODE)
        printf("Allocated new block of size %zu at %p\n", size, (void *)block);

    return block;
}

static void splitBlock(BlkHdr *block, size_t size)
{
    if (block->size >= size + MIN_BLOCK_SIZE)
    {
        BlkHdr *newBlock = (BlkHdr *)((char *)block + size);
        newBlock->size = block->size - size;
        newBlock->free = true;
        newBlock->next = block->next;

        block->size = size;
        block->next = newBlock;

        if (DEBUG_MODE)
            printf("Split block at %p. New free block at %p\n", (void *)block, (void *)newBlock);
    }
}

void *hmmAlloc(size_t size)
{
    if (size == 0)
        return NULL;

    if (heap_start == NULL)
    {
        init_hmm_s(INIT_SIZE);
    }

    size_t totalSize = ceil2nALIGN_SIZE(size + ceil2nALIGN_SIZE(sizeof(BlkHdr)));

    BlkHdr *block = findFreeBlock(totalSize);

    if (block == NULL)
    {
        extend_hmm_heap(totalSize > 4096 ? totalSize : 4096);
        block = findFreeBlock(totalSize);
        if (block == NULL)
        {
            fprintf(stderr, "Failed to allocate memory of size %zu\n", size);
            return NULL;
        }
    }

    block->free = false;
    splitBlock(block, totalSize);

    void *result = (void *)((char *)block + ceil2nALIGN_SIZE(sizeof(BlkHdr)));
    if (DEBUG_MODE)
    {
        printf("Allocated %zu bytes at %p\n", size, result);
        printHeapState();
    }
    return result;
}

void *malloc(size_t size)
{
    return hmmAlloc(size);
}

void hmmFree(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }

    BlkHdr *block = (BlkHdr *)((char *)ptr - ceil2nALIGN_SIZE(sizeof(BlkHdr)));

    if ((char *)block < (char *)heap_start || (char *)block >= (char *)heap_start + heap_size)
    {
        fprintf(stderr, "Error at Free %p\n", ptr);
        return;
    }

    block->free = true;

    BlkHdr *nextBlock = block->next;
    if (nextBlock && nextBlock->free)
    {
        block->size += nextBlock->size;
        block->next = nextBlock->next;
        if (DEBUG_MODE)
            printf("Coalesced block at %p with next block\n", (void *)block);
    }

    if (freeList == NULL || block < freeList)
    {
        block->next = freeList;
        freeList = block;
    }
    else
    {
        BlkHdr *current = freeList;
        BlkHdr *prev = NULL;
        while (current != NULL && current < block)
        {
            prev = current;
            current = current->next;
        }
        if (prev)
        {
            prev->next = block;
        }
        block->next = current;
    }

    if (DEBUG_MODE)
    {
        printf("Freed memory at address %p\n", ptr);
        printHeapState();
    }
}

void free(void *ptr)
{
    return hmmFree(ptr);
}

void *calloc(size_t nmemb, size_t size)
{
    size_t total_size = nmemb * size;
    void *ptr = malloc(total_size);
    if (ptr)
    {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

void *realloc(void *ptr, size_t size)
{
    if (ptr == NULL)
    {
        return malloc(size);
    }

    if (size == 0)
    {
        free(ptr);
        return NULL;
    }

    void *new_ptr = malloc(size);
    if (new_ptr)
    {

        BlkHdr *old_block = (BlkHdr *)((char *)ptr - ceil2nALIGN_SIZE(sizeof(BlkHdr)));
        size_t old_size = old_block->size - ceil2nALIGN_SIZE(sizeof(BlkHdr));
        size_t copy_size = (old_size < size) ? old_size : size;
        memcpy(new_ptr, ptr, copy_size);
        free(ptr);
    }
    return new_ptr;
}

void hmmCleanup()
{
    hpBrk = 0;
    freeList = NULL;
    heap_size = 0;
    heap_start = NULL;

    if (DEBUG_MODE)
        printf("Heap memory cleaned up\n");
}
