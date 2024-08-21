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

![Screenshot from 2024-08-21 22-32-50](https://github.com/user-attachments/assets/34995463-d385-47ce-9052-501d118c1b80)


### `hmmFree()` Flowchart
![Screenshot from 2024-08-21 22-31-32](https://github.com/user-attachments/assets/4e8c90b9-8bee-4818-a396-9c37f1b0edfd)


## Testing with `stress.c`
```Bash
belal@Ubuntu:~/Desktop/st/HMM/main$ gcc -o c hmm.c fs.c 
belal@Ubuntu:~/Desktop/st/HMM/main$ ./c 1 1 10
etext = 0x587103cbb97d, edata=0x587103cbe010, end=0x587103cc0070, initial program break=0x587104248000
Initial program break:          0x587104269000
Allocating 1*1 bytes
Program break is now:           0x587104269000
Freeing blocks from 1 to 1 in steps of 10
After HmmFree(), program break is: 0x587104269000
```


## Testing with `stress.c`
```Bash
belal@Ubuntu:~/Desktop/st/HMM/main$ gcc -o c hmm.c stress.c 
belal@Ubuntu:~/Desktop/st/HMM/main$ ./c
Starting random allocation and deallocation test...
Allocated memory of size 8 at address 0x5afbd526d058
Allocated memory of size 6 at address 0x5afbd526d0d8
Allocated memory of size 4 at address 0x5afbd526d158
Allocated memory of size 4 at address 0x5afbd526d1d8
Allocated memory of size 6 at address 0x5afbd526d258
Freeing memory at address 0x5afbd526d0d8
Allocated memory of size 6 at address 0x5afbd526d0d8
Allocated memory of size 1 at address 0x5afbd526d2d8
Freeing memory at address 0x5afbd526d2d8
Freeing memory at address 0x5afbd526d0d8
Freeing memory at address 0x5afbd526d058
```
#### `To Fix`:
- [ ] Infinite loop wiht testing `stress.c` 

