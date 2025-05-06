#!/bin/bash
# filepath: c:\Users\HAD\OneDrive\Computer\OS\project\BTL-OS\create_test_files.sh

echo "Creating compatible test files for CFS and MLQ comparison..."
mkdir -p ossim_sierra/input/proc/comparison

# Create test files with compatible priority/niceness values
# Format: priority execution_steps

# High priority process (MLQ: 19, CFS: 0)
cat > ossim_sierra/input/proc/comparison/proc_high.txt << EOF
0 5
alloc 300 0
calc
write 0 20 100
read 0 20 1
free 0
EOF

# Medium-high priority process (MLQ: 14, CFS: 5)
cat > ossim_sierra/input/proc/comparison/proc_med_high.txt << EOF
5 5
alloc 300 0
calc
write 0 20 101
read 0 20 1
free 0
EOF

# Medium priority process (MLQ: 10, CFS: 10)
cat > ossim_sierra/input/proc/comparison/proc_medium.txt << EOF
10 5
alloc 300 0
calc
write 0 20 102
read 0 20 1
free 0
EOF

# Medium-low priority process (MLQ: 5, CFS: 15)
cat > ossim_sierra/input/proc/comparison/proc_med_low.txt << EOF
15 5
alloc 300 0
calc
write 0 20 103
read 0 20 1
free 0
EOF

# Low priority process (MLQ: 1, CFS: 19)
cat > ossim_sierra/input/proc/comparison/proc_low.txt << EOF
19 5
alloc 300 0
calc
write 0 20 104
read 0 20 1
free 0
EOF

echo "Test files created successfully."