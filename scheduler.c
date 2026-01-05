/* scheduler.c - Cooperative priority scheduler with aging */

#include "scheduler.h"
#include "process.h"
#include "serial.h"

/* ---------- READY QUEUES ---------- */
/* One FIFO queue per priority */
static int ready_queue[MAX_PRIO][NPROC];
static int rq_head[MAX_PRIO];
static int rq_tail[MAX_PRIO];

/* ---------- QUEUE HELPERS ---------- */

static int rq_empty_prio(int pr)
{
    return rq_head[pr] == rq_tail[pr];
}

static void rq_enqueue(int pid)
{
    int pr = proctab[pid].priority;

    if (pid == NULLPROC)
        return;

    if (pr < 0)
        pr = 0;
    if (pr >= MAX_PRIO)
        pr = MAX_PRIO - 1;

    ready_queue[pr][rq_tail[pr] % NPROC] = pid;
    rq_tail[pr]++;
}

static int rq_dequeue_prio(int pr)
{
    if (rq_empty_prio(pr))
        return -1;

    int pid = ready_queue[pr][rq_head[pr] % NPROC];
    rq_head[pr]++;
    return pid;
}

static int rq_dequeue_highest(void)
{
    for (int pr = MAX_PRIO - 1; pr >= 0; pr--) {
        if (!rq_empty_prio(pr)) {
            int pid = ready_queue[pr][rq_head[pr] % NPROC];
            rq_head[pr]++;

            return pid;
        }
    }
    return -1;
}

/* ---------- INITIALIZATION ---------- */

void scheduler_init(void)
{
    for (int pr = 0; pr < MAX_PRIO; pr++) {
        rq_head[pr] = rq_tail[pr] = 0;
    }

    /* Enqueue all processes that are READY (except NULL) */
    for (int i = 0; i < NPROC; i++) {
        if (i != NULLPROC && proctab[i].state == PR_READY) {
            rq_enqueue(i);
        }
    }
}

/* ---------- YIELD ---------- */

void yield(void)
{
    if (currpid != NULLPROC) {
        proctab[currpid].state = PR_READY;
        rq_enqueue(currpid);
    }

    schedule();

    /* MUST NEVER RETURN */
}

/* ---------- CORE SCHEDULER ---------- */

void schedule(void)
{
    /* ---------- AGING ---------- */
    for (int i = 0; i < NPROC; i++) {
        if (proctab[i].state == PR_READY) {
            proctab[i].wait_ticks++;

            if (proctab[i].wait_ticks >= AGING_THRESHOLD) {
                if (proctab[i].priority < MAX_PRIO - 1)
                    proctab[i].priority++;
                proctab[i].wait_ticks = 0;
            }
        }
    }

    int old = currpid;

    /* ---------- PICK NEXT PROCESS ---------- */
    int next = rq_dequeue_highest();

    if (next < 0) {
        /* No runnable process → stay in current */
        return;
    }

    /* Don't context switch if same process */
    if (next == old) {
        return;
    }

    /* ---------- SWITCH ---------- */
    proctab[next].state = PR_CURR;
    proctab[next].wait_ticks = 0;
    currpid = next;

    ctx_switch(&proctab[old].sp, proctab[next].sp);
}
