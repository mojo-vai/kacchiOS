/* scheduler.h - Cooperative scheduler interface */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

#define AGING_THRESHOLD 100  /* High threshold to avoid premature promotion in short-running systems */
#define MAX_PRIO 8

/* Initialize scheduler */
void scheduler_init(void);

/* Yield CPU voluntarily */
void yield(void);

/* Run scheduler */
void schedule(void);
void ctx_switch(uint32_t **old_sp, uint32_t *new_sp);


#endif
