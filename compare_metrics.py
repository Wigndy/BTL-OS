
import sys
import re
import matplotlib.pyplot as plt
import os

def parse_metrics_file(file_path):
    metrics = {}
    try:
        with open(file_path, 'r') as f:
            content = f.read()
            
            # Extract numbers with better regex patterns
            avg_wait = re.search(r'Average wait time: ([\d\.]+)', content)
            avg_turnaround = re.search(r'Average turnaround time: ([\d\.]+)', content)
            avg_response = re.search(r'Average response time: ([\d\.]+)', content)
            total_time = re.search(r'Total time: (\d+)', content)
            process_count = re.search(r'Total processes: (\d+)', content)
            
            if avg_wait:
                metrics['avg_wait'] = float(avg_wait.group(1))
            if avg_turnaround:
                metrics['avg_turnaround'] = float(avg_turnaround.group(1))
            if avg_response:
                metrics['avg_response'] = float(avg_response.group(1))
            if total_time:
                metrics['total_time'] = int(total_time.group(1))
            if process_count:
                metrics['process_count'] = int(process_count.group(1))
    except Exception as e:
        print(f"Error parsing metrics file {file_path}: {str(e)}")
            
    return metrics

def compare_metrics(cfs_file, mlq_file):
    cfs_metrics = parse_metrics_file(cfs_file)
    mlq_metrics = parse_metrics_file(mlq_file)
    
    # Check if we got any metrics
    if not cfs_metrics and not mlq_metrics:
        print("ERROR: Could not extract metrics from either file!")
        return
    elif not cfs_metrics:
        print("ERROR: Could not extract CFS metrics!")
        return
    elif not mlq_metrics:
        print("ERROR: Could not extract MLQ metrics!")
        return
    
    print("===== CFS vs MLQ SCHEDULER COMPARISON =====\n")
    
    print("| Metric               | CFS        | MLQ        | Difference    | Better    |")
    print("|----------------------|------------|------------|--------------|-----------|")
    
    # Compare wait time
    if 'avg_wait' in cfs_metrics and 'avg_wait' in mlq_metrics:
        diff = cfs_metrics['avg_wait'] - mlq_metrics['avg_wait']
        better = "CFS" if diff < 0 else "MLQ" if diff > 0 else "Tie"
        print(f"| Average Wait Time    | {cfs_metrics['avg_wait']:.2f}      | {mlq_metrics['avg_wait']:.2f}      | {abs(diff):.2f} ({(abs(diff)/max(cfs_metrics['avg_wait'], mlq_metrics['avg_wait'])*100):.1f}%) | {better}     |")
    
    # Compare turnaround time
    if 'avg_turnaround' in cfs_metrics and 'avg_turnaround' in mlq_metrics:
        diff = cfs_metrics['avg_turnaround'] - mlq_metrics['avg_turnaround']
        better = "CFS" if diff < 0 else "MLQ" if diff > 0 else "Tie"
        print(f"| Average Turnaround   | {cfs_metrics['avg_turnaround']:.2f}      | {mlq_metrics['avg_turnaround']:.2f}      | {abs(diff):.2f} ({(abs(diff)/max(cfs_metrics['avg_turnaround'], mlq_metrics['avg_turnaround'])*100):.1f}%) | {better}     |")
    
    # Compare response time
    if 'avg_response' in cfs_metrics and 'avg_response' in mlq_metrics:
        diff = cfs_metrics['avg_response'] - mlq_metrics['avg_response']
        better = "CFS" if diff < 0 else "MLQ" if diff > 0 else "Tie"
        print(f"| Average Response     | {cfs_metrics['avg_response']:.2f}      | {mlq_metrics['avg_response']:.2f}      | {abs(diff):.2f} ({(abs(diff)/max(cfs_metrics['avg_response'], mlq_metrics['avg_response'])*100):.1f}%) | {better}     |")
    
    # Compare total time
    if 'total_time' in cfs_metrics and 'total_time' in mlq_metrics:
        diff = cfs_metrics['total_time'] - mlq_metrics['total_time']
        better = "CFS" if diff < 0 else "MLQ" if diff > 0 else "Tie"
        print(f"| Total Runtime        | {cfs_metrics['total_time']}        | {mlq_metrics['total_time']}        | {abs(diff)} ({(abs(diff)/max(cfs_metrics['total_time'], mlq_metrics['total_time'])*100):.1f}%) | {better}     |")
    
    print("\n===== ANALYSIS =====\n")
    
    # Generate visualizations
    charts_dir = "scheduler_comparison/comparison_charts"
    if not os.path.exists(charts_dir):
        os.makedirs(charts_dir)
    
    # Create bar charts for metrics comparison
    metrics = []
    cfs_values = []
    mlq_values = []
    
    if 'avg_wait' in cfs_metrics and 'avg_wait' in mlq_metrics:
        metrics.append('Average Wait Time')
        cfs_values.append(cfs_metrics['avg_wait'])
        mlq_values.append(mlq_metrics['avg_wait'])
    
    if 'avg_turnaround' in cfs_metrics and 'avg_turnaround' in mlq_metrics:
        metrics.append('Average Turnaround Time')
        cfs_values.append(cfs_metrics['avg_turnaround'])
        mlq_values.append(mlq_metrics['avg_turnaround'])
    
    if 'avg_response' in cfs_metrics and 'avg_response' in mlq_metrics:
        metrics.append('Average Response Time')
        cfs_values.append(cfs_metrics['avg_response'])
        mlq_values.append(mlq_metrics['avg_response'])
    
    # Create bar chart
    if metrics:
        fig, ax = plt.subplots(figsize=(10, 6))
        x = range(len(metrics))
        width = 0.35
        
        ax.bar([i - width/2 for i in x], cfs_values, width, label='CFS', color='skyblue')
        ax.bar([i + width/2 for i in x], mlq_values, width, label='MLQ', color='lightcoral')
        
        ax.set_ylabel('Time Units')
        ax.set_title('CFS vs MLQ Scheduler Performance Comparison')
        ax.set_xticks(x)
        ax.set_xticklabels(metrics)
        ax.legend()
        
        plt.savefig(f"{charts_dir}/metrics_comparison.png", dpi=300, bbox_inches='tight')
    
    # Provide interpretation
    better_wait = "CFS" if cfs_metrics.get('avg_wait', float('inf')) < mlq_metrics.get('avg_wait', float('inf')) else "MLQ"
    better_turnaround = "CFS" if cfs_metrics.get('avg_turnaround', float('inf')) < mlq_metrics.get('avg_turnaround', float('inf')) else "MLQ"
    
    print(f"- {better_wait} provides better average waiting time")
    print(f"- {better_turnaround} provides better overall turnaround time")
    
    print("\nConclusions:")
    if better_wait == better_turnaround:
        print(f"- {better_wait} scheduler shows superior performance for this workload")
    else:
        print("- Results are mixed; scheduler choice depends on optimization goals:")
        print(f"  * Choose CFS for better {'waiting' if better_wait == 'CFS' else 'turnaround'} time")
        print(f"  * Choose MLQ for better {'waiting' if better_wait == 'MLQ' else 'turnaround'} time")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python compare_metrics.py <cfs_metrics_file> <mlq_metrics_file>")
        sys.exit(1)
    
    compare_metrics(sys.argv[1], sys.argv[2])