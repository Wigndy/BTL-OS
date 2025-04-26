#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
        if (q == NULL) return 1;
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
        /* TODO: put a new process to queue [q] */
        if (q == NULL || proc == NULL || q->size >= MAX_QUEUE_SIZE) {
                printf("Queue is full or NULL\n");
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
        
        int MAX_INDEX = 0;
        #ifdef MLQ_SCHED
                uint32_t MAX = q->proc[0]->prio;
                for (int i = 1; i < q->size; i++) {
                        if (q->proc[i]->prio < MAX) {
                                MAX = q->proc[i]->prio;
                                MAX_INDEX = i;
                        }
                }
        #else
                uint32_t MAX = q->proc[0]->priority;
                for (int i = 1; i < q->size; i++) {
                        if (q->proc[i]->priority < MAX) {
                                MAX = q->proc[i]->priority;
                                MAX_INDEX = i;
                        }
                }
        #endif
        struct pcb_t * ret_proc = q->proc[MAX_INDEX];
        for (int i = MAX_INDEX; i < q->size - 1; i++) {
                q->proc[i] = q->proc[i + 1];
        }
        q->size--;
        return ret_proc;
}

void remove_proc(struct queue_t * q, struct pcb_t * proc) {
        if (q == NULL || proc == NULL) return;
        for (int i = 0; i < q->size; ++i) {
                if (q->proc[i] == proc) {
                        for (int j = i; j < q->size - 1; j++) {
                                q->proc[j] = q->proc[j + 1];
                        }
                        q->size--;
                        return;
                }
        }
}