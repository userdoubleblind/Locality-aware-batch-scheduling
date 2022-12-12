/*
 * Copyright (C) - Anonymous for double blind submission.
 */
 
#include <main.h>

/**
 * Start the jobs that were started before Day 0.
 * Uses the log of historical jobs to replicate the same start time and use the same node.
 **/
void get_state_before_day_0_scheduler(struct Job* j2, struct Node_List** n, int t)
{
	int i = 0;
	int nb_job_to_delete = 0;
	
	/* Get number of node in each size category */
	struct Node *temp = (struct Node*) malloc(sizeof(struct Node));
	int* nb_node = malloc(3*sizeof(int));
	for (i = 0; i < 3; i++)
	{
		nb_node[i] = 0;
		temp = node_list[i]->head;
		while (temp != NULL)
		{
			temp = temp->next;
			nb_node[i] += 1;
		}
	}	
	free(temp);

	struct Job* j = j2;
	while (j != NULL)
	{		
		int time_since_start = t - j->start_time_from_history;
		j->delay -= time_since_start;
		if(j->delay <= 0)
		{
			j->delay = 1;
		}
		j->walltime -= time_since_start;
		if(j->walltime <= 0)
		{
			j->walltime = 1;
		}
		
		struct Node *choosen_node = (struct Node*) malloc(sizeof(struct Node));
		int index_node = (j->node_from_history - 1)%(nb_node[0] + nb_node[1] + nb_node[2]);
		if (index_node >= nb_node[0])
		{
			if (index_node >= nb_node[0] + nb_node[1])
			{
				choosen_node = node_list[2]->head;
				for (i = 0; i < index_node - nb_node[0] - nb_node[1]; i++)
				{
					choosen_node = choosen_node->next;
				}
			}
			else
			{
				choosen_node = node_list[1]->head;
				for (i = 0; i < index_node - nb_node[0]; i++)
				{
					choosen_node = choosen_node->next;
				}
			}
		}
		else
		{
			choosen_node = node_list[0]->head;
			for (i = 0; i < index_node; i++)
			{
				choosen_node = choosen_node->next;
			}
		}
				
		schedule_job_specific_node_at_earliest_available_time(j, choosen_node, t);
		nb_job_to_delete += 1;
		
		/* Add the start time of the job in the list of starting times. */		
		insert_next_time_in_sorted_list(start_times, j->start_time);
				
		j = j->next;
	}
	
	/* Remove from the main job list and put it in scheduled job list. */
	for (i = 0; i < nb_job_to_delete; i++)
	{
		j = job_list_to_start_from_history->head;
		copy_delete_insert_job_list(job_list_to_start_from_history, scheduled_job_list, j);
	}

	free(nb_node);
}

/** FCFS **/
void FCFS(struct Job* head_job, struct Node_List** head_node, int t)
{		
	/* 
	 * Used to avoid scheduling more than necessary with the line: "if (nb_non_available_cores < nb_cores)".
	 * If all the cores are covered and we don't use backfilling, we can stop scheduling.
	 */
	int nb_non_available_cores = get_nb_non_available_cores(node_list, t);

	struct Job* j = head_job;
	while (j != NULL)
	{
		if (nb_non_available_cores < nb_cores)
		{			
			/* Schedule j on the earliest available node. */
			nb_non_available_cores = schedule_job_on_earliest_available_cores(j, head_node, t, nb_non_available_cores);
			
			/* Add it's start time in the list used to stor all the next start times of jobs. */
			insert_next_time_in_sorted_list(start_times, j->start_time);
			
			j = j->next;
		}
		else /* Stop scheduling because all cores are covered */
		{			
			/* The start time of the non-scheduled jobs are set to -1 to avoid any further issue. */
			while (j != NULL)
			{
				j->start_time = -1;
				j = j->next;
			}
			break;
		}
	}
}

/**
 * Same as FCFS but with backfilling.
 **/
