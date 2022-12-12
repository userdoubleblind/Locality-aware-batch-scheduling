/*
 * Copyright (C) - Anonymous for double blind submission.
 */
 
#include <main.h>

/** 
 * This file contains functions that can be called
 * by any scheduler. They return jobs depending
 * on the parameters that want to be maximized, return
 * values from a node list, etc... 
**/

/** Return the smallest time that can be used to compute a job of size
 * nb_cores cores. Do not deal with backfilling. **/
int get_min_EAT(struct Node_List** head_node, int first_node_size_to_choose_from, int last_node_size_to_choose_from, int nb_cores, int t)
{
	int min_EAT = INT_MAX;
	int i = 0;
	
	for (i = 0; i <= 2; i++)
	{
		struct Node* n = head_node[i]->head;
		while (n != NULL)
		{
			if (n->cores[nb_cores - 1]->available_time < min_EAT)
			{
				if (n->cores[nb_cores - 1]->available_time <= t)	
				{					
					return t;
				}
				else
				{
					min_EAT = n->cores[nb_cores - 1]->available_time;
				}
			}
			n = n->next;
		}
	}
	if (min_EAT == INT_MAX)
	{
		perror("EAT is INT_MAX in get_min_EAT");
		exit(EXIT_FAILURE);
	}
	return min_EAT;
}

/** Schedule a job on the node with the smallest start time.
 * Does not deal with backfilling. Used by FCFS. **/
int schedule_job_on_earliest_available_cores(struct Job* j, struct Node_List** head_node, int t, int nb_non_available_cores)
{
	int i = 0;
	int min_time = -1;
	int earliest_available_time = 0;
		
	/* Finding the node with the earliest available time. */
	for (i = 0; i <= 2; i++)
	{
		struct Node* n = head_node[i]->head;
		while (n != NULL)
		{
			earliest_available_time = n->cores[j->cores - 1]->available_time;
			if (earliest_available_time < t) /* A core can't be available before t. This happens when a node is idling. */				
			{
				earliest_available_time = t;
			}
			if (min_time == -1 || min_time > earliest_available_time)
			{
				min_time = earliest_available_time;
				j->node_used = n;
				
				if (min_time == t)
				{
					i = 3;
					break;
				}
			}
			n = n->next;
		}
	}
		
	/* Update infos on the job and cores. */
	j->start_time = min_time;
	j->end_time = min_time + j->walltime;
		
	for (i = 0; i < j->cores; i++)
	{
		j->cores_used[i] = j->node_used->cores[i]->unique_id;
		if (j->node_used->cores[i]->available_time <= t)
		{
			nb_non_available_cores += 1;
		}
		j->node_used->cores[i]->available_time = min_time + j->walltime;
	}
	
	#ifdef PRINT
	print_decision_in_scheduler(j);
	#endif

	sort_cores_by_available_time_in_specific_node(j->node_used);
		
	return nb_non_available_cores;
}

