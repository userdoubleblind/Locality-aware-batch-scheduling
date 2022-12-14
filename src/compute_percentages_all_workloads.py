import sys
import argparse
import csv
import statistics
import pandas as pd
from matplotlib import pyplot as plt
import numpy as np

if (int(sys.argv[2]) == 1):
	if (sys.argv[3] == "mediane"):
		output_data_bf = open("outputs/Percentages_to_fcfs_bf_all_workloads_mediane", "w")
	else:
		output_data_bf = open("outputs/Percentages_to_fcfs_bf_all_workloads_mean", "w")
else:
	if (sys.argv[3] == "mediane"):
		output_data_bf = open("outputs/Percentages_to_fcfs_all_workloads_mediane", "w")
	else:
		output_data_bf = open("outputs/Percentages_to_fcfs_all_workloads_mean", "w")
firstline = True
fcfsline = True
fcfsbfline = True
pair = True
eftline = True
output_data_bf.write("Scheduler, Maximum queue time, Total flow, Total transfer time, Stretch, Stretch with a minimum\n")

sum_max_queue_time = [[], [], [], []]
sum_total_flow = [[], [], [], []]
sum_transfer_time = [[], [], [], []]
sum_stretch = [[], [], [], []]
sum_stretch_with_min = [[], [], [], []]

nargs = float(sys.argv[1])
nargs2 = int(sys.argv[1])

# Writing data for scatter plots
if (sys.argv[3] == "mean"):
	if (int(sys.argv[2]) == 1):
		scatter_mean_stretch_all_workloads = open("outputs/scatter_mean_stretch_all_workloads_bf.csv", "w")
		scatter_mean_stretch_all_workloads.write("EFT CONSERVATIVE BF,SCORE CONSERVATIVE BF,OPPORTUNISTIC-SCORE MIX CONSERVATIVE BF,EFT-SCORE MIX CONSERVATIVE BF\n")
	else:
		scatter_mean_stretch_all_workloads = open("outputs/scatter_mean_stretch_all_workloads.csv", "w")
		scatter_mean_stretch_all_workloads.write("EFT,SCORE,OPPORTUNISTIC-SCORE MIX,EFT-SCORE MIX\n")

row_index = 0
print("Number of files:", nargs)
for i in range(0, nargs2):
	mycsv = csv.reader(open(sys.argv[i + 4]))
	print("Read " + sys.argv[i + 4] + "...")
	firstline = True
	j = 0
	row_index = 0
	for row in mycsv:
		if firstline:
			firstline = False
			continue
		sum_max_queue_time[j].append(float(row[1]))
		sum_total_flow[j].append(float(row[2]))
		sum_transfer_time[j].append(float(row[3]))
		sum_stretch[j].append(float(row[4]))
		sum_stretch_with_min[j].append(float(row[5]))
		
		# For scatter plot later on
		if (sys.argv[3] == "mean"):
			if (row_index < 3):
				scatter_mean_stretch_all_workloads.write(str(float(row[4])) + ",")
			else:
				scatter_mean_stretch_all_workloads.write(str(float(row[4])))
		row_index = row_index + 1
		
		j += 1
	
	if (sys.argv[3] == "mean"):
		scatter_mean_stretch_all_workloads.write("\n")

if (sys.argv[3] == "mean"):
	scatter_mean_stretch_all_workloads.close()

print(sum_stretch)

