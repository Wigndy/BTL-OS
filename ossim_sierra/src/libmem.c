/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

// #ifdef MM_PAGING
/*
 * System Library
 * Memory Module Library libmem.c 
 */

#include "string.h"
#include "mm.h"
#include "syscall.h"
#include "libmem.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

static pthread_mutex_t mmvm_lock = PTHREAD_MUTEX_INITIALIZER;

/*enlist_vm_freerg_list - add new rg to freerg_list
 *@mm: memory region
 *@rg_elmt: new region
 *
 */
int enlist_vm_freerg_list(struct mm_struct *mm, struct vm_rg_struct *rg_elmt)
{
  struct vm_rg_struct *rg_node = mm->mmap->vm_freerg_list;

  if (rg_elmt->rg_start >= rg_elmt->rg_end)
    return -1;

  if (rg_node != NULL)
    rg_elmt->rg_next = rg_node;

  /* Enlist the new region */
  mm->mmap->vm_freerg_list = rg_elmt;

  return 0;
}

/*get_symrg_byid - get mem region by region ID
 *@mm: memory region
 *@rgid: region ID act as symbol index of variable
 *
 */
struct vm_rg_struct *get_symrg_byid(struct mm_struct *mm, int rgid)
{
  if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return NULL;

  return &mm->symrgtbl[rgid];
}

/*__alloc - allocate a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *@alloc_addr: address of allocated memory region
 *
 */
int __alloc(struct pcb_t *caller, int vmaid, int rgid, int size, int *alloc_addr)
{
  if (rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return -1;
  
  pthread_mutex_lock(&mmvm_lock);
  /*Allocate at the toproof */
  struct vm_rg_struct rgnode;

  /* TODO: commit the vmaid */    // ???
  // rgnode.vmaid

  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
  {
    caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
    caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
 
    *alloc_addr = rgnode.rg_start;

    pthread_mutex_unlock(&mmvm_lock);
    return 0;
  }

  /* TODO get_free_vmrg_area FAILED handle the region management (Fig.6)*/

  /* TODO retrive current vma if needed, current comment out due to compiler redundant warning*/
  /*Attempt to increate limit to get space */
  struct vm_area_struct *curr_vma = get_vma_by_num(caller->mm, vmaid);
  if (curr_vma == NULL)
  {
    pthread_mutex_unlock(&mmvm_lock);
    return -1;  
  }


  /* TODO retrive old_sbrk if needed, current comment out due to compiler redundant warning*/
  //int old_sbrk = curr_vma->sbrk;
  int inc_sz = PAGING_PAGE_ALIGNSZ(size);
  //int inc_limit_ret = old_sbrk + inc_sz; 

  /* TODO INCREASE THE LIMIT as inovking systemcall
   * sys_memap with SYSMEM_INC_OP
   */
  struct sc_regs regs;
  regs.a1 = SYSMEM_INC_OP;
  regs.a2 = vmaid;
  regs.a3 = inc_sz;

  /* SYSCALL 17 sys_memmap */
  syscall(caller, 17, &regs);

  /* TODO: commit the limit increment */
    // curr_vma->sbrk = inc_limit_ret;

    // caller->mm->symrgtbl[rgid].rg_start = old_sbrk;
    // caller->mm->symrgtbl[rgid].rg_end = inc_limit_ret;
  /* TODO: commit the allocation address */
  //*alloc_addr = old_sbrk;


  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
  {
    caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
    caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
 
    *alloc_addr = rgnode.rg_start;

    pthread_mutex_unlock(&mmvm_lock);
    return 0;
  }

  pthread_mutex_unlock(&mmvm_lock);
  return -1;

}

/*__free - remove a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __free(struct pcb_t *caller, int vmaid, int rgid)
{
  struct vm_rg_struct *rgnode;

  // Dummy initialization for avoding compiler dummay warning
  // in incompleted TODO code rgnode will overwrite through implementing
  // the manipulation of rgid later

  if(rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return -1;

  pthread_mutex_lock(&mmvm_lock);
  /* TODO: Manage the collect freed region to freerg_list */
  rgnode = get_symrg_byid(caller->mm, rgid);
  if (rgnode->rg_start >= rgnode->rg_end) // Freed up
  {
    pthread_mutex_unlock(&mmvm_lock); 
    return -1;
  }
  
  /*enlist the obsolet ed memory region */
  enlist_vm_freerg_list(caller->mm, rgnode);
  //rgnode->rg_start = rgnode->rg_end = 0;

  // printf("* FOR DEBUG: Here are range from free regions:\n");
  // for( struct vm_rg_struct *free_rg = caller->mm->mmap->vm_freerg_list; free_rg; free_rg = free_rg->rg_next)
  // {
  //   printf("*\t%ld-%ld\n", free_rg->rg_start, free_rg->rg_end);
  // }

  pthread_mutex_unlock(&mmvm_lock);
  return 0;
}

