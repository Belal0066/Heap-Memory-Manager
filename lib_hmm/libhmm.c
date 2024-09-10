#include "libhmm.h"

#define ceil_n_ALIGNMENT(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

#define ceil_n_PAGE_SIZE(size) (((size) + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1))

void *request_more_memory(size_t size)
{
    size_t total_size = ceil_n_PAGE_SIZE(size);
    void *prev_break = sbrk(total_size);
    if (prev_break == (void *)-1)
    {
        return NULL;
    }
    return prev_break;
}


BlockHeader *find_best_fit_block(size_t size)
{
    BlockHeader *current = free_list_head;
    BlockHeader *best_fit = NULL;

    while (current)
    {
        if (current->is_free && current->size >= size)
        {
            if (!best_fit || current->size < best_fit->size)
            {
                best_fit = current;
            }
        }
        current = current->next;
    }
    return best_fit;
}

void coalesce_free_blocks(BlockHeader *block)
{

    if (block->next && block->next->is_free)
    {
        block->size += block->next->size;
        block->next = block->next->next;
        if (block->next)
        {
            block->next->prev = block;
        }
    }

    if (block->prev && block->prev->is_free)
    {
        block->prev->size += block->size;
        block->prev->next = block->next;
        if (block->next)
        {
            block->next->prev = block->prev;
        }
    }
}

void *hmmAlloc(size_t size)
{
    size_t total_size;
    
    if (size == 0)
    {
        total_size = sizeof(BlockHeader);
    }
    else
    {
        size = ceil_n_ALIGNMENT(size);
        total_size = sizeof(BlockHeader) + size;
    }
    
    BlockHeader *block = find_best_fit_block(total_size);

    if (!block)
    {

        block = request_more_memory(total_size);
        if (!block)
        {
            return NULL;
        }

        block->size = size;
        block->is_free = 0;
        block->next = NULL;
        block->prev = NULL;
    }
    else
    {
        block->is_free = 0;
    }

    if (block->size > size + sizeof(BlockHeader))
    {
        BlockHeader *new_block = (BlockHeader *)((char *)block + sizeof(BlockHeader) + size);
        new_block->size = block->size - size;
        new_block->is_free = 1;
        new_block->next = block->next;
        new_block->prev = block;

        block->size = size;
        block->next = new_block;

        if (new_block->next)
        {
            new_block->next->prev = new_block;
        }
    }

    return (void *)(block + 1);
}

void hmmFree(void *ptr)
{
    if (!ptr)
        return;

    BlockHeader *block = (BlockHeader *)ptr - 1;

    if (block->is_free)
    {

        fprintf(stderr, "Error: Double free detected\n");
        return;
    }

    block->is_free = 1;

    coalesce_free_blocks(block);
}

// allocation functions
void *malloc(size_t size)
{
    #ifdef LOG_MODE
    {

        int fd;

        fd = open("outputING.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  
        char log_message[50];
        void *ptr = hmmAlloc(size);
        int len = snprintf(log_message, sizeof(log_message), "Malloc called with size: %zu, %p\n", size, ptr);

        write(fd, log_message, len);

        close(fd);

        return ptr;
    }
    #endif

    return hmmAlloc(size);
}

void free(void *ptr)
{
    #ifdef LOG_MODE
    {

        int fd;

        fd = open("outputING.txt", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

        char log_message[50];
        int len = snprintf(log_message, sizeof(log_message), "Free called for: %p\n", ptr);

        write(fd, log_message, len);

        close(fd);
        hmmFree(ptr);
        return;
    }
    #endif
    hmmFree(ptr);
}

void *calloc(size_t nmemb, size_t size)
{
    size_t total_size = nmemb * size;
    void *ptr = hmmAlloc(total_size);
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
        return hmmAlloc(size);
    }
    if (size == 0)
    {
        hmmFree(ptr);
        return NULL;
    }

    BlockHeader *old_block = (BlockHeader *)ptr - 1;
    size_t old_size = old_block->size;

    void *new_ptr = hmmAlloc(size);
    if (new_ptr)
    {
        size_t copy_size = (old_size < size) ? old_size : size;
        memcpy(new_ptr, ptr, copy_size);
        hmmFree(ptr);
    }
    return new_ptr;
}