/** Used by FCFS-BF. **/
void schedule_job_on_earliest_available_cores_with_conservative_backfill(struct Job* j, struct Node_List** head_node, int t, int backfill_mode, int* nb_non_available_cores, int* nb_non_available_cores_at_time_t)
{
	int nb_cores_from_hole = 0;
	int nb_cores_from_outside = 0;
	
	int i = 0;
	int min_time = -1;
	int earliest_available_time = 0;
	bool backfilled_job = false;

	for (i = 0; i <= 2; i++)
	{
		struct Node* n = head_node[i]->head;
		while (n != NULL)
		{
				earliest_available_time = n->cores[j->cores - 1]->available_time;
				if (earliest_available_time < t)	
				{
					earliest_available_time = t;
				}
								
				if (min_time == -1 || min_time > earliest_available_time)
				{
					min_time = earliest_available_time;
					j->node_used = n;
						
					if (min_time == t)
					{							
						i = 3;
						break;
					}
				}
				backfilled_job = can_it_get_backfilled(j, n, t, &nb_cores_from_hole, &nb_cores_from_outside);
				if (backfilled_job == true)
				{
					min_time = t;
					j->node_used = n;
					i = 3;
					break;
				}
			n = n->next;
		}
	}
		
	/* Update infos on the job and on cores. */
	j->start_time = min_time;
	j->end_time = min_time + j->walltime;
	 
	if (backfilled_job == true)
	{
		update_cores_for_backfilled_job(j, t, nb_cores_from_hole, nb_cores_from_outside);
		*nb_non_available_cores_at_time_t += j->cores;
		*nb_non_available_cores += nb_cores_from_outside;
		
		if (j->node_used->unique_id == biggest_hole_unique_id)
		{
			get_new_biggest_hole(head_node);
		}
	}
	else
	{		
		if (j->start_time == t)
		{
			*nb_non_available_cores_at_time_t += j->cores;
		}
		
		if (backfill_mode == 1 || backfill_mode == 2)
		{			
			fill_cores_minimize_holes(j, true, backfill_mode, t, nb_non_available_cores);
			
			if (j->node_used->unique_id == biggest_hole_unique_id)
			{
				get_new_biggest_hole(head_node);
			}
		}
		else
		{
			for (i = 0; i < j->cores; i++)
			{
				j->cores_used[i] = j->node_used->cores[i]->unique_id;
						
				if (j->node_used->cores[i]->available_time <= t)
				{
					*nb_non_available_cores += 1;
				}
						
				if (j->node_used->cores[i]->available_time <= t && j->start_time > t)
				{							
					j->node_used->number_cores_in_a_hole += 1;
														
					struct Core_in_a_hole* new = (struct Core_in_a_hole*) malloc(sizeof(struct Core_in_a_hole));
					new->unique_id = j->node_used->cores[i]->unique_id;
					new->start_time_of_the_hole = min_time;
					new->next = NULL;
							
					if (j->node_used->cores_in_a_hole == NULL)
					{
						initialize_cores_in_a_hole(j->node_used->cores_in_a_hole, new);
					}
					else
					{
						insert_cores_in_a_hole_list_sorted_decreasing_order(j->node_used->cores_in_a_hole, new);
					}
				}
				j->node_used->cores[i]->available_time = j->start_time + j->walltime;
			}
			if (j->node_used->number_cores_in_a_hole > biggest_hole)
			{
				biggest_hole = j->node_used->number_cores_in_a_hole;
				biggest_hole_unique_id = j->node_used->unique_id;
			}
		}
	}
	if (backfilled_job == false || backfill_mode > 0 || nb_cores_from_outside > 0)
	{
		sort_cores_by_available_time_in_specific_node(j->node_used);
	}
}

/** 
 * Called by LEA-BF, LEO-BF and LEM-BF  
 * Compute a score for a job, backfilled or not, a choose the best one.
 **/