/*liballoc - PAGING-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int liballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  /* TODO Implement allocation on vm area 0 */
  int addr;
  int ret = __alloc(proc, 0, reg_index, size, &addr);

  printf("===== PHYSICAL MEMORY AFTER ALLOCATION =====\n");
  printf("PID=%d - Region=%d - Address=%08x - Size=%ld byte\n",
     proc->pid, reg_index, addr, 
     proc->mm->symrgtbl[reg_index].rg_end - proc->mm->symrgtbl[reg_index].rg_start); // I'm not using size for checking the correctness of the program
  print_pgtbl(proc, 0, -1);
  printf("================================================================\n");
  return ret;
}

/*libfree - PAGING-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */

int libfree(struct pcb_t *proc, uint32_t reg_index)
{
  /* TODO Implement free region */

  /* By default using vmaid = 0 */
  int ret = __free(proc, 0, reg_index);
  
  printf("===== PHYSICAL MEMORY AFTER DEALLOCATION =====\n");
  printf("PID=%d - Region=%ld - Size=%ld byte\n",
     proc->pid, proc->mm->mmap->vm_id,  
     proc->mm->symrgtbl[reg_index].rg_end - proc->mm->symrgtbl[reg_index].rg_start);
  
  print_pgtbl(proc, 0, -1);
  printf("================================================================\n");
  return ret;
}

/*pg_getpage - get the page in ram
 *@mm: memory region
 *@pagenum: PGN
 *@framenum: return FPN
 *@caller: caller
 *
 */
int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
{
  uint32_t pte = mm->pgd[pgn];

  if (!PAGING_PAGE_PRESENT(pte))
  { /* Page is not online, make it actively living */
    int vicpgn, swpfpn; 
    int vicfpn;
    uint32_t *vicpte;


    int tgtfpn = PAGING_PTE_SWP(pte);//the target frame storing our variable

    /* TODO: Play with your paging theory here */
    /* Find victim page */
    if( find_victim_page(caller->mm, &vicpgn) == -1) // Why did we not find the free frame in memram first???
      return -1;

    /* Get free frame in MEMSWP */
    if( MEMPHY_get_freefp(caller->active_mswp, &swpfpn) == -1)
      return -1;

    /* TODO: Implement swap frame from MEMRAM to MEMSWP and vice versa*/

    /* TODO copy victim frame to swap 
     * SWP(vicfpn <--> swpfpn)
     * SYSCALL 17 sys_memmap 
     * with operation SYSMEM_SWP_OP
     */
    vicpte = &mm->pgd[vicpgn];
    vicfpn = PAGING_FPN(*vicpte);

    struct sc_regs regs;
    regs.a1 = SYSMEM_SWP_OP;
    regs.a2 = vicfpn;
    regs.a3 = swpfpn;

    /* SYSCALL 17 sys_memmap */
    syscall(caller, 17, &regs);

    /* TODO copy target frame form swap to mem 
     * SWP(tgtfpn <--> vicfpn)
     * SYSCALL 17 sys_memmap
     * with operation SYSMEM_SWP_OP
     */
    /* TODO copy target frame form swap to mem 
    //regs.a1 =...
    //regs.a2 =...
    //regs.a3 =..
    */

    /* SYSCALL 17 sys_memmap */
    __swap_cp_page(caller->active_mswp, tgtfpn, caller->mram, vicfpn);

    /* Update page table */
    //pte_set_swap() 
    //mm->pgd;

    /* Update its online status of the target page */
    //pte_set_fpn() &
    //mm->pgd[pgn];
    //pte_set_fpn();
    pte_set_swap(&mm->pgd[vicpgn], caller->active_mswp_id, tgtfpn);
    pte_set_fpn(&caller->mm->pgd[pgn], vicfpn);

    enlist_pgn_node(&caller->mm->fifo_pgn,pgn);
  }

  *fpn = PAGING_FPN(mm->pgd[pgn]);

  return 0;
}

