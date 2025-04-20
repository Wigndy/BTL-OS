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
#include "queue.h" //????????????????
#include "stdlib.h" //????????????????
#include <string.h>
int check_name(char *proc_name, char *path)
{
    int name_index = 0, path_index = 0;
    int same_name = 1;

    while (path_index < 100 && path[path_index] != '\0') ++path_index; // Find the end of the string
    while (path_index >= 0 && path[path_index] != '/') --path_index; // Find the last '/'
    ++path_index; // Move to the start of the process name

    while (proc_name[name_index] != '\0' && path[path_index] != '\0') {
        if (proc_name[name_index] != path[path_index]) {
            same_name = 0;
            break;
        }
        ++name_index;
        ++path_index;
    }
    if (proc_name[name_index] != '\0' || path[path_index] != '\0') {
        same_name = 0;
    }

    return same_name;
}

int kill_in_queue(struct queue_t *proc_queue, char *proc_name)
{
    struct queue_t new_queue;
    new_queue.size = 0;
    int count = 0;
    
    for (int i = 0; i < proc_queue->size; ++i) {
        struct pcb_t *proc = proc_queue->proc[i];
        if (proc == NULL) continue;

        int same_name = check_name(proc_name, proc->path);

        if (same_name == 1) {
            printf("Process %s killed\n", proc_name);
            // free ?????????
            free(proc);
            ++count;
        }
        else {
            new_queue.proc[new_queue.size] = proc;
            new_queue.size++;
        }
    }

    proc_queue->size = new_queue.size;
    for (int i = 0; i < new_queue.size; ++i) {
        proc_queue->proc[i] = new_queue.proc[i];
    }

    return count;
}

int __sys_killall(struct pcb_t *caller, struct sc_regs* regs)
{
    char proc_name[100];

    //hardcode for demo only
    uint32_t memrg = regs->a1;
    
    int i = 0;
    int read_result;
    uint32_t data = 0;
    
    // Initialize proc_name to empty string
    proc_name[0] = '\0';
    
    // Read memory region until terminator or max length
    while(i < 99) {

        read_result = libread(caller, memrg, i, &data);
        
        // Check for read errors or terminator
        if (read_result != 0 || data == -1) {
            break;  // Stop reading on any error or when terminator found
        }
        
        // Store the character and move to next position
        proc_name[i] = data;
        i++;
    }
    
    // Ensure the string is properly null-terminated
    proc_name[i] = '\0';
    printf("The procname retrieved from memregionid %d is \"%s\"\n", memrg, proc_name);  
    int killed_count = 0; // Expected output

    struct queue_t *running_list = caller->running_list;
    if (running_list != NULL) {
        killed_count += kill_in_queue(running_list, proc_name);
    }

    #ifdef MLQ_SCHED
        struct queue_t *mlq_ready_queue = caller->mlq_ready_queue;
        if (mlq_ready_queue != NULL) {
            for (int i = 0; i < mlq_ready_queue->size; ++i) {
                struct queue_t *queue = &(mlq_ready_queue[i]);
                if (queue != NULL) {
                    killed_count += kill_in_queue(queue, proc_name);
                }
            }
        }
    #else
        struct queue_t *ready_queue = caller->ready_queue;
        if (ready_queue != NULL) {
            killed_count += kill_in_queue(ready_queue, proc_name);
        }
    #endif


    /* TODO Maching and terminating 
     *       all processes with given
     *        name in var proc_name
     */
    
    struct queue_t *running = caller->running_list;
    if (running != NULL) {
        // Create a new queue for processes we want to keep
        struct queue_t new_running;
        new_running.size = 0;
        
        // Check each process in running_list
        for (i = 0; i < running->size; i++) {
            struct pcb_t *proc = running->proc[i];
            
            // Check if this process matches our target name
            if (proc != NULL && check_name(proc_name, proc->path)) {
                // This is a process we want to terminate
                printf("Terminating process %s (PID: %d) from running list\n", 
                    proc_name, proc->pid);
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

    return killed_count; // Expected output
}
 