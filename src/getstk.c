/* getstk.c - getstk */
#include "memory.h"
#include "types.h"
/*------------------------------------------------------------------------
* getstk - Allocate stack memory, returning highest word address
*------------------------------------------------------------------------
*/
void *getstk(
    uint32_t nbytes /* Size of memory requested */
)
{
    //intmask mask; /* Saved interrupt mask */
    struct memblk *prev, *curr; /* Walk through memory list */
    struct memblk *fits, *fitsprev; /* Record block that fits */
    //mask = disable();
    if (nbytes == 0)
    {
        //restore(mask);
        return NULL;
    }
    nbytes = (uint32_t) roundmb(nbytes); /* Use mblock multiples */
    prev = &memlist;
    curr = memlist.mnext;
    fits = NULL;
    fitsprev = NULL; /* Just to avoid a compiler warning */
    while (curr != NULL)   /* Scan entire list */
    {
        if (curr->mlength >= nbytes)   /* Record block address */
        {
            fits = curr; /* when request fits */
            fitsprev = prev;
        }
        prev = curr;
        curr = curr->mnext;
    }
    if (fits == NULL)   /* No block was found */
    {
        //restore(mask);
        return NULL;
    }
    if (nbytes == fits->mlength)   /* Block is exact match */
    {
        fitsprev->mnext = fits->mnext;
    }
    else     /* Remove top section */
    {
        fits->mlength -= nbytes;
        fits = (struct memblk *)((uint32_t)fits + fits->mlength);
    }
    memlist.mlength -= nbytes;
    //restore(mask);
    return (void *)((uint32_t) fits + nbytes - sizeof(uint32_t));
}
