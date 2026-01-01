/* getmem.c - getmem */
#include "types.h"
#include "memory.h"
/*------------------------------------------------------------------------
* getmem - Allocate heap storage, returning lowest word address
*------------------------------------------------------------------------
*/
void *getmem(
    uint32_t nbytes /* Size of memory requested */
)
{
    //intmask mask; /* Saved interrupt mask */
    struct memblk *prev, *curr, *leftover;
    //mask = disable();
    if (nbytes == 0)
    {
        //restore(mask);
        return NULL;
    }
    nbytes = (uint32_t) roundmb(nbytes); /* Use memblk multiples */
    prev = &memlist;
    curr = memlist.mnext;
    while (curr != NULL)   /* Search free list */
    {
        if (curr->mlength == nbytes)   /* Block is exact match */
        {
            prev->mnext = curr->mnext;
            memlist.mlength -= nbytes;
            //restore(mask);
            return (void *)(curr);
        }
        else if (curr->mlength > nbytes)     /* Split big block */
        {
            leftover = (struct memblk *)((uint32_t) curr +
                                         nbytes);
            prev->mnext = leftover;
            leftover->mnext = curr->mnext;
            leftover->mlength = curr->mlength - nbytes;
            memlist.mlength -= nbytes;
            //restore(mask);
            return (void *)(curr);
        }
        else     /* Move to next block */
        {
            prev = curr;
            curr = curr->mnext;
        }
    }
    //restore(mask);
    return NULL;
}
