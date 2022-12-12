/*
 * Copyright (C) - Anonymous for double blind submission.
 */
 
/**
 * Functions used to initialize our simulator.
 * Read the cluster model and the workload as input files.
 * Fill structures with these information.
 **/

#include <main.h>

/** 
 * The cluster is an input file.
 * It is read here and used to initialize a set of nodes as a linked list.
 **/
void read_cluster(char* input_node_file)
{
	node_list = (struct Node_List**) malloc(3*sizeof(struct Node_List));
	for (int i = 0; i < 3; i++)
	{
		node_list[i] = (struct Node_List*) malloc(sizeof(struct Node_List));
		node_list[i]->head = NULL;
		node_list[i]->tail = NULL;
	}
	total_number_nodes = 0;
	
	FILE *f = fopen(input_node_file, "r");

	if (!f)
	{
		perror("fopen");
        exit(EXIT_FAILURE);
	}
    
	number_node_size[0] = 0;
	number_node_size[1] = 0;
	number_node_size[2] = 0;
    
    char s[100];
    char id[100];
    char memory[100];
    char bandwidth[100];
    char core[100];
    int index_node = 0;
    while (fscanf(f, "%s %s %s %s %s %s %s %s %s %s", s, s, id, s, memory, s, bandwidth, s, core, s) == 10)
	{
		struct Node *new = (struct Node*) malloc(sizeof(struct Node));
		new->unique_id = atoi(id);
		new->memory = atoi(memory);
		new->bandwidth = atof(bandwidth);
		new->n_available_cores = 20;
		new->index_node_list = index_node;
		new->data = malloc(sizeof(*new->data));
		new->data->head = NULL;
		new->data->tail = NULL;
		
		new->cores = (struct Core**) malloc(20*sizeof(struct Core));
		for (int i = 0; i < 20; i++)
		{
			new->cores[i] = (struct Core*) malloc(sizeof(struct Core));
			new->cores[i]->unique_id = i;
			new->cores[i]->available_time = 0;
			new->cores[i]->running_job = false;
			new->cores[i]->running_job_end = -1;
			
			#ifdef PRINT_CLUSTER_USAGE
			new->end_of_file_load = 0; /* Used to then get the total number of cores running a load */
			#endif
		}
		
		/* For conservative bf */
		new->number_cores_in_a_hole = 0;
		new->cores_in_a_hole = malloc(sizeof(*new->cores_in_a_hole));
		new->cores_in_a_hole->head = NULL;
		new->cores_in_a_hole->tail = NULL;
				
		#ifdef PRINT_CLUSTER_USAGE
		new->nb_jobs_workload_1 = 0;
		new->end_of_file_load = 0;
		#endif

		new->next = NULL;
		if (new->memory == 128)
		{
			insert_tail_node_list(node_list[0], new);
		}
		else if (new->memory == 256)
		{
			insert_tail_node_list(node_list[1], new);
		}
		else if (new->memory == 1024)
		{
			insert_tail_node_list(node_list[2], new);
		}
		else
		{
			printf("Error cluster\n");
			exit(EXIT_FAILURE);
		}
		total_number_nodes += 1;
		
		nb_cores += 20;
	}
 	fclose(f);
}

/**
 * The workload is an input file.
 * It is read here to initialize linked list of jobs;
 **/
