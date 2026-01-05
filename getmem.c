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
    struct memblk *best = NULL;
    struct memblk *best_prev = NULL;
    //mask = disable();
    if (nbytes == 0)
    {
        //restore(mask);
        return NULL;
    }
    nbytes = (uint32_t) roundmb(nbytes); /* Use memblk multiples */
    prev = &memlist;
    curr = memlist.mnext;
        while (curr != NULL) {
        if (curr->mlength >= nbytes) {
            if (best == NULL || curr->mlength < best->mlength) {
                best = curr;
                best_prev = prev;
            }
        }
        prev = curr;
        curr = curr->mnext;
    }
        /* No suitable block found */
    if (best == NULL)
        return NULL;

    /* Exact fit */
    if (best->mlength == nbytes) {
        best_prev->mnext = best->mnext;
    }
    /* Split block */
    else {
        leftover = (struct memblk *)((char *)best + nbytes);
        leftover->mlength = best->mlength - nbytes;
        leftover->mnext = best->mnext;

        best_prev->mnext = leftover;
    }

    memlist.mlength -= nbytes;

    return (void *)best;
}
