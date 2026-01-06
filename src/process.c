/* process.c - Process management for kacchiOS */

#include "types.h"
#include "string.h"
#include "memory.h"
#include "process.h"
#include "scheduler.h"

/* Forward declaration of null_idle (defined in kernel.c) */
extern void null_idle(void);

/* -----------------------------
 * Global process table
 * ----------------------------- */

struct pcb proctab[NPROC];
int currpid = NULLPROC;

/* -----------------------------
 * Internal helper: allocate PID
 * ----------------------------- */

static int alloc_pid(void)
{
    static int nextpid = 1; /* PID 0 is reserved for null process */

    for (int i = 0; i < NPROC; i++)
    {
        int pid = nextpid;
        nextpid = (nextpid + 1) % NPROC;

        if (pid == NULLPROC)
            continue;

        if (proctab[pid].state == PR_FREE)
            return pid;
    }

    return -1; /* no free PID */
}

/* -----------------------------
 * Initialize process subsystem
 * ----------------------------- */

void process_init(void)
{
    for (int i = 0; i < NPROC; i++)
    {
        proctab[i].state = PR_FREE;
    }

    /* Initialize NULL process with proper stack setup */
    proctab[NULLPROC].pid = NULLPROC;
    proctab[NULLPROC].state = PR_CURR;
    proctab[NULLPROC].priority = 0;
    proctab[NULLPROC].wait_ticks = 0;

    /* Allocate a proper stack for NULL process */
    void *stack = getstk(NULL_STACK_SIZE);
    if (stack == NULL) {
        /* Fall back to current kernel ESP if allocation fails */
        uint32_t *esp;
        __asm__ volatile("movl %%esp, %0" : "=r"(esp));
        proctab[NULLPROC].sp = esp;
        proctab[NULLPROC].stack_base = NULL;
        proctab[NULLPROC].stack_size = 0;
    } else {
        /* Set up NULL process stack with null_idle entry */
        uint32_t *sp = (uint32_t *)((uint32_t)stack & ~0xF);

        /* Bottom-most return: if null_idle() returns */
        *(--sp) = (uint32_t)process_exit;

        /* Return address for ctx_switch → ret */
        *(--sp) = (uint32_t)null_idle;

        /* Fake callee-saved registers (MUST match pop order) */
        *(--sp) = 0; // EBP
        *(--sp) = 0; // EBX
        *(--sp) = 0; // ESI
        *(--sp) = 0; // EDI

        proctab[NULLPROC].sp = sp;
        proctab[NULLPROC].stack_base = stack;
        proctab[NULLPROC].stack_size = NULL_STACK_SIZE;
    }
}

/* -----------------------------
 * Create a new process
 * ----------------------------- */

int process_create(void (*entry)(void), const char *name)
{
    int pid;
    void *stack;

    if (entry == NULL)
        return -1;

    pid = alloc_pid();
    if (pid < 0)
        return -1;

    stack = getstk(PROC_STACK_SIZE);
    if (stack == NULL)
        return -1;

    /* Initialize PCB */
    proctab[pid].pid = pid;
    proctab[pid].state = PR_READY;
    proctab[pid].priority = DEFAULT_PRIO;
    proctab[pid].wait_ticks = 0;

    uint32_t *sp = (uint32_t *)((uint32_t)stack & ~0xF);

    /* Bottom-most return: if entry() returns */
    *(--sp) = (uint32_t)process_exit;

    /* Return address for ctx_switch → ret */
    *(--sp) = (uint32_t)entry;

    /* Fake callee-saved registers (MUST match pop order) */
    *(--sp) = 0; // EBP
    *(--sp) = 0; // EBX
    *(--sp) = 0; // ESI
    *(--sp) = 0; // EDI

    proctab[pid].sp = sp;

    proctab[pid].stack_base = stack;
    proctab[pid].stack_size = PROC_STACK_SIZE;

    /* Copy process name */
    if (name)
    {
        strcpy(proctab[pid].name, name);
        proctab[pid].name[PNMLEN - 1] = '\0';
    }
    else
    {
        proctab[pid].name[0] = '\0';
    }

    return pid;
}

/* -----------------------------
 * Terminate current process
 * ----------------------------- */

void process_exit(void)
{
    int pid = currpid;

    /* Null process must never exit */
    if (pid == NULLPROC)
        return;

    /* Free process stack */
    if (proctab[pid].stack_base != NULL)
    {
        freestk(proctab[pid].stack_base,
                proctab[pid].stack_size);
    }

    /* Mark PCB free */
    proctab[pid].state = PR_FREE;
    // proctab[pid].entry = NULL;
    proctab[pid].sp = NULL;
    proctab[pid].stack_base = NULL;
    proctab[pid].stack_size = 0;
    proctab[pid].name[0] = '\0';

    /* Control will return to scheduler later */
    schedule();
}

int getpid(void)
{
    return currpid;
}

int getpstate(int pid)
{
    if (isbadpid(pid))
        return -1;

    return proctab[pid].state;
}

const char *getpname(int pid)
{
    if (isbadpid(pid))
        return NULL;

    return proctab[pid].name;
}

void block_current(void)
{
    int pid = currpid;

    /* Null process must never block */
    if (pid == NULLPROC)
        return;

    /* Mark process as blocked */
    proctab[pid].state = PR_BLOCKED;

    /* Give up CPU */
    yield();
}
int wakeup(int pid)
{
    if (isbadpid(pid))
        return -1;

    if (proctab[pid].state != PR_BLOCKED)
        return -1;

    /* Make process ready again */
    proctab[pid].state = PR_READY;

    return 0;
}

int set_priority(int pid, int prio)
{
    if (isbadpid(pid) || prio < 0)
        return -1;

    proctab[pid].priority = prio;
    return 0;
}
int get_priority(int pid)
{
    if (isbadpid(pid))
        return -1;

    return proctab[pid].priority;
}

int send(int pid, msg_t msg)
{
    if (isbadpid(pid))
        return -1;

    if (proctab[pid].has_msg)
        return -1;

    proctab[pid].msg = msg;
    proctab[pid].has_msg = 1;

    if (proctab[pid].state == PR_BLOCKED)
        wakeup(pid);

    return 0;
}

msg_t receive(void)
{
    int pid = currpid;

    while (!proctab[pid].has_msg)
    {
        block_current(); // PR_BLOCKED
    }

    proctab[pid].has_msg = 0;
    return proctab[pid].msg;
}