void schedule_job_fcfs_score_with_conservative_backfill(struct Job* j, struct Node_List** head_node, int t, int multiplier_file_to_load, int multiplier_file_evicted, int start_immediately_if_EAT_is_t, int* nb_non_available_cores, int* nb_non_available_cores_at_time_t, int mixed_strategy, int* temp_running_nodes)
{		
	int nb_cores_from_hole = 0;
	int nb_cores_from_outside = 0;
	int nb_cores_from_outside_remembered = 0;
	int choosen_nb_cores_from_hole = 0;
	int choosen_nb_cores_from_outside = 0;
	
	int i = 0;
	int min_time = -1;
	int earliest_available_time = 0;
	bool backfilled_job = false;
	int min_score = -1;
	float time_to_load_file = 0;
	bool is_being_loaded = false;
	float time_to_reload_evicted_files = 0;
	int score = 0;
	int choosen_time_to_load_file = 0;
	bool found = false;
	
	bool want_to_save_times_for_backfill = false;
	float time_to_load_file_saved = 0;
	float time_to_reload_evicted_files_saved = 0;
	
	for (i = 0; i <= 2; i++)
	{
		struct Node* n = head_node[i]->head;
		while (n != NULL)
		{
			want_to_save_times_for_backfill = false;
			earliest_available_time = n->cores[j->cores - 1]->available_time;
				
			if (earliest_available_time <= t)			
			{
				earliest_available_time = t;
				want_to_save_times_for_backfill = true; 
				time_to_load_file_saved = -1;
				time_to_reload_evicted_files_saved = -1;
			}
					
			if (start_immediately_if_EAT_is_t == 1 && earliest_available_time == t)
			{
				multiplier_file_to_load = 1;
				multiplier_file_evicted = 0;
			}
					
			if (min_score == -1 || earliest_available_time < min_score)
			{
				if (j->data == 0)
				{
					time_to_load_file = 0;
				}
				else
				{
					time_to_load_file = is_my_file_on_node_at_certain_time_and_transfer_time(earliest_available_time, n, t, j->data, j->data_size, &is_being_loaded);
				}
					
				if (want_to_save_times_for_backfill == true)
				{
					time_to_load_file_saved = time_to_load_file;
				}
				
				if (min_score == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file < min_score)
				{
					if (multiplier_file_evicted == 0)
					{
						time_to_reload_evicted_files = 0;
					}
					else
					{
						time_to_reload_evicted_files = time_to_reload_percentage_of_files_ended_at_certain_time(earliest_available_time, n, j->data, (float) j->cores/20);
					}
						
					if (want_to_save_times_for_backfill == true)
					{
						time_to_reload_evicted_files_saved = time_to_reload_evicted_files;
					}
																			
					score = earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files;
																						
					if (min_score == -1 || min_score > score)
					{
						min_time = earliest_available_time;
						min_score = score;
						j->node_used = n;
						choosen_time_to_load_file = time_to_load_file;
						backfilled_job = false;
								
						if (min_time == t && min_score == t)
						{											
							i = 3;
							break;
						}
					}
				}
			}
									
			if (can_it_get_backfilled(j, n, t, &nb_cores_from_hole, &nb_cores_from_outside) == true)
			{		
				/* Computing the score in the hole. */				
				earliest_available_time = t;
												
				if (min_score == -1 || earliest_available_time < min_score)
				{
					if (j->data == 0)
					{
						time_to_load_file = 0;
					}
					else if (want_to_save_times_for_backfill == true && time_to_load_file_saved != -1)
					{
						time_to_load_file = time_to_load_file_saved;
					}
					else
					{
						time_to_load_file = is_my_file_on_node_at_certain_time_and_transfer_time(earliest_available_time, n, t, j->data, j->data_size, &is_being_loaded);
					}
																	
					if (min_score == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file < min_score)
					{	
						if (multiplier_file_evicted == 0)
						{
							time_to_reload_evicted_files = 0;
						}
						else if (want_to_save_times_for_backfill == true && time_to_reload_evicted_files_saved != -1)
						{
							time_to_reload_evicted_files = time_to_reload_evicted_files_saved;
						}
						else
						{
							time_to_reload_evicted_files = time_to_reload_percentage_of_files_ended_at_certain_time(earliest_available_time, n, j->data, (float) j->cores/20);
						}
								
						score = earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files;
																								
						if (min_score == -1 || min_score > score)
						{
							min_time = earliest_available_time;
							min_score = score;
							j->node_used = n;
							choosen_time_to_load_file = time_to_load_file;
							backfilled_job = true;
										
							choosen_nb_cores_from_hole = nb_cores_from_hole;
							choosen_nb_cores_from_outside = nb_cores_from_outside;
										
							if (min_time == t && min_score == t)
							{											
								i = 3;
								break;
							}
						}
					}
				}
			}
			n = n->next;
		}
	}
		
	/* Update infos on the job and on cores. */
	j->start_time = min_time;
	j->end_time = min_time + j->walltime;
	j->transfer_time = choosen_time_to_load_file;
	/* Need to add here intervals for current scheduling. */
	found = false;
	
	/* For LEM-BF */
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
			*temp_running_nodes += 1;
		}
	}
	
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
	
	if (backfilled_job == true)
	{		
		update_cores_for_backfilled_job(j, t, choosen_nb_cores_from_hole, choosen_nb_cores_from_outside);
		*nb_non_available_cores_at_time_t += j->cores;
		*nb_non_available_cores += choosen_nb_cores_from_outside;
				
		if (j->node_used->unique_id == biggest_hole_unique_id)
		{
			get_new_biggest_hole(head_node);
		}
	}
	else
	{
		if (j->start_time == t)
		{
			*nb_non_available_cores_at_time_t += j->cores;
		}
				
		if (backfill_mode == 1 || backfill_mode == 2)
		{
			fill_cores_minimize_holes(j, true, backfill_mode, t, nb_non_available_cores);
			
			if (j->node_used->unique_id == biggest_hole_unique_id)
			{
				get_new_biggest_hole(head_node);
			}
		}
		else
		{
			for (i = 0; i < j->cores; i++)
			{
				j->cores_used[i] = j->node_used->cores[i]->unique_id;

				if (j->node_used->cores[i]->available_time <= t)
				{
					*nb_non_available_cores += 1;
				}
							
				if (j->node_used->cores[i]->available_time <= t && min_time > t)
				{					
					j->node_used->number_cores_in_a_hole += 1;
					struct Core_in_a_hole* new = (struct Core_in_a_hole*) malloc(sizeof(struct Core_in_a_hole));
					new->unique_id = j->node_used->cores[i]->unique_id;
					new->start_time_of_the_hole = min_time;
					new->next = NULL;
					if (j->node_used->cores_in_a_hole == NULL)
					{
						initialize_cores_in_a_hole(j->node_used->cores_in_a_hole, new);
					}
					else
					{
						insert_cores_in_a_hole_list_sorted_decreasing_order(j->node_used->cores_in_a_hole, new);
					}
				}
				
				j->node_used->cores[i]->available_time = min_time + j->walltime;
			}
			
			if (j->node_used->number_cores_in_a_hole > biggest_hole)
			{
				biggest_hole = j->node_used->number_cores_in_a_hole;
				biggest_hole_unique_id = j->node_used->unique_id;
			}
			
		}
	}
	
	/* No need to sort cores if the job was bakfilled or if it didn't use cores from outside a hole. */
	if (backfilled_job == false || backfill_mode > 0 || nb_cores_from_outside_remembered > 0)
	{
		sort_cores_by_available_time_in_specific_node(j->node_used);
	}
}

