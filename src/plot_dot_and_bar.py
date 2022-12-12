# importing package
import matplotlib.pyplot as plt
import pandas as pd
import sys

data = pd.read_csv(sys.argv[5])
workload = sys.argv[1]
comparaison1 = sys.argv[2]
comparaison2 = sys.argv[3]
cluster = sys.argv[4]

font_size = 12

bf_mode = int(sys.argv[6])

title = workload + "_" + comparaison1 + "_" + comparaison2 + "_" + cluster

if (comparaison1 == "Maximum_queue_time"):
	Y_index = 2
	plot_title = "Maximum Queue Time"
elif (comparaison1 == "Mean_queue_time"):
	Y_index = 3
	plot_title = "Mean Queue Time"
elif (comparaison1 == "Total_queue_time"):
	Y_index = 4
	plot_title = "Total Queue Time"
elif (comparaison1 == "Maximum_flow"):
	Y_index = 5
	plot_title = "Maximum Flow"
elif (comparaison1 == "Mean_flow"):
	Y_index = 6
	plot_title = "Mean Flow"
elif (comparaison1 == "Total_flow"):
	Y_index = 7
	plot_title = "Total Flow"
elif (comparaison1 == "Transfer_time"):
	Y_index = 8
	plot_title = "Transfer Time"
elif (comparaison1 == "Makespan"):
	Y_index = 9
	plot_title = "Makespan"
elif (comparaison1 == "Core_time_used"):
	Y_index = 10
	plot_title = "Core time used"
elif (comparaison1 == "Waiting_for_a_load_time"):
	Y_index = 11
	plot_title = "Waiting for a load time"
elif (comparaison1 == "Total_waiting_for_a_load_time_and_transfer_time"):
	Y_index = 12
	plot_title = "Total waiting for a load time and transfer time"
elif (comparaison1 == "Mean_Stretch"):
	Y_index = 13
	plot_title = "Mean Stretch"
elif (comparaison1 == "Mean_Stretch_With_a_Minimum"):
	Y_index = 14
	plot_title = "Mean Bounded Stretch"
elif (comparaison1 == "Max_Stretch"):
	Y_index = 15
	plot_title = "Max Stretch"
elif (comparaison1 == "Max_Stretch_With_a_Minimum"):
	Y_index = 16
	plot_title = "Max Bounded Stretch"
elif (comparaison1 == "Nb_Upgraded_Jobs"):
	Y_index = 17
	plot_title = "Nb Upgraded Jobs"
elif (comparaison1 == "Mean_Stretch_128"):
	Y_index = 19 # Car il y a large queue time job que je plot pas mais qui est dans la data
	plot_title = "Mean Stretch 128 jobs"
elif (comparaison1 == "Mean_Stretch_256"):
	Y_index = 20
	plot_title = "Mean Stretch 256 jobs"
elif (comparaison1 == "Mean_Stretch_1024"):
	Y_index = 21
	plot_title = "Mean Stretch 1024 jobs"
elif (comparaison1 == "Mean_Stretch_With_a_Minimum_128"):
	Y_index = 22
	plot_title = "Mean Bounded Stretch 128 jobs"
elif (comparaison1 == "Mean_Stretch_With_a_Minimum_256"):
	Y_index = 23
	plot_title = "Mean Bounded Stretch 256 jobs"
elif (comparaison1 == "Mean_Stretch_With_a_Minimum_1024"):
	Y_index = 24
	plot_title = "Mean Bounded Stretch 1024 jobs"
elif (comparaison1 == "Number_of_data_reuse"):
	Y_index = 25
	plot_title = "Number of data re-use"
else:
	print("Wrong comparison")
	exit(1)

if (comparaison2 == "Maximum_queue_time"):
	Y_index2 = 2
	plot_title = "Maximum Queue Time"
elif (comparaison2 == "Mean_queue_time"):
	Y_index2 = 3
	plot_title = "Mean Queue Time"
elif (comparaison2 == "Total_queue_time"):
	Y_index2 = 4
	plot_title = "Total Queue Time"
elif (comparaison2 == "Maximum_flow"):
	Y_index2 = 5
	plot_title = "Maximum Flow"
elif (comparaison2 == "Mean_flow"):
	Y_index2 = 6
	plot_title = "Mean Flow"
elif (comparaison2 == "Total_flow"):
	Y_index2 = 7
	plot_title = "Total Flow"
elif (comparaison2 == "Transfer_time"):
	Y_index2 = 8
	plot_title = "Transfer Time"
elif (comparaison2 == "Makespan"):
	Y_index2 = 9
	plot_title = "Makespan"
elif (comparaison2 == "Core_time_used"):
	Y_index2 = 10
	plot_title = "Core time used"
elif (comparaison2 == "Waiting_for_a_load_time"):
	Y_index2 = 11
	plot_title = "Waiting for a load time"
elif (comparaison2 == "Total_waiting_for_a_load_time_and_transfer_time"):
	Y_index2 = 12
	plot_title = "Total waiting for a load time and transfer time"
elif (comparaison2 == "Mean_Stretch"):
	Y_index2 = 13
	plot_title = "Mean Stretch"
elif (comparaison2 == "Mean_Stretch_With_a_Minimum"):
	Y_index2 = 14
	plot_title = "Mean Bounded Stretch"
elif (comparaison2 == "Max_Stretch"):
	Y_index2 = 15
	plot_title = "Max Stretch"
elif (comparaison2 == "Max_Stretch_With_a_Minimum"):
	Y_index2 = 16
	plot_title = "Max Bounded Stretch"
