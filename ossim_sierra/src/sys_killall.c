/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

#include "common.h"
#include "syscall.h"
#include "stdio.h"
#include "libmem.h"

int __sys_killall(struct pcb_t *caller, struct sc_regs* regs)
{
    char proc_name[100];
    uint32_t data;

    //hardcode for demo only
    uint32_t memrg = regs->a1;
    
    /* TODO: Get name of the target proc */
    //proc_name = libread..
    int i = 0;
    data = 0;
    while(data != -1){
        libread(caller, memrg, i, &data);
        proc_name[i]= data;
        if(data == -1) proc_name[i]='\0';
        i++;
    }
    printf("The procname retrieved from memregionid %d is \"%s\"\n", memrg, proc_name);

    /* TODO: Traverse proclist to terminate the proc
     *       stcmp to check the process match proc_name
     */
    //caller->running_list
    //caller->mlq_ready_queu

    /* TODO Maching and terminating 
     *       all processes with given
     *        name in var proc_name
     */
    int killed_count = 0;
    
    struct queue_t *running = caller->running_list;
    if (running != NULL) {
        // Create a new queue for processes we want to keep
        struct queue_t new_running;
        new_running.size = 0;
        
        // Check each process in running_list
        for (i = 0; i < running->size; i++) {
            struct pcb_t *proc = running->proc[i];
            
            // Check if this process matches our target name
            if (proc != NULL && strcmp(proc->name, proc_name) == 0) {
                // This is a process we want to terminate
                printf("Terminating process %s (PID: %d) from running list\n", 
                    proc->name, proc->pid);
                killed_count++;
                
                // Free process resources
                // In a real OS we'd need to free memory, close files, etc.
                // For this simulator, we'll just free the PCB if needed
                // free(proc);  // Uncomment if the OS simulator manages memory this way
            } else {
                // Keep this process by adding it to the new queue
                new_running.proc[new_running.size++] = proc;
            }
        }
        
        // Replace the old queue with the new one
        running->size = new_running.size;
        for (i = 0; i < new_running.size; i++) {
            running->proc[i] = new_running.proc[i];
        }
    }

#ifdef MLQ_SCHED
    /* Traverse MLQ ready queues to find and terminate matching processes */
    if (caller->mlq_ready_queue != NULL) {
        for (int prio = 0; prio < MAX_PRIO; prio++) {
            struct queue_t *queue = &caller->mlq_ready_queue[prio];
            struct queue_t new_queue;
            new_queue.size = 0;
            
            for (i = 0; i < queue->size; i++) {
                struct pcb_t *proc = queue->proc[i];
                
                if (proc != NULL && strcmp(proc->name, proc_name) == 0) {
                    printf("Terminating process %s (PID: %d) from MLQ priority %d\n", 
                        proc->name, proc->pid, prio);
                    killed_count++;
                    // Free process resources if needed
                    // free(proc);
                } else {
                    new_queue.proc[new_queue.size++] = proc;
                }
            }
            
            // Replace the old queue with the new one
            queue->size = new_queue.size;
            for (i = 0; i < new_queue.size; i++) {
                queue->proc[i] = new_queue.proc[i];
            }
        }
    }
#endif

    return killed_count;
    return 0; 
}
 