#!/usr/bin/env python3

import sys
import os
import matplotlib.pyplot as plt
import numpy as np
from compare_metrics import parse_metrics_file

def visualize_comparison(cfs_file, mlq_file, output_dir):
    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)
    
    # Parse metrics
    cfs_metrics = parse_metrics_file(cfs_file)
    mlq_metrics = parse_metrics_file(mlq_file)
    
    # Metrics to visualize
    metrics = {
        'avg_wait': 'Average Wait Time',
        'avg_turnaround': 'Average Turnaround Time', 
        'avg_response': 'Average Response Time',
        'total_time': 'Total Runtime'
    }
    
    # Create bar chart for each metric
    for metric_key, metric_name in metrics.items():
        if metric_key in cfs_metrics and metric_key in mlq_metrics:
            plt.figure(figsize=(10, 6))
            
            # Create bar chart
            schedulers = ['CFS', 'MLQ']
            values = [cfs_metrics[metric_key], mlq_metrics[metric_key]]
            colors = ['#3498db', '#e74c3c']
            
            bars = plt.bar(schedulers, values, color=colors, width=0.6)
            
            # Add value labels on top of bars
            for bar in bars:
                height = bar.get_height()
                plt.text(bar.get_x() + bar.get_width()/2., height + 0.1,
                        f'{height:.2f}', ha='center', fontweight='bold')
            
            # Add percentage difference
            diff_pct = abs(values[0] - values[1]) / max(values) * 100
            better = "CFS" if values[0] < values[1] else "MLQ" if values[1] < values[0] else "Tie"
            plt.title(f'{metric_name} Comparison\n{diff_pct:.1f}% difference in favor of {better}', 
                    fontsize=14, fontweight='bold')
            
            plt.ylabel(metric_name, fontsize=12)
            plt.ylim(0, max(values) * 1.2)  # Add 20% space for labels
            plt.grid(axis='y', linestyle='--', alpha=0.7)
            
            # Save the figure
            plt.tight_layout()
            plt.savefig(os.path.join(output_dir, f"{metric_key}_comparison.png"), dpi=300)
            plt.close()
    
    # Create combined chart
    plt.figure(figsize=(12, 8))
    
    # Define x positions for grouped bars
    x = np.arange(3)  # 3 main metrics (excluding total_time)
    width = 0.35
    
    metric_keys = ['avg_wait', 'avg_turnaround', 'avg_response']
    metric_names = ['Wait Time', 'Turnaround Time', 'Response Time']
    
    # Create grouped bars
    cfs_values = [cfs_metrics.get(key, 0) for key in metric_keys]
    mlq_values = [mlq_metrics.get(key, 0) for key in metric_keys]
    
    plt.bar(x - width/2, cfs_values, width, label='CFS', color='#3498db')
    plt.bar(x + width/2, mlq_values, width, label='MLQ', color='#e74c3c')
    
    # Add labels and title
    plt.xlabel('Performance Metrics', fontsize=12)
    plt.ylabel('Time Units', fontsize=12)
    plt.title('CFS vs MLQ Scheduler Performance Comparison', fontsize=14, fontweight='bold')
    plt.xticks(x, metric_names)
    plt.legend()
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    
    # Add value labels
    for i, v in enumerate(cfs_values):
        plt.text(i - width/2, v + 0.1, f"{v:.2f}", ha='center', fontweight='bold')
    for i, v in enumerate(mlq_values):
        plt.text(i + width/2, v + 0.1, f"{v:.2f}", ha='center', fontweight='bold')
    
    plt.tight_layout()
    plt.savefig(os.path.join(output_dir, "combined_metrics.png"), dpi=300)
    
    print(f"Visualization charts saved to {output_dir}")

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python visualize_comparison.py <cfs_metrics_file> <mlq_metrics_file> <output_directory>")
        sys.exit(1)
    
    visualize_comparison(sys.argv[1], sys.argv[2], sys.argv[3])