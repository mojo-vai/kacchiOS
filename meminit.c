#include "memory.h"

struct memblk memlist;
void *minheap;
void *maxheap;

void meminit(void *heap_start, void *heap_end)
{
    minheap = heap_start;
    maxheap = heap_end;

    memlist.mnext = (struct memblk *)heap_start;
    memlist.mlength = (uint32_t)((uint8_t *)heap_end -
                                 (uint8_t *)heap_start);

    memlist.mnext->mnext = NULL;
    memlist.mnext->mlength = memlist.mlength;
}
