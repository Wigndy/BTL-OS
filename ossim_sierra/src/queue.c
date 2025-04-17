#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if (q && q->size < MAX_QUEUE_SIZE)
                q->proc[q->size++] = proc;
}

struct pcb_t * dequeue(struct queue_t * q) {
        /* TODO: return a pcb whose prioprity is the highest
         * in the queue [q] and remember to remove it from q
         * */
        if (empty(q))
                return NULL;
        int best_idx = 0;
        #ifndef MLQ_SCHED
        for (int i = 1; i != q->size; ++i) {
                if (q->proc[i]->priority < q->proc[best_idx]->priority) {
                        best_idx = i;
                }
        }
        #endif

        struct pcb_t * selected_proc = q->proc[best_idx];

        for (int i = best_idx; i < q->size - 1; ++i) {
                q->proc[i] = q->proc[i+1];
        }

        --q->size;

        return selected_proc;
}

