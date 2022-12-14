#!/bin/bash

python3 src/compute_percentages_all_workloads.py 4 0 mean outputs/Percentages_to_fcfs_2022-08-16-\>2022-08-16_V10000_anonymous outputs/Percentages_to_fcfs_2022-07-16-\>2022-07-16_V10000_anonymous outputs/Percentages_to_fcfs_2022-03-26-\>2022-03-26_V10000_anonymous outputs/Percentages_to_fcfs_2022-09-09-\>2022-09-09_V10000_anonymous

python3 src/compute_percentages_all_workloads.py 4 1 mean outputs/Percentages_to_fcfs_bf_2022-08-16-\>2022-08-16_V10000_anonymous outputs/Percentages_to_fcfs_bf_2022-07-16-\>2022-07-16_V10000_anonymous outputs/Percentages_to_fcfs_bf_2022-03-26-\>2022-03-26_V10000_anonymous outputs/Percentages_to_fcfs_bf_2022-09-09-\>2022-09-09_V10000_anonymous

python3 src/plot_boxplot.py outputs/scatter_mean_stretch_all_workloads.csv

python3 src/plot_boxplot.py outputs/scatter_mean_stretch_all_workloads_bf.csv

python3 src/ecdf.py outputs/scatter_mean_stretch_all_workloads.csv
