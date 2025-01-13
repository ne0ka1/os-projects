#ifndef LIBMEM_H
#define LIBMEM_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#define POOL_SIZE 134217728

// Process Control Block
struct pcb {
    uint32_t pid;
};

// Bookkeeping region
struct mem_region {
    uint32_t free: 1;
    uint32_t size: 31;
    uint32_t pid;
    uint8_t data[0];
};

extern struct pcb *currentPCB;
extern struct mem_region *pool;

uint32_t getCurrentPID();

int myInitializeMemory();

void *myMalloc(size_t size);

int myFreeErrorCode(void *ptr);

void myFree(void *ptr);

void myCleanupMemory();

void memoryMap();

#endif