/** Called at the beggining of each call of LEA. 
 * Fill the node's data list with intervals of the time at which the data is used
 * by a job.
 **/
void get_current_intervals(struct Node_List** head_node, int t)
{
	int i = 0;
	
	for (i = 0; i < 3; i++)
	{
		struct Node* n = head_node[i]->head;
		while (n != NULL)
		{
			struct Data* d = n->data->head;
			while (d != NULL)
			{
				d->intervals = (struct Interval_List*) malloc(sizeof(struct Interval_List));	
				d->intervals->head = NULL;
				d->intervals->tail = NULL;
					
				if (d->nb_task_using_it > 0)
				{
					create_and_insert_tail_interval_list(d->intervals, t);
					if (d->start_time < t)
					{
						create_and_insert_tail_interval_list(d->intervals, t);
					}
					else
					{
						create_and_insert_tail_interval_list(d->intervals, d->start_time);
					}
					create_and_insert_tail_interval_list(d->intervals, d->end_time);
				}
				else if (d->end_time >= t) /* Cas finis et re enchaine */
				{
					create_and_insert_tail_interval_list(d->intervals, t);
					create_and_insert_tail_interval_list(d->intervals, t);
					create_and_insert_tail_interval_list(d->intervals, t);
				}					
				d = d->next;
			}
			n = n->next;
		}
	}
}

/**
 * Return the number of cors currently running a job.
 **/
