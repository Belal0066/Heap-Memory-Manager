#include "hmm.h"

static BlockHeader* free_list = NULL;
static uint8_t memory[MEMORY_SIZE];
static void* program_break = memory;
static pthread_mutex_t hmm_mutex = PTHREAD_MUTEX_INITIALIZER;

#define ceilTo_N_ALIGNMENT(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

void* my_sbrk(intptr_t increment) {
    pthread_mutex_lock(&hmm_mutex);
    
    if ((uintptr_t)program_break + increment > (uintptr_t)memory + MEMORY_SIZE) {
        pthread_mutex_unlock(&hmm_mutex);
        return (void*)-1;  
    }
    void* old_break = program_break;
    program_break += increment;
    
    pthread_mutex_unlock(&hmm_mutex);
    return old_break;
}

void* get_program_break(void) {
    pthread_mutex_lock(&hmm_mutex);
    void* current_break = program_break;
    pthread_mutex_unlock(&hmm_mutex);
    return current_break;
}

void* hmmAlloc(size_t size) {
    pthread_mutex_lock(&hmm_mutex);
    
    if (size == 0) {
        size = sizeof(BlockHeader);
    }
    else {
        size = ceilTo_N_ALIGNMENT(size);
    }

    size_t total_size = size + sizeof(BlockHeader);
    BlockHeader* current = free_list;
    BlockHeader* best_fit = NULL;

    
    while (current != NULL) {
        if (current->is_free && current->size >= total_size) {
            if (best_fit == NULL || current->size < best_fit->size) {
                best_fit = current;
            }
        }
        current = current->next;
    }

    if (best_fit != NULL) {
        if (best_fit->size >= total_size + sizeof(BlockHeader) + 1) {
            
            BlockHeader* new_block = (BlockHeader*)((char*)best_fit + total_size);
            new_block->size = best_fit->size - total_size;
            new_block->is_free = true;
            new_block->next = best_fit->next;
            new_block->prev = best_fit;

            if (best_fit->next != NULL) {
                best_fit->next->prev = new_block;
            }
            best_fit->next = new_block;
            best_fit->size = total_size;
        }
        best_fit->is_free = false;
        void* result = (char*)best_fit + sizeof(BlockHeader);
        pthread_mutex_unlock(&hmm_mutex);
        return result;
    }

    
    BlockHeader* new_block = my_sbrk(total_size);
    if (new_block == (void*)-1) {
        pthread_mutex_unlock(&hmm_mutex);
        return NULL;  
    }

    new_block->size = total_size;
    new_block->is_free = false;
    new_block->next = NULL;
    new_block->prev = NULL;

    
    if (free_list == NULL) {
        free_list = new_block;
    } else {
        current = free_list;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_block;
        new_block->prev = current;
    }

    void* result = (char*)new_block + sizeof(BlockHeader);
    pthread_mutex_unlock(&hmm_mutex);
    return result;
}

void hmmFree(void* ptr) {
    if (ptr == NULL) {
        return;
    }

    pthread_mutex_lock(&hmm_mutex);

    BlockHeader* block = (BlockHeader*)((char*)ptr - sizeof(BlockHeader));
    if (block->is_free) {
        
        fprintf(stderr, "Error: Attempt to double free memory at %p\n", ptr);
        pthread_mutex_unlock(&hmm_mutex);
        return; 
    }
    block->is_free = true;

    
    if (block->prev != NULL && block->prev->is_free) {
        block->prev->size += block->size;
        block->prev->next = block->next;
        if (block->next != NULL) {
            block->next->prev = block->prev;
        }
        block = block->prev;
    }

    if (block->next != NULL && block->next->is_free) {
        block->size += block->next->size;
        block->next = block->next->next;
        if (block->next != NULL) {
            block->next->prev = block;
        }
    }

    
    if (block->next == NULL && (char*)block + block->size == program_break) {
        if (block->prev != NULL) {
            block->prev->next = NULL;
        } else {
            free_list = NULL;
        }
        my_sbrk(-(intptr_t)block->size);
    }
    
    pthread_mutex_unlock(&hmm_mutex);
}


void print_heap_state() {
    pthread_mutex_lock(&hmm_mutex);
    
    BlockHeader* current = free_list;
    printf("Heap state:\n");
    while (current != NULL) {
        printf("Block at %p: size=%zu, is_free=%d\n", 
               (void*)current, current->size, current->is_free);
        current = current->next;
    }
    printf("Program break: %p\n", program_break);
    printf("\n");
    
    pthread_mutex_unlock(&hmm_mutex);
}



