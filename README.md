# CFS Honored Assignment 242

## Team Members
- Lê Nguyễn Kim Khôi - 2311671
- Hồ Anh Dũng - 2310543

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

### Using the run.sh Script
The easiest way to run all tests is to use the provided run.sh script:

```bash
# Make the script executable
chmod +x run.sh

# Run all tests (CFS tests, MLQ tests, and comparison)
./run.sh
```

### Manual Testing with CFS

```bash
# Clean and build with CFS scheduler enabled
cd ossim_sierra
# Edit include/os-cfg.h to enable CFS: uncomment #define CFS_SCHED 1
# Comment out MLQ_SCHED if it's enabled
make clean
make

# Run specific tests
./os input/cfs_test > ../output/cfs_output.log
./os input/cfs_high_load > ../output/cfs_high_load.log
./os input/cfs_full_niceness_range > ../output/cfs_niceness.log
```

### Manual Testing with MLQ

```bash
# Clean and build with MLQ scheduler enabled
cd ossim_sierra
# Edit include/os-cfg.h to enable MLQ: uncomment #define MLQ_SCHED 1
# Comment out CFS_SCHED if it's enabled
make clean
make

# Run the same tests with MLQ scheduler
./os input/cfs_test > ../output/mlq_output.log
```

### Running Scheduler Comparison

```bash
# Set up test environment
mkdir -p scheduler_comparison

# Run comparison script
./compare_schedulers.sh

# View comparison results
cat scheduler_comparison/comparison_report.txt

# View visualized comparison (requires matplotlib)
python3 visualize_comparison.py
```

## Expected Results

When running the CFS test, you should observe that:
- Higher-priority processes (negative niceness) receive more CPU time
- Lower-priority processes (positive niceness) still get CPU time, just proportionally less
- The process with the lowest vruntime is always selected next
- vruntime accumulates more slowly for higher-priority processes

The comparison with MLQ shows how CFS achieves both fairness and priority-based scheduling simultaneously, resolving the traditional tradeoff between throughput and responsiveness.

