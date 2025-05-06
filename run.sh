#!/bin/bash

# Sequentially run the test cases for ossim_sierra

# Define the list of test cases
test_cases=(
  "cfs_basic_fairness"
  "cfs_dynamic_workload"
  "cfs_extreme_imbalance"
  "cfs_full_niceness_range"
  "cfs_high_load"
  "cfs_multi_cpu"
  "cfs_priority_contrast"
  "cfs_staggered_arrival"
  "cfs_test"
)

# Navigate to the ossim_sierra directory
cd ossim_sierra || { echo "ossim_sierra directory not found!"; exit 1; }

# Build the project
make clean
make all

# # Run each test case
# for test in "${test_cases[@]}"; do
#   echo "Running test: $test"
#   ./os "$test"
#   echo "Finished test: $test"
#   echo "-------------------------"
# done
# Output the list of test cases
echo "The following test cases need to be checked:"
for test in "${test_cases[@]}"; do
  echo "- $test"
done

# Instructions for running individual tests
echo "To run each test individually, navigate to the ossim_sierra directory in the terminal, then execute:"
echo "1. make clean"
echo "2. make all"
echo "3. ./os [test name]"