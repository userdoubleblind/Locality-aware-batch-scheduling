all: main.c read_input_files.c print_functions.c basic_functions.c scheduler.c linked_list_functions.c backfill_functions.c scheduler_calling.c
	gcc -Wall -o main main.c read_input_files.c print_functions.c basic_functions.c scheduler.c linked_list_functions.c backfill_functions.c scheduler_calling.c -I. -lm

print_distribution_queue_times: main.c read_input_files.c print_functions.c basic_functions.c scheduler.c linked_list_functions.c backfill_functions.c scheduler_calling.c
	gcc -Wall -DPRINT_DISTRIBUTION_QUEUE_TIMES -o main main.c read_input_files.c print_functions.c basic_functions.c scheduler.c linked_list_functions.c backfill_functions.c scheduler_calling.c -I. -lm

print_cluster_usage: main.c read_input_files.c print_functions.c basic_functions.c scheduler.c linked_list_functions.c backfill_functions.c scheduler_calling.c
	gcc -Wall -DPRINT_CLUSTER_USAGE -o main main.c read_input_files.c print_functions.c basic_functions.c scheduler.c linked_list_functions.c backfill_functions.c scheduler_calling.c -I. -lm
	
print_cluster_usage_core_by_core: main.c read_input_files.c print_functions.c basic_functions.c scheduler.c linked_list_functions.c backfill_functions.c scheduler_calling.c
	gcc -Wall -DPRINT_CLUSTER_USAGE -DPRINT_CLUSTER_USAGE_CORE_BY_CORE -o main main.c read_input_files.c print_functions.c basic_functions.c scheduler.c linked_list_functions.c backfill_functions.c scheduler_calling.c -I. -lm
	
clean:
	rm *.o