int get_nb_non_available_cores(struct Node_List** n, int t)
{
	int nb_non_available_cores = 0;
	int i = 0;
	int j = 0;
	for (i = 0; i < 3; i++)
	{
		struct Node* temp = n[i]->head;
		while (temp != NULL)
		{
			for (j = 0; j < 20; j++)
			{
				if (temp->cores[j]->available_time > t)
				{
					nb_non_available_cores += 1;
				}
			}			
			temp = temp->next;
		}
	}
	return nb_non_available_cores;
}

/**
 * Schedule a job on a specific node at the earliest available time.
 * Used by get_state_before_day_0_scheduler
 **/
void schedule_job_specific_node_at_earliest_available_time(struct Job* j, struct Node* n, int t)
{
	int i = 0;

	int earliest_available_time = n->cores[j->cores - 1]->available_time;
	if (earliest_available_time < t)
	{
		earliest_available_time = t;
	}

	j->node_used = n;
		
	j->start_time = earliest_available_time;
	j->end_time = earliest_available_time + j->walltime;
	for (i = 0; i < j->cores; i++)
	{
		j->cores_used[i] = n->cores[i]->unique_id;
				
		n->cores[i]->available_time = earliest_available_time + j->walltime;
	}	
	
	sort_cores_by_available_time_in_specific_node(n);
}

/**
 * Add an input file in a node. Also return the time it takes to load it, or the time a 
 * job will have to wait before using it if another job is already loading it.
 * Called in start_jobs.
 **/
void add_data_in_node (int data_unique_id, float data_size, struct Node* node_used, int t, int end_time, int* transfer_time, int* waiting_for_a_load_time, int delay, int walltime, int start_time, int cores)
{	
	bool data_is_on_node = false;
	struct Data* d = (struct Data*) malloc(sizeof(struct Data));
	d = node_used->data->head;
	while (d != NULL)
	{
		if (data_unique_id == d->unique_id) /* It is already on node */
		{
			if (d->nb_task_using_it > 0 || d->end_time == t) /* And is still valid! */
			{
				if (d->start_time > t) /* The job will have to wait for the data to be loaded by another job before starting */
				{
					*waiting_for_a_load_time = d->start_time - t;
				}
				else
				{
					*transfer_time = 0; /* No need to wait to start the job, data is already fully loaded */
				}
			}
			else /* Need to reload it */
			{
				*transfer_time = data_size/node_used->bandwidth;
				d->start_time = t + *transfer_time;
			}
			
			data_is_on_node = true;
			d->nb_task_using_it += 1;
			
			if (d->end_time < end_time)
			{
				d->end_time = end_time;
			}
			break;
		}
		d = d->next;
	}

	if (data_is_on_node == false) /* Need to load it */
	{		
		*transfer_time = data_size/node_used->bandwidth;
		/* Create a class Data for this node */
		struct Data* new = (struct Data*) malloc(sizeof(struct Data));
		new->unique_id = data_unique_id;
		new->start_time = t + *transfer_time;
		
		new->end_time = end_time;
		
		new->nb_task_using_it = 1;
		
		new->size = data_size;
		new->next = NULL;
		new->intervals = (struct Interval_List*) malloc(sizeof(struct Interval_List));
		insert_tail_data_list(node_used->data, new);
	}
}

/**
 * Remove an input file from a node when it's not used anymore.
 **/
void remove_data_from_node(struct Job* j, int t)
{
	struct Data* d = j->node_used->data->head;
	while (d != NULL)
	{
		if (j->data == d->unique_id)
		{
			d->nb_task_using_it -= 1;
				
			if (d->nb_task_using_it == 0) /* modif ? */
			{
				d->end_time = t;
			}
			break;
		}
		d = d->next;
	}
}

/**
 * Start jobs on the nodes. Go through the list of jobs to find the ones that must start at time t.
 * Also compute the time the job will wait for the loading of it's input file.
 * Also compute statistics to plot the cluster's usage for example.
 **/