if (sys.argv[3] == "mediane"):
	if (int(sys.argv[2]) == 1): # bf
		output_data_bf.write("EFT CONSERVATIVE BF" + "," + str(statistics.median(sum_max_queue_time[0])) + "," + str(statistics.median(sum_total_flow[0])) + "," + str(statistics.median(sum_transfer_time[0])) + "," + str(statistics.median(sum_stretch[0])) + "," + str(statistics.median(sum_stretch_with_min[0])) + "\n")
		output_data_bf.write("SCORE CONSERVATIVE BF"  + "," + str(statistics.median(sum_max_queue_time[1])) + "," + str(statistics.median(sum_total_flow[1])) + "," + str(statistics.median(sum_transfer_time[1])) + "," + str(statistics.median(sum_stretch[1])) + "," + str(statistics.median(sum_stretch_with_min[1]))  + "\n")
		output_data_bf.write("OPPORTUNISTIC-SCORE MIX CONSERVATIVE BF"  + "," + str(statistics.median(sum_max_queue_time[2])) + "," + str(statistics.median(sum_total_flow[2])) + "," + str(statistics.median(sum_transfer_time[2])) + "," + str(statistics.median(sum_stretch[2])) + "," + str(statistics.median(sum_stretch_with_min[2]))  +"\n")
		output_data_bf.write("EFT-SCORE MIX CONSERVATIVE BF"  + "," + str(statistics.median(sum_max_queue_time[3])) + "," + str(statistics.median(sum_total_flow[3])) + "," + str(statistics.median(sum_transfer_time[3])) + "," + str(statistics.median(sum_stretch[3])) + "," + str(statistics.median(sum_stretch_with_min[3]))  + "\n")
	else:
		output_data_bf.write("EFT" + "," + str(statistics.median(sum_max_queue_time[0])) + "," + str(statistics.median(sum_total_flow[0])) + "," + str(statistics.median(sum_transfer_time[0])) + "," + str(statistics.median(sum_stretch[0])) + "," + str(statistics.median(sum_stretch_with_min[0])) + "\n")
		output_data_bf.write("SCORE"  + "," + str(statistics.median(sum_max_queue_time[1])) + "," + str(statistics.median(sum_total_flow[1])) + "," + str(statistics.median(sum_transfer_time[1])) + "," + str(statistics.median(sum_stretch[1])) + "," + str(statistics.median(sum_stretch_with_min[1])) + "\n")
		output_data_bf.write("OPPORTUNISTIC-SCORE MIX"  + "," + str(statistics.median(sum_max_queue_time[2])) + "," + str(statistics.median(sum_total_flow[2])) + "," + str(statistics.median(sum_transfer_time[2])) + "," + str(statistics.median(sum_stretch[2])) + "," + str(statistics.median(sum_stretch_with_min[2])) + "\n")
		output_data_bf.write("EFT-SCORE MIX"  + "," + str(statistics.median(sum_max_queue_time[3])) + "," + str(statistics.median(sum_total_flow[3])) + "," + str(statistics.median(sum_transfer_time[3])) + "," + str(statistics.median(sum_stretch[3])) + "," + str(statistics.median(sum_stretch_with_min[3]))  + "\n")
elif (sys.argv[3] == "mean"):
	if (int(sys.argv[2]) == 1): # bf
		output_data_bf.write("EFT CONSERVATIVE BF" + "," + str(statistics.mean(sum_max_queue_time[0])) + "," + str(statistics.mean(sum_total_flow[0])) + "," + str(statistics.mean(sum_transfer_time[0])) + "," + str(statistics.mean(sum_stretch[0])) + "," + str(statistics.mean(sum_stretch_with_min[0])) + "\n")
		output_data_bf.write("SCORE CONSERVATIVE BF"  + "," + str(statistics.mean(sum_max_queue_time[1])) + "," + str(statistics.mean(sum_total_flow[1])) + "," + str(statistics.mean(sum_transfer_time[1])) + "," + str(statistics.mean(sum_stretch[1])) + "," + str(statistics.mean(sum_stretch_with_min[1]))  + "\n")
		output_data_bf.write("OPPORTUNISTIC-SCORE MIX CONSERVATIVE BF"  + "," + str(statistics.mean(sum_max_queue_time[2])) + "," + str(statistics.mean(sum_total_flow[2])) + "," + str(statistics.mean(sum_transfer_time[2])) + "," + str(statistics.mean(sum_stretch[2])) + "," + str(statistics.mean(sum_stretch_with_min[2]))  +"\n")
		output_data_bf.write("EFT-SCORE MIX CONSERVATIVE BF"  + "," + str(statistics.mean(sum_max_queue_time[3])) + "," + str(statistics.mean(sum_total_flow[3])) + "," + str(statistics.mean(sum_transfer_time[3])) + "," + str(statistics.mean(sum_stretch[3])) + "," + str(statistics.mean(sum_stretch_with_min[3]))  + "\n")
	else:
		output_data_bf.write("EFT" + "," + str(statistics.mean(sum_max_queue_time[0])) + "," + str(statistics.mean(sum_total_flow[0])) + "," + str(statistics.mean(sum_transfer_time[0])) + "," + str(statistics.mean(sum_stretch[0])) + "," + str(statistics.mean(sum_stretch_with_min[0])) + "\n")
		output_data_bf.write("SCORE"  + "," + str(statistics.mean(sum_max_queue_time[1])) + "," + str(statistics.mean(sum_total_flow[1])) + "," + str(statistics.mean(sum_transfer_time[1])) + "," + str(statistics.mean(sum_stretch[1])) + "," + str(statistics.mean(sum_stretch_with_min[1])) + "\n")
		output_data_bf.write("OPPORTUNISTIC-SCORE MIX"  + "," + str(statistics.mean(sum_max_queue_time[2])) + "," + str(statistics.mean(sum_total_flow[2])) + "," + str(statistics.mean(sum_transfer_time[2])) + "," + str(statistics.mean(sum_stretch[2])) + "," + str(statistics.mean(sum_stretch_with_min[2])) + "\n")
		output_data_bf.write("EFT-SCORE MIX"  + "," + str(statistics.mean(sum_max_queue_time[3])) + "," + str(statistics.mean(sum_total_flow[3])) + "," + str(statistics.mean(sum_transfer_time[3])) + "," + str(statistics.mean(sum_stretch[3])) + "," + str(statistics.mean(sum_stretch_with_min[3]))  + "\n")