void FCFS_BF(struct Job* head_job, struct Node_List** head_node, int t)
{
	/*
	 * Here this will be usefull to know if we need to continue scheduling normally or if we should only check for backfilling.
	 * If all cores are covered I only check for backfilling.
	 */
	int nb_non_available_cores = get_nb_non_available_cores(node_list, t);
	/* 
	 * Here we look at the number of cores that are running a job or will be running a job at time t.
	 * If all the cores are in this situation we can stop scheduling. Else we continue in case of a backfill.
	 */
	int nb_non_available_cores_at_time_t = global_nb_non_available_cores_at_time_t;

	struct Job* j = head_job;
	while (j != NULL)
	{
		if (nb_non_available_cores < nb_cores)
		{			
			schedule_job_on_earliest_available_cores_with_conservative_backfill(j, head_node, t, backfill_mode, &nb_non_available_cores, &nb_non_available_cores_at_time_t);
						
			if (j->start_time < t)
			{
				printf("Error: j->start_time < t\n"); fflush(stdout);
				exit(EXIT_FAILURE);
			}
			
			insert_next_time_in_sorted_list(start_times, j->start_time);
			j = j->next;
		}
		else if (nb_non_available_cores_at_time_t < nb_cores)
		{
			/* Only check backfill because all cores are covered */						
			if (j->cores <= biggest_hole && only_check_conservative_backfill(j, head_node, t, backfill_mode, &nb_non_available_cores_at_time_t) == true)
			{
				if (j->start_time < t)
				{
					printf("Error: j->start_time < t\n");
					exit(1);
				}
					
				insert_next_time_in_sorted_list(start_times, j->start_time);
			}
			else
			{
				j->start_time = -1;
			}
			
			j = j->next;
		}
		else
		{
			while (j != NULL)
			{
				j->start_time = -1;
				j = j->next;
			}
			break;
		}
	}
	global_nb_non_available_cores_at_time_t = nb_non_available_cores_at_time_t;
}

/** 
 * Function used for LEA, LEO and LEM.
 * LEO has start_immediately_if_EAT_is_t set to 1.
 * LEM has mixed_strategy set to 1.
 * multiplier_file_to_load is the weight on the load time when computing a score.
 * It is set to 500.
 * multiplier_file_evicted is the weight on the eviction time. It is set to 1. **/
