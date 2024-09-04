#include "hmm.h"

static BlockHeader* free_list = NULL;
static uint8_t memory[MEMORY_SIZE];
static void* program_break = memory;

void* my_sbrk(intptr_t increment) {
    if ((uintptr_t)program_break + increment > (uintptr_t)memory + MEMORY_SIZE) {
        return (void*)-1;  
    }
    void* old_break = program_break;
    program_break += increment;
    return old_break;
}

void* get_program_break(void) {
    return my_sbrk(0);
}

size_t ceilTo_N_ALIGNMENT(size_t size) {
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
} 



void* hmmAlloc(size_t size) {
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
        return (char*)best_fit + sizeof(BlockHeader);
    }

    
    BlockHeader* new_block = my_sbrk(total_size);
    if (new_block == (void*)-1) {
        return NULL;  
        printf("Out of memory\n");
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

    return (char*)new_block + sizeof(BlockHeader);
}

void hmmFree(void* ptr) {
    if (ptr == NULL) {
        return;
    }


    BlockHeader* block = (BlockHeader*)((char*)ptr - sizeof(BlockHeader));
    if (block->is_free) {
        
        fprintf(stderr, "Error: Attempt to double free memory at %p\n", ptr);
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
}


void print_heap_state() {
    BlockHeader* current = free_list;
    printf("Heap state:\n");
    while (current != NULL) {
        printf("Block at %p: size=%zu, is_free=%d\n", 
               (void*)current, current->size, current->is_free);
        current = current->next;
    }
    printf("Program break: %p\n", program_break);
    printf("\n");
}



