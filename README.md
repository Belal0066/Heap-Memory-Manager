# HMM - Heap Memory Manager

HMM is a simple heap memory manager implemented in C. It provides basic memory allocation and deallocation functions, `hmmAlloc()` and `hmmFree()`, that allow you to dynamically allocate and free memory blocks.

## Design

The HMM implementation consists of the following key components:

1. **Heap**: The HMM utilizes a fixed-size heap of 1024 bytes, defined by the `HEAP_SIZE` constant. The current position of the heap break is tracked using the `hpBrk` variable.

2. **Block Header**: Each allocated memory block is preceded by a `BlkHdr` struct, which stores the following information about the block:
   - `free`: A flag indicating whether the block is free or allocated.
   - `size`: The size of the block in bytes.
   - `nxt`: A pointer to the next block in the free list.

3. **Free List**: HMM maintains a linked list of free blocks, called the "free list", to efficiently find available memory for allocation. The `freeList` pointer points to the head of the free list.

4. **Allocation and Deallocation**:
   - `hmmAlloc(size_t size)`: This function finds a suitable free block in the free list or allocates a new block at the end of the heap if no free block is available. If the found block is larger than the requested size, the function splits the block and adds the remaining part to the free list.
   - `hmmFree(void* ptr)`: This function takes a pointer to an allocated block and marks the block as free. It then coalesces the freed block with its adjacent free blocks, if any, to create larger free blocks.

5. **Alignment**: HMM ensures that all allocated memory blocks are aligned to 8 bytes, as defined by the `ALIGN_SIZE` constant. The `ceil2nALIGN_SIZE()` function is used to calculate the smallest multiple of `ALIGN_SIZE` that is greater than or equal to the requested size.

## Flowcharts


### `hmmAlloc()` Flowchart



### `hmmFree()` Flowchart


## Compilation


## Testing


