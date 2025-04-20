# I.Main notes
## 1. Memory
1.    Each process has multiple contiguous vm_areas or segments, where each has multiple regions inside. Each region acts as variables in the human-readable program's source code. 

2. MEMPHY can have the size much more smaller than the actual whole process size.

3. A page can be mapped to a frame in MEMRAM or MEMSWP, or null value

4. The assignment description say that the description file is in input directory with the following format:
    [time slice] [N = Number of CPU] [M = Number of Processes to be run]
    (Optional) [RAM SZ] [ SWP SZ 0 ] [ SWP SZ 1 ] [ SWP SZ 2 ] [ SWP SZ 3 ]
    [time 0] [path 0] [priority 0]
    [time 1] [path 1] [priority 1]
    ...
    [time M-1] [path M-1] [priority M-1]

    time slice: time slot
    time x: start time of a process

5. PTE stands for Page Table Entry

# II.Questions arised
## 1. Scheduler
    At each file, i types "???" for questions that i have not answered yet.

1. Why each process has its own ready/running queue?
    See more in common.h file

3. Running_list trong sched.c dùng để làm gì???
    Khác gì so với run_queue???

## 2. Memory
1. Each region points to the next region nearest to it. So does the symrgtbl[] will take care of all of them, or just care the first region in the chain? Also, does it take care free regions?

2. Does a region map to a page?     NO

4. I think that he forgot to add vmaid for vm_rg_struct, he want to commit it ever
ytime, such as in libmem.c, mm.c