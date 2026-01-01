/* freemem.c - freemem */
#include "memory.h"
#include "types.h"
/*------------------------------------------------------------------------
* freemem - Free a memory block, returning the block to the free list
*------------------------------------------------------------------------
*/
int freemem(
    void *blkaddr, /* Pointer to memory block */
    uint32_t nbytes /* Size of block in bytes */
)
{
    //intmask mask; /* Saved interrupt mask */
    struct memblk *next, *prev, *block;
    uint32_t top;
    //mask = disable();
    if ((nbytes == 0) || ((uint32_t) blkaddr < (uint32_t) minheap)
            || ((uint32_t) blkaddr > (uint32_t) maxheap))
    {
        //restore(mask);
        return -1;
    }
    nbytes = (uint32_t) roundmb(nbytes); /* Use memblk multiples */
    block = (struct memblk *)blkaddr;
    prev = &memlist; /* Walk along free list */
    next = memlist.mnext;
    while ((next != NULL) && (next < block))
    {
        prev = next;
        next = next->mnext;
    }
    if (prev == &memlist)   /* Compute top of previous block*/
    {
        top = (uint32_t) NULL;
    }
    else
    {
        top = (uint32_t) prev + prev->mlength;
    }
    /* Ensure new block does not overlap previous or next blocks */
    if (((prev != &memlist) && (uint32_t) block < top)
            || ((next != NULL) && (uint32_t) block+nbytes>(uint32_t)next))
    {
        //restore(mask);
        return -1;
    }
    memlist.mlength += nbytes;
    /* Either coalesce with previous block or add to free list */
    if (top == (uint32_t) block)   /* Coalesce with previous block */
    {
        prev->mlength += nbytes;
        block = prev;
    }
    else     /* Link into list as new node */
    {
        block->mnext = next;
        block->mlength = nbytes;
        prev->mnext = block;
    }
    /* Coalesce with next block if adjacent */
    if (((uint32_t) block + block->mlength) == (uint32_t) next)
    {
        block->mlength += next->mlength;
        block->mnext = next->mnext;
    }
    //restore(mask);
    return 0;
}
