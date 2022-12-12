/*
 * Copyright (C) - Anonymous for double blind submission.
 */
 
/**
 * Functions used only when using backfilling.
 **/

#include <main.h>

/**
 * Return whether or not job j can be backfilled on node n.
 * If it can, updates nb_cores_from_hole and nb_cores_from_outside
 * to indicate the core repartition of job j after being backfilled.
 **/
bool can_it_get_backfilled (struct Job* j, struct Node* n, int t, int* nb_cores_from_hole, int* nb_cores_from_outside)
{
	*nb_cores_from_hole = 0;
	*nb_cores_from_outside = 0;
	int k = 0;
	struct Core_in_a_hole* c = n->cores_in_a_hole->head;
		
	/* If you enter the if it means that there is hole and j could fit in it. */
	if (n->number_cores_in_a_hole != 0 && (j->cores <= n->number_cores_in_a_hole || n->cores[j->cores - 1 - n->number_cores_in_a_hole]->available_time <= t))
	{
		c = n->cores_in_a_hole->head;
		for (k = 0; k < n->number_cores_in_a_hole; k++)
		{
			if (t + j->walltime <= c->start_time_of_the_hole)
			{
				*nb_cores_from_hole += 1;
			}
			c = c->next;
			if (j->cores == *nb_cores_from_hole)
			{
				break;
			}
		}
					
		/* Some jobs can be backfilled with a few cores from a hole a few that are not in a hole. */			
		if (nb_cores_from_hole > 0)
		{
			*nb_cores_from_outside = j->cores - *nb_cores_from_hole;
		
			for (k = 0; k < *nb_cores_from_outside; k++)
			{
				if (n->cores[k]->available_time > t)
				{					
					return false;
				}
			}
			
			if (k == *nb_cores_from_outside)
			{
				return true;
			}
		}
	}
	return false;
}							

/** Update core available time when a job is backfilled. **/
void update_cores_for_backfilled_job(struct Job* j, int t, int nb_cores_from_hole, int nb_cores_from_outside)
{
	int i = 0;
	int k = 0;
		
	struct Core_in_a_hole* c = j->node_used->cores_in_a_hole->head;
	
	if (nb_cores_from_hole > j->cores || nb_cores_from_hole > j->node_used->number_cores_in_a_hole)
	{ 
		printf("Error cores: %d cores taken from hole, %d cores on the job, %d cores in a hole of the node.\n", nb_cores_from_hole, j->cores, j->node_used->number_cores_in_a_hole); 
		fflush(stdout); 
		exit(EXIT_FAILURE); 
	}
	
	for (k = 0; k < nb_cores_from_hole;)
	{
		if (t + j->walltime <= c->start_time_of_the_hole)
		{
			j->cores_used[i] = c->unique_id;
			i++;
			k++;
		}
	
		if (c->next == NULL && k != nb_cores_from_hole)
		{
			printf("Error: Next is null at time %d.\nJob %d uses %d cores.\ni = %d.\nk = %d.\nnb_cores_from_hole = %d.\nNode %d has %d cores in a hole.\n", t, j->unique_id, j->cores, i, k, nb_cores_from_hole, j->node_used->unique_id, j->node_used->number_cores_in_a_hole); fflush(stdout);
			exit(EXIT_FAILURE);
		}
		
		c = c->next;
	}
	
	for (k = 0; k < nb_cores_from_outside; k++)
	{
		#ifdef PRINT
		printf("Adding core from outside %d.\n", j->node_used->cores[k]->unique_id);
		#endif
				
		j->cores_used[i] = j->node_used->cores[k]->unique_id;
		j->node_used->cores[k]->available_time = t + j->walltime;
		i++;
	}		
	
	j->node_used->number_cores_in_a_hole -= nb_cores_from_hole;

	if (j->node_used->number_cores_in_a_hole < 0 || j->node_used->number_cores_in_a_hole > 19)
	{
		printf("Error nb core in hole %d on node %d.\n", j->node_used->number_cores_in_a_hole, j->node_used->unique_id); 
		fflush(stdout);
		exit(EXIT_FAILURE);
	}
	
	if (j->node_used->number_cores_in_a_hole == 0)
	{
		#ifdef PRINT
		printf("Deleting all the cores in the hole cause we use them all.\n");
		#endif
				
		free_cores_in_a_hole(&j->node_used->cores_in_a_hole->head);
	}
	else
	{
		for (i = 0; i < nb_cores_from_hole; i++)
		{				
			delete_core_in_hole_specific_core(j->node_used->cores_in_a_hole, j->cores_used[i]);
		}
	}	
}