void read_workload(char* input_job_file)
{
	job_list = (struct Job_List*) malloc(sizeof(struct Job_List));
	job_list->head = NULL;
	job_list->tail = NULL;
	job_list_to_start_from_history = (struct Job_List*) malloc(sizeof(struct Job_List));
	job_list_to_start_from_history->head = NULL;
	job_list_to_start_from_history->tail = NULL;
	scheduled_job_list = (struct Job_List*) malloc(sizeof(struct Job_List));
	scheduled_job_list->head = NULL;
	scheduled_job_list->tail = NULL;
	running_jobs = (struct Job_List*) malloc(sizeof(struct Job_List));
	running_jobs->head = NULL;
	running_jobs->tail = NULL;
	new_job_list = (struct Job_List*) malloc(sizeof(struct Job_List));
	new_job_list->head = NULL;
	new_job_list->tail = NULL;
	
	FILE *f = fopen(input_job_file, "r");
	if (!f)
	{
		perror("fopen");
        exit(EXIT_FAILURE);
	}
	
	int index_node = 0;
	char s[100];
    char id[100];
    char subtime[100];
    char delay[100];
    char walltime[100];
    char cores[100];
    char data[100];
    char data_size[100];
    char workload[100];
    char start_time_from_history[100];
    char start_node_from_history[100];
        
    while (fscanf(f, "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s", s, s, id, s, subtime, s, delay, s, walltime, s, cores, s, s, s, data, s, data_size, s, workload, s, start_time_from_history, s, start_node_from_history, s) == 24)
	{
		total_number_jobs += 1;
		
		struct Job *new = (struct Job*) malloc(sizeof(struct Job));
		new->unique_id = atoi(id);
		new->subtime = atoi(subtime);
		new->delay = atoi(delay);	
		new->walltime = atoi(walltime);
		new->cores = atoi(cores);
		new->data = atoi(data);
		new->data_size = atof(data_size);
		new->workload = atoi(workload);
		new->start_time_from_history = atoi(start_time_from_history);
		new->node_from_history = atoi(start_node_from_history);
				
		index_node = 0;
		new->index_node_list = index_node;
		
		new->start_time = -1; 
		new->end_time = 0; 
		new->end_before_walltime = false;
		
		new->node_used = (struct Node*) malloc(sizeof(struct Node));
		new->node_used = NULL;
		
		new->cores_used = (int*) malloc(new->cores*sizeof(int));
		new->transfer_time = 0;
		new->waiting_for_a_load_time = 0;
		new->next = NULL;
		
		/* Add in job list or job to start from history */
		if (new->workload != -2)
		{
			insert_tail_job_list(job_list, new);
		}
		else
		{
			insert_job_in_sorted_list(job_list_to_start_from_history, new);
		}
	}
	fclose(f);
}

int get_nb_job_to_evaluate(struct Job* l)
{
	struct Job *j = l;
	int count = 0;
	while (j != NULL)
	{
		if (j->workload == 1)
		{
			count += 1;
		}
		j = j->next;
	}
	return count;
}

/** 
 * Get the starting time of Day 0.
 * Day 0 is the first day we start to 
 * schedule with the desired scheduler.
 **/
int get_first_time_day_0(struct Job* l)
{
	struct Job *j = l;
	while (j != NULL && j->workload != 0)
	{
		j = j->next;
	}
	if (j == NULL)
	{
		printf("No jobs of category 0. First subtime day 0 is set to 0.\n");
		return 0;
	}
	return j->subtime;
}

/**
 * Used for printing the cluster' usage.
 **/
void write_in_file_first_times_all_day(struct Job* l, int first_subtime_day_0)
{
	struct Job *j = l;
	bool first_before_0 = false;
	bool first_day_1 = false;
	bool first_day_2 = false;
	int first_subtime_before_0 = 0;
	int first_subtime_day_1 = 0;
	int first_subtime_day_2 = 0;
	
	while (j != NULL)
	{
		if (j->workload == -1 && first_before_0 == false)
		{
			first_before_0 = true;
			first_subtime_before_0 = j->subtime;
		}
		else if (j->workload == 1 && first_day_1 == false)
		{
			first_day_1 = true;
			first_subtime_day_1 = j->subtime - first_subtime_day_0;
		}
		else if (j->workload == 2 && first_day_2 == false)
		{
			first_day_2 = true;
			first_subtime_day_2 = j->subtime - first_subtime_day_0;
		}
		j = j->next;
	}
	
	FILE *f = fopen("outputs/Start_end_evaluated_slice.txt", "w");
	if (!f)
	{
		perror("fopen");
        exit(EXIT_FAILURE);
	}
	fprintf(f, "%d %d %d %d", first_subtime_before_0, 0, first_subtime_day_1, first_subtime_day_2);
	fclose(f);
}
