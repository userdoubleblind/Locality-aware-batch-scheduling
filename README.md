# Locality-aware-batch-scheduling-of-I-O-intensive-workloads

You can find in this depot the source code of the batch simulator and the schedulers used in the paper Locality-aware batch scheduling of I/O intensive workloads, submitted at CCGrid 2023.
Results presented in the paper can be reproduced with this depot.
All the workloads used for our evaluations are in the folder inputs/workloads/converted/
The model of the cluster used is in inputs/clusters/
The source code is in src/
The generated figures will be in plot/

## Source code

The source code presented here is a simplified version of our simulator and schedulers.
It contains only the functions needed to run the 5 schedulers presented in the paper.
All other schedulers and settings tested and not presented in the paper, as well as tools 
to visualize our strategies are not present.

The code is constructed as follows:
- main.c calls functions to initialize the simulator and contains the loop that iterate while all jobs have not been scheduled
- scheduler.c contains the 5 schedulers presented in the paper
- basic_functions.c gather functions that can be called by any scheduler or by the main
- read_input_files.c initialize nodes and jobs list from input files
- print_functions.c contains functions used to print in an output file the results of the execution
- linked_list_functions gather basic functions for managing linked list
- backfill_functions.c regroup functions used when using conservative backfilling
- scheduller_calling.c contain the function that start a scheduler depending on the scheduler chosen by the user

The python files are used to plot the figures.

## Reproducible experiments

You can re-create the figures showing the mean stretch and the total waiting time from section V-B, V-C, V-D, V-E by doing the following steps:

```bash
~/$ bash Expe1.sh inputs/workloads/converted/2022-07-16-\>2022-07-16_V10000_anonymous inputs/clusters/cluster_450_128_32_256_4_1024.txt 1
```
```bash
~/$ bash Expe1.sh inputs/workloads/converted/2022-09-09-\>2022-09-09_V10000_anonymous inputs/clusters/cluster_450_128_32_256_4_1024.txt 1
```
```bash
~/$ bash Expe1.sh inputs/workloads/converted/2022-03-26-\>2022-03-26_V10000_anonymous inputs/clusters/cluster_450_128_32_256_4_1024.txt 1
```
```bash
~/$ bash Expe1.sh inputs/workloads/converted/2022-08-16-\>2022-08-16_V10000_anonymous inputs/clusters/cluster_450_128_32_256_4_1024.txt 1
```

To reproduce the boxplots and ecdf from section V-F you must use the previous commands with each workload contained in inputs/workloads/converted/:
```bash
~/$ bash Expe1.sh inputs/workloads/converted/date_of_the_workload_V10000_anonymous inputs/clusters/cluster_450_128_32_256_4_1024.txt 1
```
And then do:
```bash
~/$ bash Plot_all_workloads.sh
```