void start_jobs(int t, struct Job* head)
{
	int i = 0;
	int k = 0;
	int overhead_of_load = 0;
	int min_between_delay_and_walltime = 0;
	int transfer_time = 0;
	int waiting_for_a_load_time = 0;
	
	struct Job* j = head;
	
	while (j != NULL)
	{
		if (j->start_time == t)
		{
			/* Update number of jobs left to schedule */
			nb_job_to_schedule -= 1;
			nb_cores_to_schedule -= j->cores;
			
			if (nb_job_to_schedule < 0)
			{
				printf("Error nb_job_to_schedule = %d\n", nb_job_to_schedule);
				exit(EXIT_FAILURE);
			}
			
			if (j->delay <= 0)
			{
				printf("Error delay is %d for job %d.\n", j->delay, j->unique_id);
				exit(EXIT_FAILURE);
			}
			
			/* Remove from list of starting times. */
			if (start_times->head != NULL && start_times->head->time == t)
			{
				delete_next_time_linked_list(start_times, t);
			}
			
			total_queue_time += j->start_time - j->subtime;
			
			transfer_time = 0;
			waiting_for_a_load_time = 0;
			
			if (j->data != 0)
			{
				/* Let's look if a data transfer is needed */
				add_data_in_node(j->data, j->data_size, j->node_used, t, j->end_time, &transfer_time, &waiting_for_a_load_time, j->delay, j->walltime, j->start_time, j->cores);
			}
			j->transfer_time = transfer_time;
			j->waiting_for_a_load_time = waiting_for_a_load_time;
			
			if (j->workload == 1 && (j->transfer_time == 0 || j->waiting_for_a_load_time != 0))
			{
				nb_data_reuse += 1;
			}
						
			overhead_of_load = 0;
			if (transfer_time == 0)
			{
				overhead_of_load = waiting_for_a_load_time;
			}
			else if (waiting_for_a_load_time == 0)
			{
				overhead_of_load = transfer_time;
			}
			else
			{
				printf("Error when computing transfer time.\n");
				exit(EXIT_FAILURE);
			}
			
			if (j->delay + overhead_of_load < j->walltime)
			{
				min_between_delay_and_walltime = j->delay + overhead_of_load;
				j->end_before_walltime = true;
			}
			else
			{
				min_between_delay_and_walltime = j->walltime;
				j->end_before_walltime = false;
			}
			j->end_time = j->start_time + min_between_delay_and_walltime;
			
			if (j->end_time <= j->start_time)
			{
				printf("Error end time job %d workload %d: %d -> %d\n min_between_delay_and_walltime is %d, walltime was %d, delay was %d\n", j->unique_id, j->workload, j->start_time, j->end_time, min_between_delay_and_walltime, j->walltime, j->delay);
				exit(EXIT_FAILURE);
			}
			
			insert_next_time_in_sorted_list(end_times, j->end_time);
			running_cores += j->cores;
						
			/* Defining cluster usage */
			if (j->node_used->n_available_cores == 20)
			{
				running_nodes += 1;
			}
			
			#ifdef PRINT_CLUSTER_USAGE
			if (j->workload == 1)
			{
				if (j->node_used->nb_jobs_workload_1 == 0)
				{
					running_nodes_workload_1 += 1;
				}
				j->node_used->nb_jobs_workload_1 += 1;
				running_cores_from_workload_1 += j->cores;
			}
			
			if (overhead_of_load > 0 && j->node_used->end_of_file_load < t + overhead_of_load)
			{
				j->node_used->end_of_file_load = t + overhead_of_load;
			}
			
			for (i = 0; i < j->cores; i++)
			{
				if (overhead_of_load > 0 && j->node_used->cores[j->cores_used[i]]->end_of_file_load < t + overhead_of_load)
				{
					j->node_used->cores[j->cores_used[i]]->end_of_file_load = t + overhead_of_load;
				}
			}		
			
			#endif
			
			j->node_used->n_available_cores -= j->cores;
			if (j->node_used->n_available_cores < 0 || j->node_used->n_available_cores > 20)
			{
				printf("ERROR start_jobs %d available cores at time %d.\n", j->node_used->n_available_cores, t); 
				print_cores_in_specific_node(j->node_used);
				exit(EXIT_FAILURE);
			}
			/* End of defining cluster usage */
						
			for (i = 0; i < j->cores; i++)
			{
				for (k = 0; k < 20; k++)
				{
					if (j->node_used->cores[k]->unique_id == j->cores_used[i])
					{
						j->node_used->cores[k]->running_job = true;
						j->node_used->cores[k]->running_job_end = j->start_time + j->walltime;
						break;
					}
				}
			}
			
			if (j->workload == 1)
			{
				nb_job_to_evaluate_started += 1;
			}
			to_print_job_csv(j, t);
		}
		j = j->next;
	}
	
	j = scheduled_job_list->head;
	while (j != NULL)
	{
		if (j->start_time == t)
		{
			struct Job* temp = j->next;
			copy_delete_insert_job_list(scheduled_job_list, running_jobs, j);
			j = temp;
		}
		else
		{
			j = j->next;
		}
	}
}

