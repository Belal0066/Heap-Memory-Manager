#include "hmm.h"
int ceil2nALIGN_SIZE(size_t memSize)
{
    if (memSize & (ALIGN_SIZE - 1))
    {
        return ((memSize + (ALIGN_SIZE - 1)) & ~(ALIGN_SIZE - 1));
    }
    return memSize;
}
#define MIN_BLOCK_SIZE (ceil2nALIGN_SIZE(sizeof(struct BlkHdr)) + ceil2nALIGN_SIZE(sizeof(void *)))
static struct BlkHdr *findFreeBlock(size_t size)
{
    struct BlkHdr *current = freeList;
    while (current != NULL)
    {
        if (current->free && current->size >= size)
        {
            return current;
        }
        current = current->nxt;
    }
    return NULL;
}
static struct BlkHdr *allocateNewBlock(size_t size)
{
    if (hpBrk + size > HEAP_SIZE)
    {
        return NULL;
    }
    struct BlkHdr *block = (struct BlkHdr *)&hpArr[hpBrk];
    block->size = size;
    block->free = 0;
    block->nxt = NULL;
    hpBrk += size;
    return block;
}
static void splitBlock(struct BlkHdr *block, size_t size)
{
    if (block->size >= size + MIN_BLOCK_SIZE)
    {
        struct BlkHdr *newBlock = (struct BlkHdr *)((unsigned char *)block + size);
        newBlock->size = block->size - size;
        newBlock->free = 1;
        newBlock->nxt = block->nxt;

        block->size = size;
        block->nxt = newBlock;
    }
}
void *hmmAlloc(size_t size)
{
    if (size == 0)
    {
        return NULL;
    }
    size = ceil2nALIGN_SIZE(size) + ceil2nALIGN_SIZE(sizeof(struct BlkHdr));
    struct BlkHdr *block = findFreeBlock(size);
    if (block)
    {
        block->free = 0;
        splitBlock(block, size);
        return (void *)((unsigned char *)block + ceil2nALIGN_SIZE(sizeof(struct BlkHdr)));
    }

    block = allocateNewBlock(size);
    if (!block)
    {
        return NULL;
    }
    return (void *)((unsigned char *)block + ceil2nALIGN_SIZE(sizeof(struct BlkHdr)));
}
static struct BlkHdr *coalescePrev(struct BlkHdr *block)
{
    struct BlkHdr *current = freeList;
    struct BlkHdr *prev = NULL;

    while (current != NULL && current != block)
    {
        prev = current;
        current = current->nxt;
    }

    if (current == block)
    {

        if (prev != NULL && prev->free)
        {
            prev->size += block->size;
            prev->nxt = block->nxt;
            block = prev;
        }
    }
    return block;
}
static struct BlkHdr *coalesceNext(struct BlkHdr *block)
{
    if (block->nxt != NULL && block->nxt->free)
    {
        block->size += block->nxt->size;
        block->nxt = block->nxt->nxt;
    }
    return block;
}
void hmmFree(void *ptr)
{
    if (ptr == NULL)
    {
        return;
    }

    struct BlkHdr *block = (struct BlkHdr *)((unsigned char *)ptr - ceil2nALIGN_SIZE(sizeof(struct BlkHdr)));

    block->free = 1;

    block = coalescePrev(block);
    block = coalesceNext(block);

    if (freeList == NULL)
    {
        freeList = block;
        block->nxt = NULL;
    }
    else
    {
        struct BlkHdr *current = freeList;
        struct BlkHdr *prev = NULL;
        while (current != NULL && current < block)
        {
            prev = current;
            current = current->nxt;
        }
        if (prev == NULL)
        {

            block->nxt = freeList;
            freeList = block;
        }
        else
        {

            block->nxt = current;
            prev->nxt = block;
        }
    }
}
