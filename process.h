/* process.h - Process management definitions for kacchiOS */

#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"

/* -----------------------------
 * System-wide process limits
 * ----------------------------- */

/* Maximum number of processes */
#define NPROC       8

/* Process name length */
#define PNMLEN      16

/* PID of the null (idle) process */
#define NULLPROC    0

/* Default stack size for processes (bytes) */
#define PROC_STACK_SIZE  4096

#define DEFAULT_PRIO 1
#define NULL_PRIO    0

#define NULL_STACK_SIZE 4096

/* -----------------------------
 * Process states
 * ----------------------------- */

#define PR_FREE     0   /* PCB entry unused */
#define PR_READY    1   /* Process is ready to run */
#define PR_CURR     2   /* Process is currently running */
#define PR_TERM     3   /* Process has terminated */
#define PR_SLEEP    4
#define PR_BLOCKED  5

typedef int msg_t;
/* -----------------------------
 * Process Control Block (PCB)
 * ----------------------------- */

struct pcb {
    int         pid;                    /* Process ID */
    uint16_t    state;                  /* Process state */
    int priority;
    int wait_ticks;

    /* Stack management */
    uint32_t      *sp;              /* Saved stack pointer */
    void       *stack_base;             /* Base (lowest addr) of stack */
    uint32_t    stack_size;             /* Stack size in bytes */
    msg_t msg;        /* message */
    int has_msg;      /* 0 = no message, 1 = message available */

    /* Process entry point */
    void      (*entry)(void);            /* Function where process starts */

    /* Debugging / identification */
    char        name[PNMLEN];
};


/* -----------------------------
 * Global process table & state
 * (defined in process.c)
 * ----------------------------- */

extern struct pcb proctab[NPROC];
extern int currpid;     /* PID of currently running process */


/* -----------------------------
 * Utility macros
 * ----------------------------- */

/* Check whether a PID is invalid or refers to a free entry */
#define isbadpid(pid) \
    ((pid) < 0 || (pid) >= NPROC || proctab[(pid)].state == PR_FREE)


/* -----------------------------
 * Process manager API
 * ----------------------------- */

/* Initialize process table and create null process */
void process_init(void);

/* Create a new process */
int process_create(void (*entry)(void), const char *name);

/* Terminate the currently running process */
void process_exit(void);

/* Process utility functions */
int getpid(void);
int getpstate(int pid);
const char *getpname(int pid);

/* Blocking and wakeup */
void block_current(void);
int wakeup(int pid);

int set_priority(int pid, int prio);
int get_priority(int pid);

int send(int pid, msg_t msg);
msg_t receive(void);


#endif /* PROCESS_H */
