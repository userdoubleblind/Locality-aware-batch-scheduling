#!/bin/bash

start=`date +%s`

if [ "$#" -ne 3 ]; then
    echo "Usage is bash Expe1.sh converted_workload cluster save_time"
    exit
fi

WORKLOAD=$1
WORKLOAD_TP=${WORKLOAD:27}
CLUSTER=$2
CLUSTER_TP=${CLUSTER:24}
CLUSTER_TP=${CLUSTER_TP::-4}

make -C src/

SAVE_TIME=$(($3))

OUTPUT_FILE=outputs/Results_${WORKLOAD_TP}_${CLUSTER_TP}.csv
echo "Scheduler,Number of jobs,Maximum queue time,Mean queue time,Total queue time,Maximum flow,Mean flow,Total flow,Transfer time,Makespan,Core time used, Waiting for a load time, Total waiting for a load time and transfer time, Mean Stretch, Mean Stretch With a Minimum, Max Stretch, Max Stretch With a Minimum, Nb Upgraded Jobs, Nb jobs large queue time, Mean flow stretch 128 jobs, Mean flow stretch 256 jobs, Mean flow stretch 1024 jobs, Mean flow stretch with a minimum 128 jobs, Mean flow stretch with a minimum 256 jobs, Mean flow stretch with a minimum 1024 jobs, Number of data reuse" > ${OUTPUT_FILE}

for ((i=1; i<=10; i++))
do
	if [ $((i)) == 1 ]; then SCHEDULER="FCFS";
	elif [ $((i)) == 2 ]; then SCHEDULER="FCFS_BF";
	elif [ $((i)) == 3 ]; then SCHEDULER="EFT";
	elif [ $((i)) == 4 ]; then SCHEDULER="EFT_BF";
	elif [ $((i)) == 5 ]; then SCHEDULER="LEA";
	elif [ $((i)) == 6 ]; then SCHEDULER="LEA_BF";
	elif [ $((i)) == 7 ]; then SCHEDULER="LEO";
	elif [ $((i)) == 8 ]; then SCHEDULER="LEO_BF";
	elif [ $((i)) == 9 ]; then SCHEDULER="LEM";
	elif [ $((i)) == 10 ]; then SCHEDULER="LEM_BF";
	fi
	
	if (($((SAVE_TIME)) == 1))
	then
		./src/main $WORKLOAD $CLUSTER $SCHEDULER $CONTRAINTES_TAILLES $OUTPUT_FILE
	else
		./src/main $WORKLOAD $CLUSTER $SCHEDULER $CONTRAINTES_TAILLES $OUTPUT_FILE save $SAVE_TIME
		./src/main $WORKLOAD $CLUSTER $SCHEDULER $CONTRAINTES_TAILLES $OUTPUT_FILE resume
	fi
done

echo "Final results are:"
cat ${OUTPUT_FILE}

python3 src/plot_dot_and_bar.py Results_${WORKLOAD_TP} Mean_Stretch Total_waiting_for_a_load_time_and_transfer_time 450_128_32_256_4_1024 outputs/Results_${WORKLOAD_TP}_450_128_32_256_4_1024.csv 0
python3 src/compute_percentage_reduction.py outputs/Results_${WORKLOAD_TP}_450_128_32_256_4_1024.csv ${WORKLOAD_TP}

end=`date +%s` 
runtime=$((end-start))
echo "Execution complete! It lasted" $((runtime/60))" minute(s) and "$((runtime%60))" second(s)."
