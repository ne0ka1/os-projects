#include "libmem.h"

// Global value pointing to current PCB
struct pcb *currentPCB = NULL;

// getCurrentPID returns the PID contained in the PCB of the current process
uint32_t getCurrentPID(){
    return currentPCB->pid;
}

// pointer to the start of 128MB pool
struct mem_region *pool = NULL;

// size of mem_region (should be 8)
int mem_region_size = (int)sizeof(struct mem_region);

// Initialize memory pool and currentPC
int myInitializeMemory(){
    // set up a single PCB if havn't
    if (currentPCB == NULL){
        currentPCB = malloc(sizeof(struct pcb));

        // check if malloc was successful
        if (currentPCB == NULL){
            fprintf(stderr, "Error: Memory allocation for pcb failed.\n");
            return 1;
        }

        currentPCB->pid = 0;

    }

    // allocate a 128MB memory pool if havn't
    if (pool == NULL){
        pool = (struct mem_region *)malloc(POOL_SIZE);

        // Check if malloc was successful
        if (pool == NULL){
            fprintf(stderr, "Error: Memory allocation for pool failed.\n");
            return 1;
        }

        // Bookkeeping section of the pool
        pool->free = 1;
        pool->size = POOL_SIZE - mem_region_size;
    }
    return 0;
}

/*
 * myMalloc takes the size in bytes of the storage needed by the caller,
 * allocates an appropriately sized region of memory,
 * and return a pointer to the first byte of that region
 * It return a NULL pointer if the storage cannot be successfully allocated
 * or a request is made to allocate 0 bytes of memory
 * The pointer returned is always on an 8-byte boundary
 */
void *myMalloc(size_t size){
    if (size == 0){
        return NULL;
    }

    // bookkeeping section to be allocated
    struct mem_region *mem;
    
    // round up size to 8-byte
    size_t rounded = ((size + 7) / 8) * 8;

    // iterator head
    struct mem_region *head = pool;

    // iterate through LL
    int iterated = 0;
    while (iterated < POOL_SIZE){
        // find a block large enough, first fit algorithm
        if (head->free && head->size >= rounded){
            mem = head;
            // split if the space larger than 64;
            if ((head->size - rounded) > 64){
                struct mem_region *next;
                next = (struct mem_region *)&head->data[rounded];
                next->free = 1;
                next->size = head->size - rounded - mem_region_size;
            }
            break;
        }
        else{
            iterated = iterated + mem_region_size + head->size;
            head = (struct mem_region *)&head->data[head->size];
        }
    }

    // cannot find the appropriate block
    if (iterated == POOL_SIZE){
        return NULL; 
    }
    
    mem->free = 0;
    mem->size = rounded;
    mem->pid = getCurrentPID();
    return mem->data;
}

/*
 * myFreeErrorCode takes a pointer to a region of memory previously
 * allocated by the myMalloc function and deallocates that region.
 * It returns an int to indicate success or failure of the deallocation.
 * 1 means success; 2 means attempt to free storage at an invalid block address.
 * 3 means attempt to free storage that is not currently allocated.
 * 4 means attempt to free storage owned by a different PID.
 */
int myFreeErrorCode(void *ptr){
    // attempt to free storage at an invalid block address
    if (ptr == NULL){
        return 2;
    }

    // ptr is at the beginning of the pool
    struct mem_region *head = pool;
    struct mem_region *prev;
    if (head->data == ptr){
        if (head->pid != getCurrentPID()){
            // attempt to free storage owned by a different PID
            return 4;
        }
        // deallocate
        head->free = 1;
	// possibly merge with next block
	if (mem_region_size + head->size < POOL_SIZE){
            struct mem_region *next;
            next = (struct mem_region *)&head->data[head->size];
            if (next->free){
	        head->size = head->size + mem_region_size + next->size;
	    }
	}
        return 1;
    }
    prev = head;
    head = (struct mem_region *)&head->data[head->size];

    // iterate through LL to find ptr
    int iterated = mem_region_size + prev->size;
    while (iterated < POOL_SIZE){
        if (head->data == ptr){
            if (head->pid != getCurrentPID()){
                return 4;
            }
            // attempt to free storage that is not currently allocated;
            if (head->free){
                return 3;
            }
            // deallocate
            head->free = 1;
	    // possibly merge with next block
	    if (iterated + mem_region_size + head->size < POOL_SIZE){
                struct mem_region *next;
                next = (struct mem_region *)&head->data[head->size];
		if (next->free){
		    head->size = head->size + mem_region_size + next->size;
		}
	    }
            // possibly merge with previous block
            if (prev->free){
                prev->size = prev->size + mem_region_size + head->size; 
            }
            return 1;
        }
        else{
            iterated = iterated + mem_region_size + head->size;
            prev = head;
            head = (struct mem_region *)&head->data[head->size];
        }
    }

    // ptr not found; invalid block address
    if (iterated == POOL_SIZE){
        return 2; 
    }
    return 1;
}

/*
 * myFree takes a pointer to a region of memory previously
 * allocated by the myMalloc function and deallocates that region.
 * If the value of the parameter is invalid, the call has no effect.
 */
void myFree(void *ptr){
    (void)myFreeErrorCode(ptr);
}

/*
 * print out a map of all used and free regions in the 128M byte region
 * of memory.
 */
void memoryMap(){
    struct mem_region *head = pool;
     
    fputs(" pid  free    size        addr\r\n", stdout);
    fputs("-------------------------------------\r\n", stdout);
    // iterate through LL
    int iterated = 0;
    while (iterated < POOL_SIZE){
        fprintf(stdout, " %3d   %3s  %9d  %p\r\n",\
                head->pid, head->free ? "yes" : "no", head->size, head->data);
        iterated = iterated + mem_region_size + head->size;
        head = (struct mem_region *)&head->data[head->size];
    }
    fputs("\n", stdout);
}
