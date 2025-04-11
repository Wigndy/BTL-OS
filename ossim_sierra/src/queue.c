#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if (q->size >= MAX_QUEUE_SIZE) {
                printf("max sizee queue");
                return;
        }
        q->proc[q->size] = proc;
        q->size++;
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
        if (q == NULL || q->size == 0) return NULL;
        struct pcb_t *res = NULL;
        uint32_t highest_prio = 999999, proc_prio;
        int key = q->size;
        // Find pcb has highest prio -> res, highest_prio; key to remove that pcb from q
        for (int i = 0, size = q->size; i < size; ++i) {
                if (q->proc[i] == NULL) continue;
        #ifdef MLQ_SCHED
                proc_prio = q->proc[i]->prio;
        #else
                proc_prio = q->proc[i]->priority;
        #endif
                if (proc_prio < highest_prio) {
                        highest_prio = proc_prio;
                        key = i;
                        res = q->proc[i];
                }
        }

        for(int i = key + 1, size = q->size; i < size; ++i)
                q->proc[i - 1] = q->proc[i];
        if (res != NULL) q->size--;
	return res;
}

void remove_proc(struct queue_t * q, int index) {
        for (int i = index + 1; i < q->size; ++i) {
                q->proc[i - 1] = q->proc[i];
        }
        q->size--;
}