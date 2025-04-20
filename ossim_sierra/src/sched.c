#include "queue.h"
#include "sched.h"
#include <pthread.h>
#ifdef CFS_SCHED
#include "rb_tree.h"
#include <math.h>
#endif
#include <stdlib.h>
#include <stdio.h>

// Common variables
static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;
static struct queue_t running_list;

// MLQ variables
#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
static int slot[MAX_PRIO];
#endif

// CFS variables
#ifdef CFS_SCHED
static struct rb_tree cfs_rq;
static unsigned long target_latency = 20; // 20ms target latency
static unsigned long total_weight = 0;    // Total weight of all tasks
static unsigned long min_vruntime = 0;    // System-wide minimum vruntime

// Niceness to weight conversion table (approximation of Linux's formula)
static const unsigned long sched_prio_to_weight[40] = {
    /* -20 */ 88761, 71755, 56483, 46273, 36291,
    /* -15 */ 29154, 23254, 18705, 14949, 11916,
    /* -10 */ 9548, 7620, 6100, 4904, 3906,
    /* -5 */ 3121, 2501, 1991, 1586, 1277,
    /* 0 */ 1024, 820, 655, 526, 423,
    /* 5 */ 335, 272, 215, 172, 137,
    /* 10 */ 110, 87, 70, 56, 45,
    /* 15 */ 36, 29, 23, 18, 15,
};
#endif

int queue_empty(void)
{
#ifdef MLQ_SCHED
    unsigned long prio;
    for (prio = 0; prio < MAX_PRIO; prio++)
        if (!empty(&mlq_ready_queue[prio]))
            return -1;
    return (empty(&ready_queue) && empty(&run_queue));
#endif
#ifdef CFS_SCHED
    return rb_empty(&cfs_rq) && empty(&ready_queue) && empty(&run_queue);
#endif
    return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void)
{
#ifdef MLQ_SCHED
    int i;
    for (i = 0; i < MAX_PRIO; i++)
    {
        mlq_ready_queue[i].size = 0;
        slot[i] = MAX_PRIO - i;
    }
#endif
#ifdef CFS_SCHED
    rb_init(&cfs_rq);
    total_weight = 0;
    min_vruntime = 0;
#endif
    ready_queue.size = 0;
    running_list.size = 0;
    run_queue.size = 0;
    pthread_mutex_init(&queue_lock, NULL);
}

// MLQ scheduler functions
#ifdef MLQ_SCHED
struct pcb_t *get_mlq_proc(void)
{
    // [MLQ implementation code...]
}

void put_mlq_proc(struct pcb_t *proc)
{
    pthread_mutex_lock(&queue_lock);
    enqueue(&mlq_ready_queue[proc->prio], proc);
    pthread_mutex_unlock(&queue_lock);
}

void add_mlq_proc(struct pcb_t *proc)
{
    pthread_mutex_lock(&queue_lock);
    enqueue(&mlq_ready_queue[proc->prio], proc);
    pthread_mutex_unlock(&queue_lock);
}
#endif

// CFS scheduler functions
#ifdef CFS_SCHED
// Initialize a process for CFS scheduling
static void init_cfs_proc(struct pcb_t *proc)
{
    // Set default niceness to 0 if not set
    if (proc->niceness < -20)
        proc->niceness = -20;
    else if (proc->niceness > 19)
        proc->niceness = 19;

    // Convert niceness to weight using the lookup table
    proc->weight = sched_prio_to_weight[proc->niceness + 20];

    // Set initial vruntime to current system minimum
    proc->vruntime = min_vruntime;

    // Update total weight
    total_weight += proc->weight;
}

// Update vruntime based on execution time
static void update_vruntime(struct pcb_t *proc, unsigned long execution_time)
{
    // vruntime += (execution_time * 1024) / weight
    // Scale by 1024 to maintain precision with integer division
    proc->vruntime += (execution_time * 1024) / proc->weight;

    // Update system-wide minimum vruntime
    if (proc->vruntime < min_vruntime)
        min_vruntime = proc->vruntime;
}

// Calculate time slice for a process based on its weight and total weight
unsigned long calc_time_slice(struct pcb_t *proc)
{
    // time_slice = (proc->weight / total_weight) * target_latency
    if (total_weight == 0)
        return 1; // Fallback to 1 time unit if no processes

    unsigned long time_slice = (proc->weight * target_latency) / total_weight;
    if (time_slice < 1)
        time_slice = 1; // Minimum time slice of 1 time unit

    return time_slice;
}

struct pcb_t *get_cfs_proc(void)
{
    pthread_mutex_lock(&queue_lock);

    // Get the process with the smallest vruntime from the RB tree
    struct pcb_t *proc = rb_extract_min(&cfs_rq);

    if (proc != NULL)
    {
        // Update total weight when removing a process
        total_weight -= proc->weight;
    }

    pthread_mutex_unlock(&queue_lock);
    return proc;
}

void put_cfs_proc(struct pcb_t *proc)
{
    pthread_mutex_lock(&queue_lock);

    // Update vruntime based on how long the process executed
    // For simplicity, assume it executed for 1 time slice
    update_vruntime(proc, 1);

    // Re-insert the process into the RB tree
    rb_insert(&cfs_rq, proc);

    // Update total weight when adding a process back
    total_weight += proc->weight;

    pthread_mutex_unlock(&queue_lock);
}

void add_cfs_proc(struct pcb_t *proc)
{
    pthread_mutex_lock(&queue_lock);

    // Initialize the process for CFS scheduling
    init_cfs_proc(proc);

    // Insert the process into the RB tree
    rb_insert(&cfs_rq, proc);

    pthread_mutex_unlock(&queue_lock);
}
#endif

// Generic scheduler functions that dispatch to the active scheduler
struct pcb_t *get_proc(void)
{
#ifdef MLQ_SCHED
    return get_mlq_proc();
#endif

#ifdef CFS_SCHED
    return get_cfs_proc();
#endif

    // Default scheduler
    struct pcb_t *proc = NULL;
    pthread_mutex_lock(&queue_lock);
    proc = dequeue(&ready_queue);
    pthread_mutex_unlock(&queue_lock);
    return proc;
}

void put_proc(struct pcb_t *proc)
{
    proc->ready_queue = &ready_queue;
#ifdef MLQ_SCHED
    proc->mlq_ready_queue = mlq_ready_queue;
#endif
    proc->running_list = &running_list;

    pthread_mutex_lock(&queue_lock);
    enqueue(&running_list, proc);
    pthread_mutex_unlock(&queue_lock);

#ifdef MLQ_SCHED
    put_mlq_proc(proc);
    return;
#endif

#ifdef CFS_SCHED
    put_cfs_proc(proc);
    return;
#endif

    pthread_mutex_lock(&queue_lock);
    enqueue(&ready_queue, proc);
    pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t *proc)
{
    proc->ready_queue = &ready_queue;
#ifdef MLQ_SCHED
    proc->mlq_ready_queue = mlq_ready_queue;
#endif
    proc->running_list = &running_list;

    pthread_mutex_lock(&queue_lock);
    enqueue(&running_list, proc);
    pthread_mutex_unlock(&queue_lock);

#ifdef MLQ_SCHED
    add_mlq_proc(proc);
    return;
#endif

#ifdef CFS_SCHED
    add_cfs_proc(proc);
    return;
#endif

    pthread_mutex_lock(&queue_lock);
    enqueue(&ready_queue, proc);
    pthread_mutex_unlock(&queue_lock);
}