/*pg_getval - read value at given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if (pg_getpage(mm, pgn, &fpn, caller) != 0)
    return -1; /* invalid page access */

  /* TODO 
   *  MEMPHY_read(caller->mram, phyaddr, data);
   *  MEMPHY READ 
   *  SYSCALL 17 sys_memmap with SYSMEM_IO_READ
   */
  int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off; // Not sure bro ??? I see people use this: (fpn << PAGING_ADDR_FPN_LOBIT) + off;
  struct sc_regs regs;
  regs.a1 = SYSMEM_IO_READ;
  regs.a2 = phyaddr;
  regs.a3 = data;

  /* SYSCALL 17 sys_memmap */
  syscall(caller, 17, &regs);

  // Update data
  // data = (BYTE)
  data = (BYTE)regs.a3;

  return 0;
}

/*pg_setval - write value to given offset
 *@mm: memory region
 *@addr: virtual address to acess
 *@value: value
 *
 */
int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr); 
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if (pg_getpage(mm, pgn, &fpn, caller) != 0)
    return -1; /* invalid page access */

  /* TODO
   *  MEMPHY_write(caller->mram, phyaddr, value);
   *  MEMPHY WRITE
   *  SYSCALL 17 sys_memmap with SYSMEM_IO_WRITE
   */
  int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off; // But why not fpn * PAGING_PGSZ???
  struct sc_regs regs;
  regs.a1 = SYSMEM_IO_WRITE;
  regs.a2 = phyaddr;
  regs.a3 = value;

  /* SYSCALL 17 sys_memmap */
  syscall(caller, 17, &regs);

  // Update data
  // data = (BYTE) 

  return 0;
}

/*__read - read value in region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __read(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE *data)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
    return -1;

  pg_getval(caller->mm, currg->rg_start + offset, data, caller);

  return 0;
}

/*libread - PAGING-based read a region memory */
int libread(
    struct pcb_t *proc, // Process executing the instruction
    uint32_t source,    // Index of source register
    uint32_t offset,    // Source address = [source] + [offset]
    uint32_t* destination)
{
  BYTE data;
  int val = __read(proc, 0, source, offset, &data);

  /* TODO update result of reading action*/
  *destination = (uint32_t)data; // Is this right???
  // destination
  printf("================================================================\n");
  printf("===== PHYSICAL MEMORY AFTER READING =====\n");
#ifdef IODUMP
      printf("read region=%d offset=%d value=%d\n", source, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif
  printf("================================================================\n");
  return val;
}

/*__write - write a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size
 *
 */
int __write(struct pcb_t *caller, int vmaid, int rgid, int offset, BYTE value)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if (currg == NULL || cur_vma == NULL) /* Invalid memory identify */
    return -1;

  pg_setval(caller->mm, currg->rg_start + offset, value, caller);

  return 0;
}