/** Choose cores in order to minimize the amount of holes created whe nscheduling a job and multiple combination of cores are possible. **/
void fill_cores_minimize_holes (struct Job* j, bool backfill_activated, int backfill_mode, int t, int* nb_non_available_cores)
{
	int i = 0;
	int k = 0;
	sort_cores_by_unique_id_in_specific_node(j->node_used);
	
	for (k = 0; k < 20; k++)
	{
		if (j->node_used->cores[k]->available_time <= j->start_time)
		{
			j->cores_used[i] = j->node_used->cores[k]->unique_id;
			i++;
			if (j->node_used->cores[k]->available_time <= t)
			{
				*nb_non_available_cores += 1;
			}
			if (backfill_activated == true)
			{
				if (j->node_used->cores[k]->available_time <= t && j->start_time > t)
				{						
					j->node_used->number_cores_in_a_hole += 1;
					struct Core_in_a_hole* new = (struct Core_in_a_hole*) malloc(sizeof(struct Core_in_a_hole));
					new->unique_id = j->node_used->cores[k]->unique_id;
					new->start_time_of_the_hole = j->start_time;
					new->next = NULL;
						
					if (j->node_used->cores_in_a_hole == NULL)
					{
						initialize_cores_in_a_hole(j->node_used->cores_in_a_hole, new);
					}
					else
					{
						/* Optimized version to minimize holes. All our schedulers use backfill_mode set to this mode (2). */
						if (backfill_mode == 2)
						{
							insert_cores_in_a_hole_list_sorted_increasing_order(j->node_used->cores_in_a_hole, new);
						}
						else
						{
							insert_cores_in_a_hole_list_sorted_decreasing_order(j->node_used->cores_in_a_hole, new);
						}
					}
				}
			}
			j->node_used->cores[k]->available_time = j->start_time + j->walltime;
		}
		if (i == j->cores)
		{
			break;
		}
	}
	
	if (i != j->cores)
	{ 
		printf("Error fill_cores_minimize_holes. i = %d job %d j->cores = %d j->start time is %d\n", i, j->unique_id, j->cores, j->start_time); printf("Dans l'erreur\n"); print_cores_in_specific_node(j->node_used); 
		exit(EXIT_FAILURE);
	}
}

/** 
 * Only check if I can backfill a job. If I can I do.
 * There is an option for a locality backfill that only backfill if no data is evicted.
 */
bool only_check_conservative_backfill(struct Job* j, struct Node_List** head_node, int t, int backfill_mode, int* nb_non_available_cores_at_time_t)
{
	int nb_cores_from_hole = 0;
	int nb_cores_from_outside = 0;
		
	int i = 0;
	int min_time = -1;
	bool backfilled_job = false;
	
	for (i = 0; i <= 2; i++)
	{
		struct Node* n = head_node[i]->head;
		while (n != NULL)
		{
			#ifdef PRINT
			printf("Checking node %d.\n", n->unique_id);
			#endif
					
			if (n->number_cores_in_a_hole > 0 && j->cores <= n->number_cores_in_a_hole)
			{
				nb_cores_from_hole = 0;
				struct Core_in_a_hole* c = n->cores_in_a_hole->head;
				for (int k = 0; k < n->number_cores_in_a_hole; k++)
				{
					if (t + j->walltime <= c->start_time_of_the_hole)
					{
						nb_cores_from_hole += 1;
					}
						
					if (j->cores == nb_cores_from_hole)
					{
						backfilled_job = true;
						break;
					}
					c = c->next;
				}
			}
					
			if (backfilled_job == true)
			{
				min_time = t;
				j->node_used = n;
				i = 3;
				j->start_time = min_time;
				j->end_time = min_time + j->walltime;
				update_cores_for_backfilled_job(j, t, nb_cores_from_hole, nb_cores_from_outside);
				*nb_non_available_cores_at_time_t += j->cores;
					
				if (nb_cores_from_outside > 0)
				{
					sort_cores_by_available_time_in_specific_node(j->node_used);
				}
			
				if (j->node_used->unique_id == biggest_hole_unique_id)
				{
					get_new_biggest_hole(head_node);
				}						
				return true;
			}
			n = n->next;
		}
	}
	return false;
}

