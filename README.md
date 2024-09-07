# Heap Memory Manager Projects

This repository contains two projects focused on managing heap memory in C: a simulation of a Heap Memory Manager (HMM) and a custom HMM Library. These projects demonstrate low-level memory management concepts, including dynamic memory allocation, deallocation, and fragmentation management.

## Table of Contents    

- [Projects Overview](#projects-overview)
  - [HMM Simulation](#heap-memory-manager-simulation)
  - [HMM Library](#hmm-library)
- [Prerequisites](#prerequisites)
- [Contributing](#contributing)

## Projects Overview
### Heap Memory Manager Simulation

This project simulates a heap memory manager in C, designed to manage dynamic memory allocation and deallocation, and track the state of the heap. It mimics the behavior of low-level memory management routines using a best-fit algorithm to manage free memory blocks.

For more details, refer to the [Heap Memory Manager Simulation README](hmm_sim/README.md).

### HMM Library

The HMM Library is a custom memory allocation library that implements standard memory allocation functions such as `malloc`, `free`, `calloc`, and `realloc`. It uses a best-fit allocation strategy and includes features like memory coalescing to reduce fragmentation and optional logging of allocation and deallocation operations.

For more details, refer to the [HMM Library README](lib_hmm/README.md).


## Prerequisites

- GCC (GNU Compiler Collection)
- Make





## Contributing

Contributions are welcomed! Please fork the repository and submit a pull request for any enhancements or bug fixes.