/*libwrite - PAGING-based write a region memory */
int libwrite(
    struct pcb_t *proc,   // Process executing the instruction
    BYTE data,            // Data to be wrttien into memory
    uint32_t destination, // Index of destination register
    uint32_t offset)
{
  printf("===== PHYSICAL MEMORY AFTER WRITING =====\n");
  int ret = __write(proc, 0, destination, offset, data);  // I changed it to this location to write before printing
#ifdef IODUMP
  printf("write region=%d offset=%d value=%d\n", destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif
  printf("================================================================\n");
  return ret;
}

/*free_pcb_memphy - collect all memphy of pcb
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 */
int free_pcb_memph(struct pcb_t *caller)
{
  int pagenum, fpn;
  uint32_t pte;


  for(pagenum = 0; pagenum < PAGING_MAX_PGN; pagenum++)
  {
    pte= caller->mm->pgd[pagenum];

    if (!PAGING_PAGE_PRESENT(pte))
    {
      fpn = PAGING_PTE_FPN(pte);
      MEMPHY_put_freefp(caller->mram, fpn);
    } else {
      fpn = PAGING_PTE_SWP(pte);
      MEMPHY_put_freefp(caller->active_mswp, fpn);    
    }
  }

  return 0;
}


/*find_victim_page - find victim page
 *@caller: caller
 *@pgn: return page number
 *
 */
int find_victim_page(struct mm_struct *mm, int *retpgn)
{
  struct pgn_t *pg = mm->fifo_pgn;
  if(pg == NULL)
  {
    return -1;
  }
  /* TODO: Implement the theorical mechanism to find the victim page */

  struct pgn_t *temp = NULL;
  while(pg->pg_next)
  {
    temp = pg;
    pg = pg->pg_next;
  }

  if(temp != NULL)
  {
    temp->pg_next = NULL;
  }
  else
  {
    mm->fifo_pgn = NULL;
  }
  *retpgn = pg->pgn;

  free(pg);

  return 0;
}

/*get_free_vmrg_area - get a free vm region
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@size: allocated size
 *
 */
int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
{
  /*
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  if(cur_vma == NULL)
    return -1;

  struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;

  if (rgit == NULL)
    return -1;
  */
  
  /* Probe unintialized newrg */
  // newrg->rg_start = newrg->rg_end = -1;

  /* TODO Traverse on list of free vm region to find a fit space */
  //while (...)
  // ..

  /*
  // Find best-fit region
  struct vm_rg_struct *rgit_hold = NULL; // To hold the best-fit region
  for (; rgit; rgit = rgit->rg_next)
  {
    unsigned long rgit_size = (rgit->rg_end - rgit->rg_start);    

    if (size < rgit_size)
    {
      // Initial or smaller region
      if (rgit_hold == NULL || rgit_size < (rgit_hold->rg_end - rgit_hold->rg_start))
      {
        rgit_hold = rgit;
      }
    }
    else if (size == rgit_size) // Equal: best fit
    {
      rgit_hold = rgit;
      break;
    }
  }

  if(rgit_hold == NULL)
  {
    printf("ERROR: libmem.c/get_free_vmrg_area(): Can not find out a region to fit in.\n");
    return -1;
  }

  newrg->rg_start = rgit_hold->rg_start;
  newrg->rg_end = rgit_hold->rg_start + size;

  struct vm_rg_struct **p_rgit_hold = &rgit_hold;
  if(size == rgit_hold->rg_end - rgit_hold->rg_start)
  {
    *p_rgit_hold = rgit_hold->rg_next;
    free(rgit_hold);
  }
  else
  {
    rgit_hold->rg_start += size; // or size + 1???
  }

  return 0;
  */

  struct vm_area_struct * vma = get_vma_by_num(caller->mm, vmaid);
  if (vma == NULL)
  {
    printf("ERROR: libmem.c/get_free_vmrg_area(): vma pointer is null\n");
    return -1;
  }
  for (struct vm_rg_struct ** p_rgit = &vma->vm_freerg_list; *p_rgit; p_rgit = &((*p_rgit)->rg_next)) {
    struct vm_rg_struct * rgit = *p_rgit;
    if (rgit->rg_start + size < rgit->rg_end) {
      newrg->rg_start = rgit->rg_start;
      newrg->rg_end = rgit->rg_start += size;
      return 0;
    }
    if (rgit->rg_start + size == rgit->rg_end) {
      newrg->rg_start = rgit->rg_start;
      newrg->rg_end = rgit->rg_end;
      *p_rgit = rgit->rg_next;
      free(rgit);
      return 0;
    }
  }

  printf("ERROR: libmem.c/get_free_vmrg_area(): Can not find out a region to fit in.\n");
  return -1;
}

//#endif
