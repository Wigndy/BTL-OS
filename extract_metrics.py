import sys
import re
from collections import defaultdict

def extract_metrics(log_file):
    # Store process events
    processes = defaultdict(lambda: {'load': None, 'dispatches': [], 'finish': None})
    
    # Current time from logs
    current_time = 0
    max_time = 0
    
    print(f"Processing log file: {log_file}")
    
    try:
        with open(log_file, 'r') as f:
            content = f.read()
            
            # Extract all time slots to find the max time
            time_slots = re.findall(r'Time slot\s+(\d+)', content)
            if time_slots:
                max_time = max(map(int, time_slots))
                print(f"Maximum time slot found: {max_time}")
            else:
                print("WARNING: No time slots found in the log")
                
            # Process the file line by line for detailed events
            lines = content.split('\n')
            for line_num, line in enumerate(lines):
                # Track time slots
                time_match = re.match(r'Time slot\s+(\d+)', line)
                if time_match:
                    current_time = int(time_match.group(1))
                    continue
                
                # Process load events (detect both CFS and MLQ formats)
                load_match = re.search(r'Loaded a process at.*?PID: (\d+)', line)
                if load_match:
                    pid = int(load_match.group(1))
                    processes[pid]['load'] = current_time
                    # Try to extract priority/niceness if available
                    nice_match = re.search(r'NICE: (-?\d+)', line)
                    prio_match = re.search(r'PRIO: (\d+)', line)
                    if nice_match:
                        processes[pid]['nice'] = int(nice_match.group(1))
                    elif prio_match:
                        processes[pid]['prio'] = int(prio_match.group(1))
                    continue
                
                # Process dispatch events - more flexible pattern
                dispatch_match = re.search(r'CPU \d+: Dispatched process\s+(\d+)', line)
                if dispatch_match:
                    pid = int(dispatch_match.group(1))
                    if pid in processes:
                        processes[pid]['dispatches'].append(current_time)
                    else:
                        # Create entry for processes we detect dispatch but missed load
                        processes[pid] = {
                            'load': current_time,  # Estimate load time
                            'dispatches': [current_time],
                            'finish': None
                        }
                    continue
                
                # Process finish events - more flexible pattern
                finish_match = re.search(r'(CPU \d+: )?Processed\s+(\d+) has finished', line)
                if finish_match:
                    pid = int(finish_match.group(2))
                    if pid in processes:
                        processes[pid]['finish'] = current_time
                    else:
                        # Create entry for processes we detect finish but missed load/dispatch
                        processes[pid] = {
                            'load': max(0, current_time - 5),  # Estimate load time
                            'dispatches': [max(0, current_time - 1)],  # Estimate first dispatch
                            'finish': current_time
                        }
                    continue
                    
        if not processes:
            # Try an alternate approach with wider regex if no processes found
            print("No processes found with initial parsing. Trying alternate approach...")
            with open(log_file, 'r') as f:
                content = f.read()
                # Wider regex patterns
                loads = re.findall(r'Time slot\s+(\d+).*?Loaded.*?PID: (\d+)', content, re.DOTALL)
                dispatches = re.findall(r'Time slot\s+(\d+).*?Dispatched process\s+(\d+)', content, re.DOTALL)
                finishes = re.findall(r'Time slot\s+(\d+).*?(Processed|process)\s+(\d+) has finished', content, re.DOTALL)
                
                for time, pid in loads:
                    processes[int(pid)]['load'] = int(time)
                
                for time, pid in dispatches:
                    if int(pid) in processes:
                        processes[int(pid)]['dispatches'].append(int(time))
                
                for time, _, pid in finishes:
                    if int(pid) in processes:
                        processes[int(pid)]['finish'] = int(time)
                        
    except Exception as e:
        print(f"Error processing file {log_file}: {str(e)}")
        return {}
    
    # Filter out processes that don't have both load and finish times
    complete_processes = {pid: data for pid, data in processes.items() 
                         if data['load'] is not None and data['finish'] is not None}
    
    if not complete_processes:
        print("WARNING: No complete process records found!")
        
    # Calculate metrics
    total_wait_time = 0
    total_turnaround_time = 0
    total_response_time = 0
    process_count = len(complete_processes)
    
    for pid, data in sorted(complete_processes.items()):
        # First response time (time from load to first dispatch)
        first_dispatch = data['dispatches'][0] if data['dispatches'] else data['load']
        response_time = first_dispatch - data['load']
        total_response_time += response_time
        
        # Turnaround time (time from load to finish)
        turnaround_time = data['finish'] - data['load']
        total_turnaround_time += turnaround_time
        
        # Wait time approximation: turnaround time minus execution time
        # Assumes execution time is approximately (finish - first_dispatch)
        execution_time = data['finish'] - first_dispatch
        wait_time = turnaround_time - execution_time
        total_wait_time += wait_time
        
        print(f"PID {pid}: Load={data['load']}, Finish={data['finish']}, " +
              f"Response={response_time}, Turnaround={turnaround_time}, Wait={wait_time}")
    
    if process_count > 0:
        avg_wait_time = total_wait_time / process_count
        avg_turnaround_time = total_turnaround_time / process_count
        avg_response_time = total_response_time / process_count
        
        print("\nSUMMARY:")
        print(f"Total processes: {process_count}")
        print(f"Average wait time: {avg_wait_time:.2f} time units")
        print(f"Average turnaround time: {avg_turnaround_time:.2f} time units")
        print(f"Average response time: {avg_response_time:.2f} time units")
        print(f"Total time: {max_time} time units")
        
        # Return calculated metrics
        return {
            "process_count": process_count,
            "avg_wait_time": avg_wait_time,
            "avg_turnaround_time": avg_turnaround_time,
            "avg_response_time": avg_response_time,
            "total_time": max_time
        }
    else:
        print("No complete processes found for metrics calculation")
        return {}

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python extract_metrics.py <log_file>")
        sys.exit(1)
    
    metrics = extract_metrics(sys.argv[1])
    
    # Write metrics to output file if requested
    if len(sys.argv) > 2:
        with open(sys.argv[2], 'w') as f:
            for key, value in metrics.items():
                if isinstance(value, float):
                    f.write(f"{key}: {value:.2f}\n")
                else:
                    f.write(f"{key}: {value}\n")