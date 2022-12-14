# Print an ECDF from mean stretch of various workloads

import numpy as np
import matplotlib.pyplot as plt
from statsmodels.distributions.empirical_distribution import ECDF
import sys
import pandas as pd

df = pd.read_csv(sys.argv[1])
print("Opening file", sys.argv[1])

font_size = 14

eft = list(df.iloc[:, 0])
score = list(df.iloc[:, 1])
opportunistic = list(df.iloc[:, 2])
eft_score = list(df.iloc[:, 3])


min_ecdf = min(eft + score + opportunistic + eft_score)
max_ecdf = 3.15
x = np.linspace(min_ecdf, max_ecdf)

print("min:", min_ecdf, "max:", max_ecdf)
colors=["#E50000","#00bfff","#ff9b15","#91a3b0"]
plt.axvline(x = 1, linestyle = "dotted", color = "black", linewidth=4)

if sys.argv[1] == "outputs/scatter_mean_stretch_all_workloads.csv":
	linestyle="solid"
else:
	linestyle="dashed"

ecdf = ECDF(eft)
y = ecdf(x)
plt.step(x, y, label = "EFT", color = colors[0], linewidth=2, linestyle=linestyle)

ecdf = ECDF(score)
y = ecdf(x)
plt.step(x, y, label = "LEA", color = colors[1], linewidth=2, linestyle=linestyle)

ecdf = ECDF(opportunistic)
y = ecdf(x)
plt.step(x, y, label = "LEO", color = colors[2], linewidth=2, linestyle=linestyle)

ecdf = ECDF(eft_score)
y = ecdf(x)
plt.step(x, y, label = "LEM", color = colors[3], linewidth=2, linestyle=linestyle)

if sys.argv[1] == "outputs/scatter_mean_stretch_all_workloads.csv":
	filename = "plot/ecdf_mean_stretch_all_workloads.pdf"
	plt.xlabel('Stretch\'s improvement from FCFS', fontsize=font_size)
else:
	filename = "plot/ecdf_mean_stretch_all_workloads_bf.pdf"
	plt.xlabel('Stretch\'s improvement from FCFS-BF', fontsize=font_size)

plt.xticks(fontsize=font_size)
plt.yticks(fontsize=font_size)
plt.ylabel('Cumulative probability', fontsize=font_size)
plt.legend(fontsize=font_size, loc="lower right")

plt.savefig(filename)
