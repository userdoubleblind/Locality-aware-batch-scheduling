import sys
import csv

input_data = sys.argv[1]
output_data = open("outputs/Percentages_to_fcfs_" + sys.argv[2], "w")
output_data_bf = open("outputs/Percentages_to_fcfs_bf_" + sys.argv[2], "w")

firstline = True
fcfsline = True
fcfsbfline = True
pair = True
eftline = True
mycsv = csv.reader(open(input_data))
output_data.write("Scheduler, Maximum queue time, Total flow, Total transfer time, Stretch, Stretch with a minimum\n")
output_data_bf.write("Scheduler, Maximum queue time, Total flow, Total transfer time, Stretch, Stretch with a minimum\n")
for row in mycsv:
    if firstline:
        firstline = False
        continue
    if fcfsline:
        fcfsline = False
        max_queue_fcfs = float(row[2])
        total_flow_fcfs = float(row[7])
        transfer_time_fcfs = float(row[12])
        stretch_fcfs = float(row[13])
        stretch_with_a_min_fcfs = float(row[14])
        continue
    if fcfsbfline:
        fcfsbfline = False
        max_queue_fcfsbf = float(row[2])
        total_flow_fcfsbf = float(row[7])
        transfer_time_fcfsbf = float(row[12])
        stretch_fcfsbf = float(row[13])
        stretch_with_a_min_fcfsbf = float(row[14])
        continue
    if (max_queue_fcfs == 0):
	    max_queue_fcfs = 1
    if (max_queue_fcfsbf == 0):
	    max_queue_fcfsbf = 1
	   
    if (float(row[2]) == 0):
	    max_queue_to_compare = 1
    else:
	    max_queue_to_compare = float(row[2])
    
    if pair:
	    pair = False
	    output_data.write(row[0] + "," + str(max_queue_fcfs/max_queue_to_compare) + "," + str(total_flow_fcfs/float(row[7])) + "," + str(transfer_time_fcfs/float(row[12])) + "," + str(stretch_fcfs/float(row[13])) + "," + str(stretch_with_a_min_fcfs/float(row[14])) + "\n")
    else:
	    pair = True
	    output_data_bf.write(row[0] + "," + str(max_queue_fcfsbf/max_queue_to_compare) + "," + str(total_flow_fcfsbf/float(row[7])) + "," + str(transfer_time_fcfsbf/float(row[12])) + "," + str(stretch_fcfsbf/float(row[13])) + "," + str(stretch_with_a_min_fcfsbf/float(row[14])) + "\n")
