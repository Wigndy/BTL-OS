#include "common.h"
#include "syscall.h"
#include "stdio.h"

int __sys_xxxhandler(struct pcb_t *caller, struct sc_regs *regs)
{
    /* Implementation of the system call */
    printf("The first system call parameter %d\n", regs->a1);
    return 0;
}