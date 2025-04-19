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
    //caller->mlq_ready_queue    
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

    // if (same_name) {
    //     remove_proc(running_list, queue_index);
    //     if (proc->page_table != NULL) free(proc->page_table);
    //     if (proc->code != NULL) {
    //         if (proc->code->text != NULL) free(proc->code->text);
    //         free(proc->code);
    //     }
    //     #ifdef MM_PAGING
    //         if (proc->mm != NULL) {
    //             // Delete the paging ??????????????
    //             free(proc->mm);
    //         }
    //     #endif
    //     free(proc);
    // }
    return killed_count; // Expected output
}