/**
 * Clean and remove jobs that ended from the list of jobs.
 **/
void end_jobs(struct Job* job_list_head, int t)
{	
	int i = 0;
	int k = 0;
	struct Job* j= job_list_head;
	while(j != NULL)
	{
		if (j->end_time == t) /* A job has finished, let's remove it from the cores, write its results and figure out if we need to fill */
		{
			/* Remove from list of ending times. */
			if (end_times->head != NULL && end_times->head->time == t)
			{				
				delete_next_time_linked_list(end_times, t);
			}
						
			finished_jobs += 1;
						
			/* Just to print in the terminal the progress. */
			if (finished_jobs%5000 == 0)
			{
				printf("Evaluated jobs: %d/%d | All jobs: %d/%d | T = %d.\n", nb_job_to_evaluate_started, nb_job_to_evaluate, finished_jobs, total_number_jobs, t); fflush(stdout);
			}
			
			running_cores -= j->cores;				
						
			/* Defining cluster usage */
			j->node_used->n_available_cores += j->cores;
			if (j->node_used->n_available_cores == 20)
			{
				running_nodes -= 1;
			}

			if (j->node_used->n_available_cores < 0 || j->node_used->n_available_cores > 20)
			{
				printf("ERROR on end jobs\n");
				exit(EXIT_FAILURE); 
			}
			
			#ifdef PRINT_CLUSTER_USAGE
			if (j->workload == 1)
			{
				if (j->node_used->nb_jobs_workload_1 == 1)
				{
					running_nodes_workload_1 -= 1;
				}
				j->node_used->nb_jobs_workload_1 -= 1;
							running_cores_from_workload_1 -= j->cores;
			}
			
			#endif
			/* End of defining cluster usage */
									
			for (i = 0; i < j->cores; i++)
			{
				for (k = 0; k < 20; k++)
				{
					if (j->node_used->cores[k]->unique_id == j->cores_used[i])
					{
						j->node_used->cores[k]->running_job = false;
						j->node_used->cores[k]->running_job_end = -1;
						break;
					}
				}
			}
			
			if (j->data != 0)
			{
				remove_data_from_node(j, t);
			}
		}
		j = j->next;
	}				
		
	/* Delete from running jobs. */
	j = job_list_head;
	while (j != NULL)
	{
		if (j->end_time == t)
		{
			struct Job* temp = j->next;
			delete_job_linked_list(running_jobs, j->unique_id);
			j = temp;
		}
		else
		{
			j = j->next;
		}
	}
}

/**
 * Reset the next available times of the cores and updates the running jobs.
 * Called when we want to re-schedule the jobs.
 **/
