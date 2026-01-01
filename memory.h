#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

#define PAGE_SIZE 4096

/* round and truncate to 8-byte boundary */
#define roundmb(x) ( (uint32_t)( (x + 7) & ~7 ) )
#define truncmb(x) ( (uint32_t)( x & ~7 ) )

struct memblk {
    struct memblk *mnext;
    uint32_t mlength;
};

/* Free list head */
extern struct memblk memlist;

/* Heap bounds */
extern void *minheap;
extern void *maxheap;

/* Memory manager API */
void meminit(void *heap_start, void *heap_end);
void *getmem(uint32_t nbytes);
int freemem(void *blkaddr, uint32_t nbytes);
void *getstk(uint32_t nbytes);

/* Stack free macro (XINU style) */
#define freestk(p,len) \
    freemem((void *)((uint32_t)(p) - roundmb(len) + sizeof(uint32_t)), \
            roundmb(len))

#endif

