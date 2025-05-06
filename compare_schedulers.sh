#!/bin/bash
# filepath: c:\Users\HAD\OneDrive\Computer\OS\project\BTL-OS\compare_schedulers.sh

OUTPUT_DIR="scheduler_comparison"
mkdir -p $OUTPUT_DIR

echo "====== SCHEDULER COMPARISON TEST ======"
echo "Starting comparison at $(date)"

# First create test files
./create_test_files.sh

# Function to extract metrics from output files
extract_metrics() {
    local scheduler=$1
    local output_file=$2
    
    # Calculate process waiting times and turnaround times
    python3 ./extract_metrics.py $output_file > "${OUTPUT_DIR}/${scheduler}_metrics.txt"
    
    # Print summary
    echo "Results for $scheduler:"
    cat "${OUTPUT_DIR}/${scheduler}_metrics.txt"
    echo ""
}

# Test CFS Scheduler
echo "Testing CFS Scheduler..."
cd ossim_sierra
# Ensure CFS is enabled and MLQ is disabled
sed -i 's/\/\/ #define CFS_SCHED/#define CFS_SCHED/' include/os-cfg.h
sed -i 's/#define MLQ_SCHED/\/\/ #define MLQ_SCHED/' include/os-cfg.h
sed -i 's/\/\/ #define TEST_COMPARISON/#define TEST_COMPARISON/' include/os-cfg.h
cat include/os-cfg.h
# Build with CFS
make clean
make

# Run tests
echo "Running CFS tests..."
./os scheduler_comparison.txt > "../${OUTPUT_DIR}/cfs_output.log"

# Extract metrics
cd ..
extract_metrics "CFS" "${OUTPUT_DIR}/cfs_output.log"

# Test MLQ Scheduler
echo "Testing MLQ Scheduler..."
cd ossim_sierra
# Switch to MLQ - disable CFS
sed -i 's/#define CFS_SCHED/\/\/ #define CFS_SCHED/' include/os-cfg.h
# Remove all existing MLQ_SCHED lines to avoid duplicates
sed -i '/.*#define MLQ_SCHED.*/d' include/os-cfg.h
# Add MLQ_SCHED definition at the top of the file, after the header guard
sed -i '/#define OSCFG_H/a #define MLQ_SCHED 1' include/os-cfg.h
sed -i 's/\/\/ #define TEST_COMPARISON/#define TEST_COMPARISON/' include/os-cfg.h
cat include/os-cfg.h
# Build with MLQ
make clean
make
# Run tests
echo "Running MLQ tests..."
./os scheduler_comparison.txt > "../${OUTPUT_DIR}/mlq_output.log"

# Extract metrics
cd ..
extract_metrics "MLQ" "${OUTPUT_DIR}/mlq_output.log"

# Generate comparison report
echo "Generating comparison report..."
python3 ./compare_metrics.py "${OUTPUT_DIR}/cfs_metrics.txt" "${OUTPUT_DIR}/mlq_metrics.txt" > "${OUTPUT_DIR}/comparison_report.txt"

echo "Comparison complete. Results saved in ${OUTPUT_DIR}/comparison_report.txt"
echo "Generating visualization charts..."

# Create visualization of the results
python3 ./visualize_comparison.py "${OUTPUT_DIR}/cfs_metrics.txt" "${OUTPUT_DIR}/mlq_metrics.txt" "${OUTPUT_DIR}/comparison_charts"

echo "All tests complete!"
# Make sure we're in the right directory for the final cleanup
cd ossim_sierra 2>/dev/null || true
sed -i 's/#define TEST_COMPARISON/\/\/ #define TEST_COMPARISON/' include/os-cfg.h
cd ..