void reset_cores(struct Node_List** l, int t)
{	
	biggest_hole = 0;
	global_nb_non_available_cores_at_time_t = 0;
	
	int i = 0;
	int j = 0;
	for (i = 0; i < 3; i++)
	{
		struct Node* n = l[i]->head;
		while (n != NULL)
		{
			if (n->number_cores_in_a_hole != 0)
			{
				free_cores_in_a_hole(&n->cores_in_a_hole->head);
				n->number_cores_in_a_hole = 0;
			}
			
			for (j = 0; j < 20; j++)
			{
				if (n->cores[j]->running_job == false)
				{
					n->cores[j]->available_time = t;
				}
				else
				{
					if (n->cores[j]->running_job_end == -1)
					{
						perror("error in reset cores.\n");
						exit(EXIT_FAILURE);
					}
					n->cores[j]->available_time = n->cores[j]->running_job_end;
				}
			}
			
			/* Need to sort cores after a reset as well. */
			sort_cores_by_available_time_in_specific_node(n);
			
			n = n->next;
		}
	}
}

/** 
 * Look at a node at a certain time to check if current_data
 * will be available on it at that time.
 * Return the time it will take to load this data (it can be 0 if the data will be on node at that time).
 * Used by EFT, LEA, LEO and LEM.
 **/
float is_my_file_on_node_at_certain_time_and_transfer_time(int predicted_time, struct Node* n, int t, int current_data, float current_data_size, bool* is_being_loaded)
{
	struct Data* d = n->data->head;
	
	int* temp_interval_usage_time = malloc(3*sizeof(int));
	while (d != NULL)
	{		
		struct Interval* i = d->intervals->head;
		if (d->unique_id == current_data && i != NULL)
		{			
			while (i != NULL)
			{
				temp_interval_usage_time[0] = i->time;
				i = i->next;
				temp_interval_usage_time[1] = i->time;
				i = i->next;
				temp_interval_usage_time[2] = i->time;
								
				if (temp_interval_usage_time[0] <= predicted_time && temp_interval_usage_time[1] <= predicted_time && predicted_time <= temp_interval_usage_time[2])
				{
					*is_being_loaded = false;
					free(temp_interval_usage_time);
					return 0;
				}
				else if (temp_interval_usage_time[0] <= predicted_time && predicted_time <= temp_interval_usage_time[2])
				{
					*is_being_loaded = true;
					int temp = temp_interval_usage_time[1];
					free(temp_interval_usage_time);
					return temp - t;
				}				
				i = i->next;
			}
			break;
		}
		d = d->next;
	}
	*is_being_loaded = false;
	free(temp_interval_usage_time);
	return current_data_size/n->bandwidth;
}

/**
 * Compute the time it will take to reloead the file that would be evicted by scheduling a job on node n.
 * Used by LEA, LEO and LEM.
 **/
float time_to_reload_percentage_of_files_ended_at_certain_time(int predicted_time, struct Node* n, int current_data, float percentage_occupied)
{
	float size_file_ended = 0;

	struct Data* d = n->data->head;
	
	while (d != NULL)
	{
		struct Interval* i = d->intervals->head;
		if (d->unique_id != current_data && i != NULL)
		{			
			if (predicted_time >= d->intervals->tail->time)
			{
				size_file_ended += d->size;
			}
		}
		d = d->next;
	}	
	return (size_file_ended*percentage_occupied)/n->bandwidth;
}

/**
 * Get informations to print the cluster' usage.
 **/			
void get_nb_nodes_and_cores_loading_a_file(struct Node_List** head_node, int t, int* nodes_loading_a_file, int* cores_loading_a_file)
{
	#ifdef PRINT_CLUSTER_USAGE
	int i = 0;
	int j = 0;
	*nodes_loading_a_file = 0;
	*cores_loading_a_file = 0;
	
	for (i = 0; i < 3; i++)
	{
		struct Node* n = head_node[i]->head;
		while (n != NULL)
		{
			if (n->end_of_file_load > t)
			{
				*nodes_loading_a_file += 1;
			}
			
			for (j = 0; j < 20; j++)
			{
				if (n->cores[j]->end_of_file_load > t)
				{
					*cores_loading_a_file += 1;
				}
			}
			
			n = n->next;
		}
	}
	#endif
}