void LEA(struct Job* head_job, struct Node_List** head_node, int t, int multiplier_file_to_load, int multiplier_file_evicted, int start_immediately_if_EAT_is_t, int mixed_strategy)
{
	int nb_non_available_cores = get_nb_non_available_cores(node_list, t); /* Get the number of cores that are already running a job, and thus not available. */
	int i = 0;
	int min_score = -1;
	int earliest_available_time = 0;
	float time_to_load_file = 0;
	bool is_being_loaded = false;
	float time_to_reload_evicted_files = 0;
	int score = 0;
	int min_time = 0;
	int choosen_time_to_load_file = 0;
	bool found = false;
				
	/* For LEO, we wan to save the multiplier to use them when needed. */
	int temp_multiplier_file_to_load = multiplier_file_to_load;
	int temp_multiplier_file_evicted = multiplier_file_evicted;
	//~ int temp_multiplier_nb_copy = multiplier_nb_copy;

	int temp_running_nodes = running_nodes;
	
	/* Get intervals of data. Each data contains an intervals of times at which it will
	 * be used by a job. */ 
	get_current_intervals(head_node, t);
				
	/* Let's breakdown LEA's strategy step by step.
	 * 1. Loop on available jobs. */
	struct Job* j = head_job;
	while (j != NULL)
	{
		if (nb_non_available_cores < nb_cores)
		{			
			/* 1.1.LEO. Re-setting the multipliers for LEO */
			if (start_immediately_if_EAT_is_t == 1)
			{
				multiplier_file_to_load = temp_multiplier_file_to_load;
				multiplier_file_evicted = temp_multiplier_file_evicted;
			}
			
			/* 1.1.LEM. Get the occupation for LEM */
			if (mixed_strategy == 1)
			{
				/* Get the curent occupation of the cluster. If it's inferior to the threshold we use parameters that replicate EFT. EFT we keep the same parameters as LEA. */
				if ((temp_running_nodes*100)/486 < busy_cluster_threshold)
				{
					multiplier_file_to_load = 1;
					multiplier_file_evicted = 0;
				}
				else
				{
					multiplier_file_to_load = temp_multiplier_file_to_load;
					multiplier_file_evicted = temp_multiplier_file_evicted;
				}
			}
			
			/* 2. Choose a node. */		
			min_score = -1;
			earliest_available_time = 0;
			
			
			is_being_loaded = false;
			time_to_reload_evicted_files = 0;
			//~ nb_copy_file_to_load = 0;
			
			/* In which node size I can pick. */
			
						
			for (i = 0; i <= 2; i++)
			{
				/* Loop over all nodes. */
				struct Node* n = head_node[i]->head;
				while (n != NULL)
				{										
					/* 2.1. A = Get the earliest available time (t_k) from the number of cores required by the job. */
					earliest_available_time = n->cores[j->cores - 1]->available_time;
					if (earliest_available_time < t)				
					{
						earliest_available_time = t;
					}
					
					/* 2.1.1.LEO. LEO checks if the node is available immediatly. If yes it puts the multiplier to 1 and 0. */
					if (start_immediately_if_EAT_is_t == 1 && earliest_available_time == t)
					{
						multiplier_file_to_load = 1;
						multiplier_file_evicted = 0;
					}
					
					/* If we are still able to beat the min score we continue to compute the score on the current node. */								
					if (min_score == -1 || earliest_available_time < min_score)
					{						
						/* 2.2. B = Compute the time to load the input file. */
						if (j->data == 0)
						{
							time_to_load_file = 0;
						}
						else
						{
							time_to_load_file = is_my_file_on_node_at_certain_time_and_transfer_time(earliest_available_time, n, t, j->data, j->data_size, &is_being_loaded);
						}
																	
						if (min_score == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file < min_score)
						{
							/* 2.3. Get the amount of files that will be lost because of this load by computing the amount of data that end at the earliest time, excluding current file of course. */
							if (multiplier_file_evicted == 0)
							{
								time_to_reload_evicted_files = 0;
							}
							else
							{
								time_to_reload_evicted_files = time_to_reload_percentage_of_files_ended_at_certain_time(earliest_available_time, n, j->data, (float) j->cores/20);
							}
															
							/* 2.4. Compute node's score. */
							score = earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files;
																
							/* 2.5. Get minimum score/ */
							if (min_score == -1 || min_score > score)
							{
								min_time = earliest_available_time;
								min_score = score;
								j->node_used = n;
								choosen_time_to_load_file = time_to_load_file;
							}
						}
					}					
					n = n->next;
				}
			}
			
			j->transfer_time = choosen_time_to_load_file;
					
			/* Get start time and update available times of the cores. */
			j->start_time = min_time;
			j->end_time = min_time + j->walltime;
			
			/* For LEM, we update the number of nodes running a jobs if the start of the scheduled jobs is the current time. */
			if (mixed_strategy == 1 && j->node_used->n_available_cores == 20 && j->start_time == t)
			{				
				bool new_running_node = true;
				for (int v = 0; v < 20; v++)
				{
					if (j->node_used->cores[v]->available_time > t)
					{
						new_running_node = false;
						break;
					}
				}
				if (new_running_node == true)
				{
					temp_running_nodes += 1;
				}
			}
			
			/* Updates the choosen cores next available time. */
			for (int k = 0; k < j->cores; k++)
			{
				j->cores_used[k] = j->node_used->cores[k]->unique_id;
				if (j->node_used->cores[k]->available_time <= t)
				{
					nb_non_available_cores += 1;
				}
				j->node_used->cores[k]->available_time = min_time + j->walltime;
			}
			
			/* Updating the intervals at which the data will be used on the node. */
			found = false;
			struct Data* d = j->node_used->data->head;			
			while (d != NULL)
			{
				if (d->unique_id == j->data)
				{
					found = true;
					create_and_insert_tail_interval_list(d->intervals, j->start_time);
					create_and_insert_tail_interval_list(d->intervals, j->start_time + j->transfer_time);
					create_and_insert_tail_interval_list(d->intervals, j->end_time);
					break;
				}
				d = d->next;
			}
			if (found == false)
			{
				/* Create a class Data for this node. */
				struct Data* new = (struct Data*) malloc(sizeof(struct Data));
				new->next = NULL;
				new->unique_id = j->data;
				new->start_time = -1;
				new->end_time = -1;
				new->nb_task_using_it = 0;				
				new->intervals = (struct Interval_List*) malloc(sizeof(struct Interval_List));
				new->intervals->head = NULL;
				new->intervals->tail = NULL;
				create_and_insert_tail_interval_list(new->intervals, j->start_time);
				create_and_insert_tail_interval_list(new->intervals, j->start_time + j->transfer_time);
				create_and_insert_tail_interval_list(new->intervals, j->end_time);
				new->size = j->data_size;
				insert_tail_data_list(j->node_used->data, new);
			}			
						
			/* Need to sort cores by available time after each schedule of a job. */
			sort_cores_by_available_time_in_specific_node(j->node_used);
																
			/* Insert in start times. */
			insert_next_time_in_sorted_list(start_times, j->start_time);			
			
			j = j->next;
		}				
		else
		{			
			while (j != NULL)
			{
				j->start_time = -1;
				j = j->next;
			}
			break;
		}
	}
}

