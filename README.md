# CFS Honored Assignment 242

## Overview

This project implements the Completely Fair Scheduler (CFS) for a simple operating system simulation. CFS is the default scheduler of the Linux kernel since version 2.6.23 and aims to maximize overall CPU utilization while providing a fair distribution of CPU time among processes.

## How CFS Works

The Completely Fair Scheduler operates on the principle of fair CPU time allocation using:

1. **Virtual Runtime (vruntime)**: A metric that represents the effective CPU time a process has consumed. Processes with lower vruntime are selected to run first.

2. **Red-Black Tree**: A self-balancing binary search tree that maintains processes ordered by their vruntime, ensuring O(log n) time complexity for operations.

3. **Nice Values**: Priority levels ranging from -20 (highest priority) to 19 (lowest priority) that determine how quickly a process accumulates vruntime.

4. **Dynamic Time Slices**: Unlike fixed time slices in traditional schedulers, CFS calculates time slices based on process weights and system load.

## Implementation Details

Our CFS implementation includes:

- Red-Black Tree data structure for efficient process management
- Weight-based scheduling using the Linux kernel's niceness-to-weight conversion
- vruntime calculation: `vruntime += (execution_time * 1024) / weight`
- Time slice calculation: `time_slice = (process_weight / total_weight) * target_latency`

## Test Cases

The implementation includes test processes with different niceness values:

- p0_cfs_nice_0: Default priority (niceness = 0, weight = 1024)
- p1_cfs_nice_neg10: High priority (niceness = -10, weight = 9548)
- p2_cfs_nice_pos10: Low priority (niceness = 10, weight = 110)
- p3_cfs_nice_neg20: Highest priority (niceness = -20, weight = 88761)
- p4_cfs_nice_pos19: Lowest priority (niceness = 19, weight = 15)

## Advantages Over MLQ

1. **Proportional Fairness**: CFS provides proportional CPU time based on priority, preventing starvation of low-priority processes.

2. **Efficient Performance**: O(log n) operations regardless of the number of priority levels.

3. **Adaptive Time Slices**: Time slices adjust based on system load and process priorities.

4. **Reduced Latency**: Dynamic time slice calculation results in lower average waiting times for interactive processes.

## How to Run

```bash
# Clean and build
make clean
make

# Run the CFS test
./os cfs_test > output_cfs.txt

# View results
cat output_cfs.txt
```

## Expected Results

When running the CFS test, you should observe that:
- Higher-priority processes (negative niceness) receive more CPU time
- Lower-priority processes (positive niceness) still get CPU time, just proportionally less
- The process with the lowest vruntime is always selected next
- vruntime accumulates more slowly for higher-priority processes

This demonstrates how CFS achieves both fairness and priority-based scheduling simultaneously, resolving the traditional tradeoff between throughput and responsiveness.