/** Check if a job can be backfilled while considering the computation of the score. **/
bool only_check_conservative_backfill_with_a_score(struct Job* j, struct Node_List** head_node, int t, int backfill_mode, int* nb_non_available_cores_at_time_t, int multiplier_file_to_load, int multiplier_file_evicted, int start_immediately_if_EAT_is_t)
{
	int nb_cores_from_hole = 0;
	int nb_cores_from_outside = 0;
	int i = 0;
	bool backfilled_job = false;
	bool can_backfill_on_this_node = false;
	int min_score = -1;
	float time_to_load_file = 0;
	bool is_being_loaded = false;
	float time_to_reload_evicted_files = 0;
	int score = 0;
	int choosen_time_to_load_file = 0;
		
	if (start_immediately_if_EAT_is_t == 1)
	{
		multiplier_file_to_load = 1;
		multiplier_file_evicted = 0;
	}

	for (i = 0; i <= 2; i++)
	{
		struct Node* n = head_node[i]->head;
		while (n != NULL)
		{
			can_backfill_on_this_node = false;
			
			#ifdef PRINT
			printf("Checking node %d.\n", n->unique_id);
			#endif
			
			if (n->number_cores_in_a_hole > 0 && j->cores <= n->number_cores_in_a_hole)
			{
				nb_cores_from_hole = 0;
				struct Core_in_a_hole* c = n->cores_in_a_hole->head;
				for (int k = 0; k < n->number_cores_in_a_hole; k++)
				{
					if (t + j->walltime <= c->start_time_of_the_hole)
					{
						nb_cores_from_hole += 1;
					}
						
					if (j->cores == nb_cores_from_hole)
					{
						can_backfill_on_this_node = true;
						break;
					}
					c = c->next;
				}
			}
					
			if (can_backfill_on_this_node == true)
			{
				backfilled_job = true;
				if (j->data == 0)
				{
					time_to_load_file = 0;
				}
				else
				{
					time_to_load_file = is_my_file_on_node_at_certain_time_and_transfer_time(t, n, t, j->data, j->data_size, &is_being_loaded);
				}
																	
				if (min_score == -1 || multiplier_file_to_load*time_to_load_file < min_score)
				{
					if (multiplier_file_evicted == 0)
					{
						time_to_reload_evicted_files = 0;
					}
					else
					{
						time_to_reload_evicted_files = time_to_reload_percentage_of_files_ended_at_certain_time(t, n, j->data, (float) j->cores/20);
					}
																
					score = multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files;
					
					if (min_score == -1 || min_score > score)
					{
						min_score = score;
						j->node_used = n;
						choosen_time_to_load_file = time_to_load_file;
																				
						if (min_score == 0)
						{			
							i = 3;
							break;
						}
					}
				}
			}
			n = n->next;
		}
	}
	
	if(backfilled_job == true)
	{
		j->start_time = t;
		j->end_time = t + j->walltime;
		j->transfer_time = choosen_time_to_load_file;
		update_cores_for_backfilled_job(j, t, j->cores, 0);
		*nb_non_available_cores_at_time_t += j->cores;
					
		if (nb_cores_from_outside > 0)
		{
			sort_cores_by_available_time_in_specific_node(j->node_used);
		}
			
		if (j->node_used->unique_id == biggest_hole_unique_id)
		{
			get_new_biggest_hole(head_node);
		}		
		return true;
	}
	return false;
}

/** 
 * Get the size and id of the node with the largest hole in term of number of cores.
 * Allow to quickly check if a node can be used or not for a backfill.
 **/
void get_new_biggest_hole(struct Node_List** head_node)
{
	biggest_hole = -1;
	for (int i = 0; i < 3; i++)
	{
		struct Node* n = head_node[i]->head;
		while (n != NULL)
		{
			if (n->number_cores_in_a_hole > biggest_hole)
			{
				biggest_hole = n->number_cores_in_a_hole;
				biggest_hole_unique_id = n->unique_id;
			}
			n = n->next;
		}
	}
}