elif (comparaison2 == "Nb_Upgraded_Jobs"):
	Y_index2 = 17
	plot_title = "Nb Upgraded Jobs"
elif (comparaison2 == "Mean_Stretch_128"):
	Y_index2 = 19 # Car il y a large queue time job que je plot pas mais qui est dans la data
	plot_title = "Mean Stretch 128 jobs"
elif (comparaison2 == "Mean_Stretch_256"):
	Y_index2 = 20
	plot_title = "Mean Stretch 256 jobs"
elif (comparaison2 == "Mean_Stretch_1024"):
	Y_index2 = 21
	plot_title = "Mean Stretch 1024 jobs"
elif (comparaison2 == "Mean_Stretch_With_a_Minimum_128"):
	Y_index2 = 22
	plot_title = "Mean Bounded Stretch 128 jobs"
elif (comparaison2 == "Mean_Stretch_With_a_Minimum_256"):
	Y_index2 = 23
	plot_title = "Mean Bounded Stretch 256 jobs"
elif (comparaison2 == "Mean_Stretch_With_a_Minimum_1024"):
	Y_index2 = 24
	plot_title = "Mean Bounded Stretch 1024 jobs"
elif (comparaison2 == "Number_of_data_reuse"):
	Y_index2 = 25
	plot_title = "Number of data re-use"
else:
	print("Wrong comparison")
	exit(1)
	
df = pd.DataFrame(data)

X = list(df.iloc[:, 0])
Y = list(df.iloc[:, Y_index])
Y2 = list(df.iloc[:, Y_index2])

# ~ print("Y2:", Y2)
if (bf_mode == 0):
	FCFS_time_to_load = Y2[0]
elif (bf_mode == 3):
	FCFS_time_to_load = Y2[1]
elif (bf_mode == 4):
	FCFS_time_to_load = Y2[0]
	FCFS_time_to_load_bf = Y2[1]

fig = plt.figure()
fig.set_figheight(3.6)
# ~ fig.set_figwidth(10)
	
# ~ fig, ax = plt.subplots(1, 2)
ax0 = fig.add_subplot(121)
ax1 = fig.add_subplot(122)

fig.subplots_adjust(wspace=0.3)

# Pour renommer les algos
X[0] = "FCFS"
X[1] = "FCFS-BF"
X[2] = "EFT"
X[3] = "EFT-BF"
X[4] = "LEA"
X[5] = "LEA-BF"
X[6] = "LEO"
X[7] = "LEO-BF"
X[8] = "LEM"
X[9] = "LEM-BF"
	
plt.rcParams['hatch.linewidth'] = 2

# ~ markers=["^", "v", "s", "o", "D"]
markers=["x", "x", "x", "x", "x"]
colors=["#4c0000","#E50000","#00bfff","#ff9b15","#91a3b0"]

# Pour diviser en 2 BF/NON BF
if (bf_mode == 0): # Non BF
	print("bf_mode:",bf_mode)
	Y_non_bf = [Y[2*i] for i in range(len(Y)//2)]
	Y_non_bf_2 = [Y2[2*i] for i in range(len(Y2)//2)]
	X_non_bf = [X[2*i] for i in range(len(X)//2)]
	hatches = ["x","x","x","x","x"]
	for i in range(len(X_non_bf)):
		ax0.scatter(X_non_bf[i], Y_non_bf[i], color=colors[i], s=200, marker=markers[i], linewidths=3)
		ax1.bar(X_non_bf[i], Y_non_bf_2[i]/FCFS_time_to_load, color=colors[i])		
elif (bf_mode == 3): # BF
	Y_bf = [Y[2*i+1] for i in range(len(Y)//2)]
	Y_bf_2 = [Y2[2*i+1] for i in range(len(Y2)//2)]
	X_bf = [X[2*i+1] for i in range(len(X)//2)]
	for i in range(len(X_bf)):
		ax0.scatter(X_bf[i], Y_bf[i], color=colors[i], s=200, marker=markers[i])
		ax1.bar(X_bf[i], Y_bf_2[i]/FCFS_time_to_load, color=colors[i])
elif (bf_mode == 4): # BF and NON BF on same plot
	print(bf_mode)
	hatches = ['','/','','/','','/','','/','','/']
	colors=["#4c0000","#4c0000","#E50000","#E50000","#00bfff","#00bfff","#ff9b15","#ff9b15","#91a3b0","#91a3b0"]
	for i in range(len(X)):
		ax0.scatter(X[i], Y[i], color=colors[i], s=200, marker=markers[i])
else:
	print("bf_mode not dealt with", bf_mode)
	exit

ax0.set_ylim(0.9,)
ax1.set_ylim(0, 1.03)

ax0.axhline(y = 1, color = 'black', linestyle = "dotted", linewidth=4)
ax0.set_ylabel("Stretch", fontsize=font_size)
ax1.set_ylabel("Ratio", fontsize=font_size)


ax0.annotate("(a) Mean stretch", xy=(0.13, -0.13), xycoords="axes fraction", fontsize=font_size)
ax1.annotate("(b) Load time ratio from FCFS", xy=(-0.14, -0.13), xycoords="axes fraction", fontsize=font_size) 
          
if (bf_mode == 3):
	filename = "plot/BF_" + title + ".pdf"
elif (bf_mode == 4):
	filename = "plot/BF_AND_NON_BF_" + title + ".pdf"
else:
	filename = "plot/" + title + ".pdf"


plt.savefig(filename, bbox_inches='tight')


