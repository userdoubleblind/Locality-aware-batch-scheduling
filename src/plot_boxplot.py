# Boxplots from mean stretch of all workloads

# Import libraries
import matplotlib.pyplot as plt
import numpy as np
import sys
import pandas as pd

df = pd.read_csv(sys.argv[1])
print("Opening file", sys.argv[1])

font_size = 14

eft = list(df.iloc[:, 0])
score = list(df.iloc[:, 1])
opportunistic = list(df.iloc[:, 2])
eft_score = list(df.iloc[:, 3])

columns = [eft, score, opportunistic, eft_score]
colors=["#E50000","#00bfff","#ff9b15","#91a3b0"]
fig, ax = plt.subplots()

box = plt.boxplot(columns, patch_artist=True, meanline=True, showmeans=True, whis=[12.5, 87.5])

c="#095228"

for boxes in box['boxes']:
	if sys.argv[1] != "outputs/scatter_mean_stretch_all_workloads.csv":
		boxes.set(hatch = '/', color="white")
		
for median in box['medians']:
    median.set_color(c)
    median.set_linewidth(1.9)
for mean in box['means']:
    mean.set_color(c)
    mean.set_linewidth(1.9)

print([item.get_ydata()[1] for item in box['whiskers']])


if sys.argv[1] == "outputs/scatter_mean_stretch_all_workloads.csv":
	plt.xticks([1, 2, 3, 4], ["EFT", "LEA", "LEO", "LEM"], fontsize=font_size)
else:
	plt.xticks([1, 2, 3, 4], ["EFT-BF", "LEA-BF", "LEO-BF", "LEM-BF"], fontsize=font_size)
	
plt.axhline(y = 1, color = 'black', linestyle = "dotted", linewidth=4)

plt.ylim(0, 3.15)
plt.yticks(fontsize=font_size)

plt.rcParams['hatch.linewidth'] = 9

if sys.argv[1] != "outputs/scatter_mean_stretch_all_workloads.csv":
	for patch, color in zip(box['boxes'], colors):
		patch.set_facecolor("white")
		patch.set_edgecolor(color)
else:
	for patch, color in zip(box['boxes'], colors):
		patch.set_facecolor(color)
		
if sys.argv[1] == "outputs/scatter_mean_stretch_all_workloads.csv":
	filename = "plot/box_plot_mean_stretch_all_workloads.pdf"
	plt.ylabel('Stretch\'s improvement from FCFS', fontsize=font_size)
else:
	filename = "plot/box_plot_mean_stretch_all_workloads_bf.pdf"
	plt.ylabel('Stretch\'s improvement from FCFS-BF', fontsize=font_size)
plt.savefig(filename, bbox_inches='tight')

