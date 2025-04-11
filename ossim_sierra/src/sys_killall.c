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
    #ifdef MLQ_SCHED
        for (int mlq_index = 0; mlq_index < MAX_PRIO; ++mlq_index) {
            struct queue_t *ready_queue = &caller->mlq_ready_queue[mlq_index];
            int queue_index = 0;
            while (queue_index < ready_queue->size) {
                struct pcb_t *proc = ready_queue->proc[queue_index];
                int name_index = 0, path_index = 0;
                int same_name = 1;

                while (path_index < 100 && proc->path[path_index] != '\0') ++path_index; // Find the end of the string
                while (path_index >= 0 && proc->path[path_index] != '/') --path_index; // Find the last '/'
                ++path_index; // Move to the start of the process name

                while (proc_name[name_index] != '\0' && proc->path[path_index] != '\0') {
                    if (proc_name[name_index] != proc->path[path_index]) {
                        same_name = 0;
                        break;
                    }
                    ++name_index;
                    ++path_index;
                }
                if (proc_name[name_index] != '\0' || proc->path[path_index] != '\0') {
                    same_name = 0;
                }

                if (same_name == 1) {
                    remove_proc(ready_queue, queue_index);
                    if (proc->page_table != NULL) free(proc->page_table);
                    if (proc->code != NULL) {
                        if (proc->code->text != NULL) free(proc->code->text);
                        free(proc->code);
                    }
                    #ifdef MM_PAGING
                        if (proc->mm != NULL) {
                            // Delete the paging ??????????????
                            free(proc->mm);
                        }
                    #endif
                    free(proc);
                }
                else ++queue_index;
            }
        }
    #else
        int queue_index = 0;
        struct queue_t *running_list = caller->running_list;
        while (queue_index < running_list->size) {
            struct pcb_t *proc = running_list->proc[queue_index];
            int name_index = 0, path_index = 0;
            bool same_name = true;

            while (path_index < 100 && proc->path[path_index] != '\0') ++path_index; // Find the end of the string
            while (path_index >= 0 && proc->path[path_index] != '/') --path_index; // Find the last '/'
            ++path_index; // Move to the start of the process name

            while (proc_name[name_index] != '\0' && proc->path[path_index] != '\0') {
                if (proc_name[name_index] != proc->path[path_index]) {
                    same_name = false;
                    break;
                }
                ++name_index;
                ++path_index;
            }
            if (proc_name[name_index] != '\0' || proc->path[path_index] != '\0') {
                same_name = false;
            }

            if (same_name) {
                remove_proc(running_list, queue_index);
                if (proc->page_table != NULL) free(proc->page_table);
                if (proc->code != NULL) {
                    if (proc->code->text != NULL) free(proc->code->text);
                    free(proc->code);
                }
                #ifdef MM_PAGING
                    if (proc->mm != NULL) {
                        // Delete the paging ??????????????
                        free(proc->mm);
                    }
                #endif
                free(proc);
            }
            else ++queue_index;
        }
    #endif

    /* TODO Maching and terminating 
     *       all processes with given
     *        name in var proc_name
     */

    return 0; 
}
