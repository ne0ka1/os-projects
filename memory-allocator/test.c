#include "libmem.h"

int main(){
    if (myInitializeMemory()){
        return 1;
    }
    
    // memory allocation and deallocation requests
    void *p0 = myMalloc(16);
    memoryMap();
    void *p1 = myMalloc(400);
    memoryMap();
    void *p2 = myMalloc(80);
    memoryMap();
    myFree(p1);
    memoryMap();
    void *p3 = myMalloc(120);
    memoryMap();
    void *p4 = myMalloc(200);
    memoryMap();
    void *p5 = myMalloc(160);
    memoryMap();
    myFree(p3);
    memoryMap();
    myFree(p4);
    memoryMap();
    myFree(p2);
    memoryMap();
    myFree(p0);
    memoryMap();
    myFree(p5);
    memoryMap();

    // free PCB and memory pool
    myCleanupMemory();
}
