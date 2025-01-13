# Custom Memory Management Library

`libmem` is a lightweight **custom memory allocator**.
It provides essential functions for dynamic memory allocation and deallocation, mimicking standard C functions (`malloc`, `free`) while introducing custom handling through a **linked list memory pool**.

Advantages:
0. Custom allocation strategy: implements a **first-fit** allocation algorithm.
Blocks are allocated from a pre-allocated 128MB memory pool.
2. Alignment and safety: memory allocations are aligned to 8-byte boundaries for better memory access efficiency. Prevents allocation of zero bytes and handles invalid memory addresses.
3. Error handling: returns error codes for different failure scenarios, including attempting ot free unallocated memory and attempting to free memory not owned by the current process.
4. Memory visualization: provides a `memoryMap()` function to point a visual representation of allocated and free memory regions.

Usage Summary:
- Initialize memory: `myInitializeMemory()`
- Allocate memory: `myMalloc(size)`
- Free memory: `myFree(ptr)`
- Print memory map: `memoryMap()`
