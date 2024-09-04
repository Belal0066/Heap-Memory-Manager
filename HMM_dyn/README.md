# HMM_dyn

This project provides a simple custom implementation of malloc, free, calloc, and realloc functions.



## Usage

### Dynamic Linking (LD_PRELOAD)

1. Build:
   ```bash
   gcc -fPIC -shared -o libdhmm.so dhmm.c
   ```

2. Run:
   ```bash
   LD_PRELOAD=./libdhmm.so ./prog
   ```



## Functions

- `void *malloc(size_t size)`: Allocates `size` bytes of memory.
- `void free(void *ptr)`: Frees the memory pointed to by `ptr`.
- `void *calloc(size_t nmemb, size_t size)`: Allocates memory for an array of `nmemb` elements of `size` bytes each and initializes to zero.
- `void *realloc(void *ptr, size_t size)`: Changes the size of the memory block pointed to by `ptr` to `size` bytes.


