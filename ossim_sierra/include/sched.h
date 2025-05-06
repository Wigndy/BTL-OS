#ifndef QUEUE_H
#define QUEUE_H

#include "common.h"

#define MAX_PRIO 140

int queue_empty(void);

void init_scheduler(int time_slot);
void finish_scheduler(void);

/* Get the next process from ready queue */
struct pcb_t * get_proc(void);

/* Put a process back to run queue */
void put_proc(struct pcb_t * proc);

/* Add a new process to ready queue */
void add_proc(struct pcb_t * proc);

#ifdef CFS_SCHED
/* Calculate time slice for a process based on its weight */
unsigned long calc_time_slice(struct pcb_t *proc);
#endif

#endif