# HMM Library

The HMM Library is a custom memory allocation library that implements standard memory allocation functions such as `malloc`, `free`, `calloc`, and `realloc`. It uses a best-fit allocation strategy and includes features like memory coalescing to reduce fragmentation and optional logging of allocation and deallocation operations.


## Table of Contents

- [Features](#features)
- [Functionality](#functionality)
- [Configuration](#configuration)
- [Usage](#usage)
- [Limitations](#limitations)



## Files

- `libhmm.c`: Main implementation of the custom allocator
- `libhmm.h`: Header file with function declarations
- `Makefile`: Used to build the shared library

## Features

- Custom implementation of standard memory allocation functions
- Best-fit allocation strategy
- Memory coalescing to reduce fragmentation
- Optional logging of allocation and deallocation operations

## Functionality

### `void *malloc(size_t size);`

Allocates `size` bytes of memory and returns a pointer to the allocated memory. The memory is not initialized.

### `void free(void *ptr);`

Frees the memory space pointed to by `ptr`, which must have been returned by a previous call to `malloc()`, `calloc()`, or `realloc()`.

### `void *calloc(size_t nmemb, size_t size);`

Allocates memory for an array of `nmemb` elements of `size` bytes each and returns a pointer to the allocated memory. The memory is set to zero.

### `void *realloc(void *ptr, size_t size);`

Changes the size of the memory block pointed to by `ptr` to `size` bytes. The contents will be unchanged in the range from the start of the region up to the minimum of the old and new sizes.

## Configuration

The libhmm can be configured by modifying the following macros in `libhmm.h`:

- `LOG_MODE`: Set to 1 to enable logging of allocation and deallocation operations
- `ALIGNMENT`: Set the byte alignment for allocated memory (default is 8)
- `PAGE_SIZE`: Set the size of memory pages requested from the system (default is 4096)

## Usage

1. Build the shared library:
   ```
   make
   ```

2. Using the `LD_PRELOAD` environment variable. This will cause the custom HMM functions to be used instead of the standard libc functions.
   ```
   LD_PRELOAD=./libhmm.so vim
   ```


## Limitations

- This hmm is not thread-safe. It should not be used in multi-threaded applications without additional synchronization.
- The logging feature, when enabled, may impact performance and generate large log file for allocation-heavy applications.