/** Used for LEA-BF, LEO-BF, LEM-BF 
 * Similar to LEA but also compute a score for the jobs that can be backfilled. **/
void LEA_BF(struct Job* head_job, struct Node_List** head_node, int t, int multiplier_file_to_load, int multiplier_file_evicted, int start_immediately_if_EAT_is_t, int mixed_strategy)
{
	int nb_non_available_cores = get_nb_non_available_cores(node_list, t);
	int nb_non_available_cores_at_time_t = global_nb_non_available_cores_at_time_t;
	
	/* Get intervals of data. */ 
	get_current_intervals(head_node, t);
	
	/* For LEM to know the proportion of used nodes. */
	int temp_running_nodes = running_nodes;
	
	struct Job* j = head_job;
	
	while (j != NULL)
	{		
		if (nb_non_available_cores < nb_cores)
		{		
			/* For LEM */	
			if (mixed_strategy == 1)
			{
				if ((temp_running_nodes*100)/486 < busy_cluster_threshold)
				{
					schedule_job_fcfs_score_with_conservative_backfill(j, head_node, t, 1, 0, start_immediately_if_EAT_is_t, &nb_non_available_cores, &nb_non_available_cores_at_time_t, mixed_strategy, &temp_running_nodes);
				}
				else
				{
					schedule_job_fcfs_score_with_conservative_backfill(j, head_node, t, multiplier_file_to_load, multiplier_file_evicted, start_immediately_if_EAT_is_t, &nb_non_available_cores, &nb_non_available_cores_at_time_t, mixed_strategy, &temp_running_nodes);
				}
			}
			else /* For LEA and LEO. */
			{
				schedule_job_fcfs_score_with_conservative_backfill(j, head_node, t, multiplier_file_to_load, multiplier_file_evicted, start_immediately_if_EAT_is_t, &nb_non_available_cores, &nb_non_available_cores_at_time_t, mixed_strategy, &temp_running_nodes);
			}
						
			if (j->start_time < t)
			{
				printf("Error: j->start_time < t\n");
				exit(EXIT_FAILURE);
			}
			
			insert_next_time_in_sorted_list(start_times, j->start_time);
			j = j->next;
		}
		else if (nb_non_available_cores_at_time_t < nb_cores)
		{
			/* Only check backfill */				
			if (mixed_strategy == 1)
			{
				if ((temp_running_nodes*100)/486 < busy_cluster_threshold)
				{
					if (j->cores <= biggest_hole && only_check_conservative_backfill_with_a_score(j, head_node, t, backfill_mode, &nb_non_available_cores_at_time_t, 1, 0, start_immediately_if_EAT_is_t) == true)
					{
						if (j->start_time < t)
						{
							printf("Error: j->start_time < t\n");
							exit(1);
						}
						insert_next_time_in_sorted_list(start_times, j->start_time);
					}
					else
					{
						j->start_time = -1;
					}
				}
				else
				{
					if (j->cores <= biggest_hole && only_check_conservative_backfill_with_a_score(j, head_node, t, backfill_mode, &nb_non_available_cores_at_time_t, multiplier_file_to_load, multiplier_file_evicted, start_immediately_if_EAT_is_t) == true)
					{
						if (j->start_time < t)
						{
							printf("Error: j->start_time < t\n");
							exit(1);
						}
						insert_next_time_in_sorted_list(start_times, j->start_time);
					}
					else
					{
						j->start_time = -1;
					}
				}
			}
			else
			{
				if (j->cores <= biggest_hole && only_check_conservative_backfill_with_a_score(j, head_node, t, backfill_mode, &nb_non_available_cores_at_time_t, multiplier_file_to_load, multiplier_file_evicted, start_immediately_if_EAT_is_t) == true)
				{
					if (j->start_time < t)
					{
						printf("Error: j->start_time < t\n");
						exit(1);
					}
					insert_next_time_in_sorted_list(start_times, j->start_time);
				}
				else
				{
					j->start_time = -1;
				}
			}			
			j = j->next;
		}
		else
		{			
			while (j != NULL)
			{
				j->start_time = -1;
				j = j->next;
			}
			break;
		}
	}
	global_nb_non_available_cores_at_time_t = nb_non_available_cores_at_time_t;
}
