#include <main.h>

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
	
	#ifdef PRINT
	printf("%d nodes of size 128, %d of size 256 and %d of size 1024.\n", nb_node[0], nb_node[1], nb_node[2]);
	#endif
	
	free(temp);

	struct Job* j = j2;
	while (j != NULL)
	{
		/* Insert in scheduled_job_list */
		//~ copy_job_and_insert_tail_job_list(scheduled_job_list, j);
		//~ insert_tail_job_list(scheduled_job_list, j);
		
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
		
		/* Add in list of starting times. */		
		insert_next_time_in_sorted_list(start_times, j->start_time);
		
		#ifdef PRINT
		print_decision_in_scheduler(j);
		#endif
		
		j = j->next;
	}
	
	/* Remove from job list to start and put it in scheduled job list. */
	for (i = 0; i < nb_job_to_delete; i++)
	{
		j = job_list_to_start_from_history->head;
		copy_delete_insert_job_list(job_list_to_start_from_history, scheduled_job_list, j);
	}

	free(nb_node);
}

/* FCFS */
void fcfs_scheduler(struct Job* head_job, struct Node_List** head_node, int t, bool use_bigger_nodes)
{
	#ifdef PRINT
	printf("Start fcfs scheduler. Use bigger nodes: %d.\n", use_bigger_nodes);
	#endif
		
	int nb_non_available_cores = get_nb_non_available_cores(node_list, t);

	struct Job* j = head_job;
	while (j != NULL)
	{
		if (nb_non_available_cores < nb_cores)
		{
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);
			#endif
			
			nb_non_available_cores = schedule_job_on_earliest_available_cores(j, head_node, t, nb_non_available_cores, use_bigger_nodes);
			
			/* test reduced complexity */
			if (j->start_time >= t)
			{
				insert_next_time_in_sorted_list(start_times, j->start_time);
			}
			
			j = j->next;
		}
		else
		{
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);
			#endif
			
			/* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			while (j != NULL)
			{
				j->start_time = -1;
				j = j->next;
			}
			
			break;
		}
	}
}

//~ void fcfs_with_a_score_mixed_strategy_scheduler(struct Job* head_job, struct Node_List** head_node, int t, int multiplier_file_to_load, int multiplier_file_evicted, int multiplier_nb_copy)
//~ {
	//~ #ifdef PRINT
	//~ printf("Start fcfs mixed scheduler.\n");
	//~ #endif
		
	//~ int nb_non_available_cores = get_nb_non_available_cores(node_list, t);

	//~ struct Job* j = head_job;
	//~ while (j != NULL)
	//~ {
		//~ if (nb_non_available_cores < nb_cores)
		//~ {
			//~ #ifdef PRINT
			//~ printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);
			//~ #endif
			
			//~ nb_non_available_cores = schedule_job_on_earliest_available_cores(j, head_node, t, nb_non_available_cores, use_bigger_nodes);
			
			//~ insert_next_time_in_sorted_list(start_times, j->start_time);
			
			//~ j = j->next;
		//~ }
		//~ else
		//~ {
			//~ #ifdef PRINT
			//~ printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);
			//~ #endif
			
			//~ /* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			//~ while (j != NULL)
			//~ {
				//~ j->start_time = -1;
				//~ j = j->next;
			//~ }
			
			//~ break;
		//~ }
	//~ }
//~ }

/**
 * Schedule normalement.
 * Si je vois que un trou va se créer avec EAT == t, je le marque dans la liste des trous dans la struct node.
 * Dans la struct node je met la liste des cores ainsi que leurs nombre qui forment le trou au temps t pour aller plus vite.
 * Attention il faut reset cette liste de trou au moment du reset du reschedule.
 * Je ne check que les trou au temps t car sinon ca change rien avec le reschedule normalement.
 * Pour les jobs suivant je check node par node le EAT et les trou de la node. Si je rentre dans le trou je me schedule la pour fcfs.
 * nb_non_available_cores ne vaut que quand c'est t qui est recouvert, pas plus loin.
 **/
void fcfs_conservativebf_scheduler(struct Job* head_job, struct Node_List** head_node, int t, int backfill_mode)
{
	//~ int nb_cores_rescheduled = 0; /* 486*20 = 9720 */
	int nb_non_available_cores = get_nb_non_available_cores(node_list, t);
	int nb_non_available_cores_at_time_t = global_nb_non_available_cores_at_time_t;
	//~ printf("biggest hole is %d.\n", biggest_hole);
	//~ biggest_hole = 0;
	
	#ifdef PRINT
	printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);
	printf("There are %d/%d available cores at time t.\n", nb_cores - nb_non_available_cores_at_time_t, nb_cores);
	printf("Start FCFS CONSERVATIVE BF at time %d. backfill mode is %d.\n", t, backfill_mode);
	#endif

	struct Job* j = head_job;
	while (j != NULL)
	{
		if (nb_non_available_cores < nb_cores)
		{
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);
			printf("There are %d/%d available cores at time t.\n", nb_cores - nb_non_available_cores_at_time_t, nb_cores);
			printf("Schedule and backfill.\n");
			#endif
			
			schedule_job_on_earliest_available_cores_with_conservative_backfill(j, head_node, t, backfill_mode, &nb_non_available_cores, &nb_non_available_cores_at_time_t);
			
			//~ nb_cores_rescheduled += j->cores;
			
			//~ if (j->start_time < t)
			//~ {
				//~ printf("Error: j->start_time < t\n"); fflush(stdout);
				//~ exit(1);
			//~ }
			
			//~ insert_next_time_in_sorted_list(start_times, j->start_time);
			
			/* Test complexité réduite */
			if (j->start_time >= t)
			{
				insert_next_time_in_sorted_list(start_times, j->start_time);
			}
			
			j = j->next;
		}
		else if (nb_non_available_cores_at_time_t < nb_cores) // Ajouter un && nb_non_available_cores_at_time_t > 20 car ça peut tout bloquer pour rien ?
		{
			/* Only check backfill */
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);
			printf("There are %d/%d available cores at time t.\n", nb_cores - nb_non_available_cores_at_time_t, nb_cores);
			printf("Only check backfill.\n");
			printf("Biggest hole is %d on node %d.\n", biggest_hole, biggest_hole_unique_id);
			#endif
						
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
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);
			printf("There are %d/%d available cores at time t.\n", nb_cores - nb_non_available_cores_at_time_t, nb_cores);
			printf("Break scheduling.\n");
			#endif
			
			/* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
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

void fcfs_with_a_score_conservativebf_scheduler(struct Job* head_job, struct Node_List** head_node, int t, int multiplier_file_to_load, int multiplier_file_evicted, int adaptative_multiplier, int start_immediately_if_EAT_is_t, int backfill_mode, int mixed_strategy)
{
	//~ int nb_cores_rescheduled = 0; /* 486*20 = 9720 */
	int nb_non_available_cores = get_nb_non_available_cores(node_list, t);
	int nb_non_available_cores_at_time_t = global_nb_non_available_cores_at_time_t;
	//~ biggest_hole = 0;
	
	#ifdef PRINT
	printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);
	printf("There are %d/%d available cores at time t.\n", nb_cores - nb_non_available_cores_at_time_t, nb_cores);
	printf("Start SCORE CONSERVATIVE BF at time %d. backfill mode is %d.\n", t, backfill_mode);
	#endif
	
	/* Get intervals of data. */ 
	get_current_intervals(head_node, t);
	
	/* For mixed */
	int temp_running_nodes = running_nodes;
	
	struct Job* j = head_job;
	
	while (j != NULL)
	{		
		//~ if (nb_non_available_cores < nb_cores) /* Vrai version */
		if (j->cores <= nb_cores - nb_non_available_cores)
		{
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);
			printf("There are %d/%d available cores at time t.\n", nb_cores - nb_non_available_cores_at_time_t, nb_cores);
			printf("Schedule and backfill.\n");
			#endif
			
			if (mixed_strategy == 1)
			{
				if ((temp_running_nodes*100)/486 < busy_cluster_threshold)
				{
					schedule_job_fcfs_score_with_conservative_backfill(j, head_node, t, 1, 0, adaptative_multiplier, start_immediately_if_EAT_is_t, backfill_mode, &nb_non_available_cores, &nb_non_available_cores_at_time_t, mixed_strategy, &temp_running_nodes);
				}
				else
				{
					schedule_job_fcfs_score_with_conservative_backfill(j, head_node, t, multiplier_file_to_load, multiplier_file_evicted, adaptative_multiplier, start_immediately_if_EAT_is_t, backfill_mode, &nb_non_available_cores, &nb_non_available_cores_at_time_t, mixed_strategy, &temp_running_nodes);
				}
			}
			else
			{
				schedule_job_fcfs_score_with_conservative_backfill(j, head_node, t, multiplier_file_to_load, multiplier_file_evicted, adaptative_multiplier, start_immediately_if_EAT_is_t, backfill_mode, &nb_non_available_cores, &nb_non_available_cores_at_time_t, mixed_strategy, &temp_running_nodes);
			}
						
			//~ if (j->start_time < t)
			//~ {
				//~ printf("Error: j->start_time < t\n");
				//~ exit(EXIT_FAILURE);
			//~ }
			
			/* Test complexité réduite */
			if (j->start_time >= t)
			{
				insert_next_time_in_sorted_list(start_times, j->start_time);
			}
			
			j = j->next;
		}
		else if (nb_non_available_cores_at_time_t < nb_cores) // Ajouter un && nb_non_available_cores_at_time_t > 20 car ça peut tout bloquer pour rien ?
		{
			/* Only check backfill */
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);
			printf("There are %d/%d available cores at time t.\n", nb_cores - nb_non_available_cores_at_time_t, nb_cores);
			printf("Only check backfill for job %d.\n", j->unique_id);
			printf("Biggest hole is %d on node %d.\n", biggest_hole, biggest_hole_unique_id);
			#endif
				
			//~ printf("%d running nodes.\n", temp_running_nodes);

			if (mixed_strategy == 1)
			{
				if ((temp_running_nodes*100)/486 < busy_cluster_threshold)
				{
					//~ printf("Call hole not busy.\n");
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
					//~ printf("Call hole busy.\n");
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
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);
			printf("There are %d/%d available cores at time t.\n", nb_cores - nb_non_available_cores_at_time_t, nb_cores);
			printf("Break scheduling.\n");
			#endif
			
			/* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
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

void fcfs_easybf_scheduler(struct Job* head_job, struct Node_List** head_node, int t, bool use_bigger_nodes)
{
	#ifdef PRINT
	printf("Start fcfs easybf scheduler. Use bigger nodes: %d.\n", use_bigger_nodes);
	#endif
	
	int nb_running_cores = running_cores;
	
	#ifdef PRINT
	printf("Nb of running cores before j1: %d.\n", nb_running_cores);
	#endif
	
	/* First schedule J_1. */
	struct Job* j1 = head_job;
	nb_running_cores = schedule_job_on_earliest_available_cores_return_running_cores(j1, head_node, t, nb_running_cores, use_bigger_nodes);
	insert_next_time_in_sorted_list(start_times, j1->start_time);
	//~ printf("Inserted time %d at t = %d for j1.\n", j1->start_time, t);
	#ifdef PRINT
	printf("Nb of running cores after j1: %d.\n", nb_running_cores);
	#endif
	
	bool result = false;
	struct Job* j = j1->next;

	while (j != NULL)
	{
		if (nb_running_cores < nb_cores)
		{
			#ifdef PRINT
			printf("There are %d/%d running cores.\n", nb_running_cores, nb_cores);
			#endif
			
			result = false;
			
			nb_running_cores = try_to_start_job_immediatly_without_delaying_j1(j, j1, head_node, nb_running_cores, &result, use_bigger_nodes, t);
			
			if (result == true)
			{
				insert_next_time_in_sorted_list(start_times, j->start_time);
			}
			else
			{
				j->start_time = -1;
			}
			
			#ifdef PRINT
			printf("Nb of running cores after starting (or not: %d) Job %d: %d.\n", result, j->unique_id, nb_running_cores);
			#endif
			
			j = j->next;
		}
		else
		{
			#ifdef PRINT
			printf("There are %d/%d running cores. Break.\n", nb_running_cores, nb_cores);
			#endif
			
			/* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			while (j != NULL)
			{
				j->start_time = -1;
				j = j->next;
			}

			break;
		}
	}
}

void fcfs_with_a_score_easybf_scheduler(struct Job* head_job, struct Node_List** head_node, int t, int multiplier_file_to_load, int multiplier_file_evicted, int multiplier_nb_copy, int adaptative_multiplier, int penalty_on_job_sizes, int start_immediately_if_EAT_is_t)
{
	#ifdef PRINT
	printf("Start fcfs with a score easybf scheduler with parameters int t: %d, int multiplier_file_to_load: %d, int multiplier_file_evicted: %d, int multiplier_nb_copy: %d, int adaptative_multiplier: %d, int penalty_on_job_sizes: %d, int start_immediately_if_EAT_is_t: %d.\n", t, multiplier_file_to_load, multiplier_file_evicted, multiplier_nb_copy, adaptative_multiplier, penalty_on_job_sizes, start_immediately_if_EAT_is_t);
	#endif
	
	/* Notre condition d'arrêt pour le EASY backfilling */
	int nb_running_cores = running_cores;
	
	/* Valeurs de base de FCFS with a score */
	int i = 0;
	int min_score = -1;
	int earliest_available_time = 0;
	// int // first_node_size_to_choose_from = 0;
	// int // last_node_size_to_choose_from = 0;
	float time_to_load_file = 0;
	bool is_being_loaded = false;
	float time_to_reload_evicted_files = 0;
	int nb_copy_file_to_load = 0;
	int time_or_data_already_checked = 0;
	int score = 0;
	int min_time = 0;
	int choosen_time_to_load_file = 0;
	bool found = false;
	bool could_schedule = false;
	bool ok_on_this_node = false;
	bool need_to_break = false;
	int k = 0;
	if (adaptative_multiplier == 1) 
	{
		if (multiplier_file_to_load != 0)
		{
			multiplier_file_to_load = running_nodes;
		}
	}
	else if (adaptative_multiplier == 2)
	{
		if (multiplier_file_to_load != 0)
		{
			multiplier_file_to_load = (int) ceil ((float) nb_job_to_schedule/486);
		}
	}
	else if (adaptative_multiplier == 3)
	{
		if (running_nodes < 454)
		{
			multiplier_file_to_load = 1;
			multiplier_file_evicted = 0;
			multiplier_nb_copy = 0;
		}
		else
		{
			if (multiplier_file_to_load != 0)
			{
				multiplier_file_to_load = running_nodes;
			}
		}
	}
	else if (adaptative_multiplier == 4)
	{
		if (486*20 - running_cores >= nb_cores_to_schedule)
		{
			multiplier_file_to_load = 1;
			multiplier_file_evicted = 0;
			multiplier_nb_copy = 0;
		}
		else
		{
			if (multiplier_file_to_load != 0)
			{
				multiplier_file_to_load = running_nodes;
			}
		}
	}
	
	/* temp multiplier pour le cas avec if EAT is t start now */
	int temp_multiplier_file_to_load = multiplier_file_to_load;
	int temp_multiplier_file_evicted = multiplier_file_evicted;
	int temp_multiplier_nb_copy = multiplier_nb_copy;
	
	/* Get intervals of data. */ 
	get_current_intervals(head_node, t);
	
	#ifdef PRINT
	print_data_intervals(head_node, t);
	#endif
	
	#ifdef PRINT_SCORES_DATA
	FILE* f_fcfs_score = fopen("outputs/Scores_data.txt", "a");
	#endif
	
	struct Time_or_Data_Already_Checked_Nb_of_Copy_List* time_or_data_already_checked_nb_of_copy_list = (struct Time_or_Data_Already_Checked_Nb_of_Copy_List*) malloc(sizeof(struct Time_or_Data_Already_Checked_Nb_of_Copy_List));
	time_or_data_already_checked_nb_of_copy_list->head = NULL;
	
	#ifdef PRINT
	printf("Nb of running cores before j1: %d.\n", nb_running_cores);
	#endif
	
	/* First schedule J_1. */
	struct Job* j1 = head_job;
	
	/* pas de get intervals dedans. */
	#ifdef PRINT
	printf("Scheduling j1.\n");
	#endif
	
	nb_running_cores = schedule_job_fcfs_score_return_running_cores(j1, head_node, t, nb_running_cores, multiplier_file_to_load, multiplier_file_evicted, multiplier_nb_copy, adaptative_multiplier, penalty_on_job_sizes, start_immediately_if_EAT_is_t);
	//~ insert_next_time_in_sorted_list(start_times, j1->start_time); /* je le fais dans la fonction si dessus. */
	
	#ifdef PRINT
	printf("Nb of running cores after j1: %d.\n", nb_running_cores);
	#endif
	
	struct Job* j = j1->next;

	while (j != NULL)
	{
		if (nb_running_cores < nb_cores)
		{
			#ifdef PRINT
			printf("There are %d/%d running cores.\n", nb_running_cores, nb_cores);
			#endif
			
			//~ result = false;
			
			//~ void fcfs_with_a_score_scheduler_without_delaying_j1(struct Job* j, struct Job* j1, struct Node_List** head_node, int nb_running_cores, bool* result, int t, int multiplier_file_to_load, int multiplier_file_evicted, int multiplier_nb_copy, int adaptative_multiplier, int penalty_on_job_sizes, int start_immediately_if_EAT_is_t)
//~ {	
	//~ int nb_non_available_cores = get_nb_non_available_cores(node_list, t);		

	/* 1. Loop on available jobs. */
	//~ struct Job* j = head_job;
	//~ while (j != NULL)
	//~ {		
		//~ if (nb_non_available_cores < nb_cores)
		//~ {
			#ifdef PRINT
			printf("\nNeed to schedule job %d using file %d. T = %d\n", j->unique_id, j->data, t); fflush(stdout);
			#endif
			
			/* cas if EAT is t reset multipliers */
			if (start_immediately_if_EAT_is_t == 1)
			{
				multiplier_file_to_load = temp_multiplier_file_to_load;
				multiplier_file_evicted = temp_multiplier_file_evicted;
				multiplier_nb_copy = temp_multiplier_nb_copy;
			}
			
			/* 2. Choose a node. */		
			/* Reset some values. */					
			min_score = -1;
			earliest_available_time = 0;
			// first_node_size_to_choose_from = 0;
			// last_node_size_to_choose_from = 0;
			is_being_loaded = false;
			time_to_reload_evicted_files = 0;
			nb_copy_file_to_load = 0;
			could_schedule = false;
			
			/* In which node size I can pick. */
			// get_node_size_to_choose_from(j->index_node_list, &first_node_size_to_choose_from, &last_node_size_to_choose_from);
			//~ if (j->index_node_list == 0)
			//~ {
				//~ // first_node_size_to_choose_from = 0;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 1)
			//~ {
				//~ first_node_size_to_choose_from = 1;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 2)
			//~ {
				//~ first_node_size_to_choose_from = 2;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else
			//~ {
				//~ printf("Error index value in schedule_job_on_earliest_available_cores.\n");  fflush(stdout);
				//~ exit(EXIT_FAILURE);
			//~ }
						
			/* --- Reduced complexity nb of copy --- */
			if (multiplier_nb_copy != 0)
			{
				time_or_data_already_checked = was_time_or_data_already_checked_for_nb_copy(j->data, time_or_data_already_checked_nb_of_copy_list);
			}

			for (i = first_node_size_to_choose_from; i <= last_node_size_to_choose_from; i++)
			{
				struct Node* n = head_node[i]->head;
				while (n != NULL)
				{
					#ifdef PRINT
					printf("On node %d?\n", n->unique_id); fflush(stdout);
					#endif
										
					/* 2.1. A = Get the earliest available time from the number of cores required by the job and add it to the score. */
					earliest_available_time = n->cores[j->cores - 1]->available_time; /* -1 because tab start at 0 */
					if (earliest_available_time <= t)				
					{
						if (start_immediately_if_EAT_is_t == 1 && earliest_available_time == t) /* Ou dans une fenêtre ? */
						{
							multiplier_file_to_load = 1;
							multiplier_file_evicted = 0;
							multiplier_nb_copy = 0;
						}
						
						ok_on_this_node = true;			
						earliest_available_time = t;
						
						/* But is it the same node as j1 ? If yes I need to be careful. */
						if (n->unique_id == j1->node_used->unique_id)
						{
							if (earliest_available_time + j->walltime > j1->start_time) /* It will finish later so I need to check if it's the same cores. If yes I can't do it. */
							{
								need_to_break = false;
								for (i = 0; i < j->cores; i++)
								{
									for (k = 0; k < j1->cores; k++)
									{
										if (n->cores[i]->unique_id == j1->cores_used[k])
										{
											/* Need to exit. */
											ok_on_this_node = false;
											need_to_break = true;
											break;
										}
									}
									if (need_to_break == true)
									{
										break;
									}
								}
							}
						}
						
						if (ok_on_this_node == true)
						{
							#ifdef PRINT
							printf("Node %d is ok.\n", n->unique_id);
							#endif
							
							could_schedule = true;

							#ifdef PRINT
							printf("A: EAT is: %d.\n", earliest_available_time); fflush(stdout);
							#endif
					
							if (min_score == -1 || earliest_available_time < min_score)
							{								
								/* 2.2. B = Compute the time to load all data. For this look at the data that will be available at the earliest available time of the node. */
								if (j->data == 0)
								{
									time_to_load_file = 0;
								}
								else
								{
									time_to_load_file = is_my_file_on_node_at_certain_time_and_transfer_time(earliest_available_time, n, t, j->data, j->data_size, &is_being_loaded); /* Use the intervals in each data to get this info. */
								}
						
								#ifdef PRINT
								printf("B: Time to load file: %f. Is being loaded? %d.\n", time_to_load_file, is_being_loaded); fflush(stdout);
								#endif
											
								if (min_score == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file < min_score)
								{
									/* 2.5. Get the amount of files that will be lost because of this load by computing the amount of data that end at the earliest time only on the supposely choosen cores, excluding current file of course. */
									if (multiplier_file_evicted == 0)
									{
										time_to_reload_evicted_files = 0;
									}
									else
									{
										time_to_reload_evicted_files = time_to_reload_percentage_of_files_ended_at_certain_time(earliest_available_time, n, j->data, (float) j->cores/20);
									}
							
									#ifdef PRINT
									printf("C: Time to reload evicted files %f.\n", time_to_reload_evicted_files); fflush(stdout);
									#endif
									
									if (min_score == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files < min_score)
									{
										/* 2.5bis Get number of copy of the file we want to load on other nodes (if you need to load a file that is) at the time that is predicted to be used. So if a file is already loaded on a lot of node, you have a penalty if you want to load it on a new node. */
										if (time_to_load_file != 0 && is_being_loaded == false && multiplier_nb_copy != 0)
										{
											/* --- Reduced complexity nb of copy --- */
											if (time_or_data_already_checked == -1)
											{
												#ifdef PRINT
												printf("Need to compute nb of copy it was never done.\n");
												#endif
												nb_copy_file_to_load = get_nb_valid_copy_of_a_file(t, head_node, j->data);
												create_and_insert_head_time_or_data_already_checked_nb_of_copy_list(time_or_data_already_checked_nb_of_copy_list, j->data, nb_copy_file_to_load);
												time_or_data_already_checked = nb_copy_file_to_load;
												#ifdef PRINT
												printf("Compute nb of copy done, it's %d.\n", nb_copy_file_to_load);
												#endif
											}
											else
											{
												nb_copy_file_to_load = time_or_data_already_checked;
												#ifdef PRINT
												printf("Already done for job %d at time %d so nb of copy is %d.\n", j->unique_id, t, nb_copy_file_to_load);
												#endif
											}
										}
										else
										{
											nb_copy_file_to_load = 0;
										}
								
										#ifdef PRINT
										printf("Nb of copy for data %d at time %d on node %d is %d.\n", j->data, earliest_available_time, n->unique_id, nb_copy_file_to_load); fflush(stdout);
										#endif
								
										/* Compute node's score. */
										score = earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files + nb_copy_file_to_load*time_to_load_file*multiplier_nb_copy;
																		
										#ifdef PRINT	
										printf("Score for job %d is %d (EAT: %d + TL %d*%f + TRL %d*%f + NCP %d*%d*%f) with node %d.\n", j->unique_id, score, earliest_available_time, multiplier_file_to_load, time_to_load_file, multiplier_file_evicted, time_to_reload_evicted_files, nb_copy_file_to_load, multiplier_nb_copy, time_to_load_file, n->unique_id); fflush(stdout);
										#endif
																					
										/* 2.6. Get minimum score/ */
										/* TODO : simpliefier la complexité: si EAT est t et TL et TLE == 0 alors break */
										if (min_score == -1 || min_score > score)
										{
											min_time = earliest_available_time;
											min_score = score;
											j->node_used = n;
											choosen_time_to_load_file = time_to_load_file;
											if (min_time == t && min_score == t) /* Temps de début est t et pas de temps de chargements du tout. Pour réduire la complexité un peu. */
											{
												i = last_node_size_to_choose_from + 1;
												break;
											}
										}
									}
								}
							}
						}
					}
					
					#ifdef PRINT_SCORES_DATA
					fprintf(f_fcfs_score, "Node: %d EAT: %d C: %f CxX: %f Score: %f\n", n->unique_id, earliest_available_time, time_to_reload_evicted_files, time_to_reload_evicted_files*multiplier_file_evicted, earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files);
					#endif
					
					n = n->next;
				}
			}
			
			if (could_schedule == true)
			{
				#ifdef PRINT
				printf("Could schedule.\n");
				#endif
				
				/* Get start time and update available times of the cores. */
				j->transfer_time = choosen_time_to_load_file;
				j->start_time = min_time;
				j->end_time = min_time + j->walltime;
				nb_running_cores += j->cores;
				
				for (int k = 0; k < j->cores; k++)
				{
					j->cores_used[k] = j->node_used->cores[k]->unique_id;
					j->node_used->cores[k]->available_time = min_time + j->walltime;
				}

				/* Need to add here intervals for current scheduling. */
				found = false;
				
				#ifdef DATA_PERSISTENCE
				struct Data* d = j->node_used->temp_data->head;
				#else
				struct Data* d = j->node_used->data->head;
				#endif
				
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
					#ifdef PRINT
					printf("Need to create a data and intervals for the node %d data %d.\n", j->node_used->unique_id, j->data); fflush(stdout);
					#endif
					
					/* Create a class Data for this node. */
					struct Data* new = (struct Data*) malloc(sizeof(struct Data));
					new->next = NULL;
					new->unique_id = j->data;
					new->start_time = -1;
					new->end_time = -1;
					
					#ifndef DATA_PERSISTENCE
					new->nb_task_using_it = 0;
					#endif
					
					new->intervals = (struct Interval_List*) malloc(sizeof(struct Interval_List));
					new->intervals->head = NULL;
					new->intervals->tail = NULL;
					create_and_insert_tail_interval_list(new->intervals, j->start_time);
					create_and_insert_tail_interval_list(new->intervals, j->start_time + j->transfer_time);
					create_and_insert_tail_interval_list(new->intervals, j->end_time);
					new->size = j->data_size;
					
					#ifdef DATA_PERSISTENCE
					insert_tail_data_list(j->node_used->temp_data, new);
					#else
					insert_tail_data_list(j->node_used->data, new);
					#endif
				}
				
				/* Need to sort cores after each schedule of a job. */
				sort_cores_by_available_time_in_specific_node(j->node_used);
			
				#ifdef PRINT
				print_decision_in_scheduler(j);
				#endif
				
				/* Insert in start times. */
				insert_next_time_in_sorted_list(start_times, j->start_time);
								
				/* --- Reduced complexity nb of copy --- */
				/* Free time already checked. */
			}
			else
			{
				#ifdef PRINT
				printf("Could not start the job %d.\n", j->unique_id);
				#endif

				j->start_time = -1;
			}
			
			//~ j = j->next;
		//~ }				
		//~ else
		//~ {
			//~ #ifdef PRINT
			//~ printf("No more available cores.\n"); fflush(stdout);
			//~ #endif
			
			//~ /* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			//~ while (j != NULL)
			//~ {
				//~ j->start_time = -1;
				//~ j = j->next;
			//~ }
			
			//~ break;
		//~ }
	//~ }
	

			
		
		
		//~ if (result == true)
			//~ {
				//~ insert_next_time_in_sorted_list(start_times, j->start_time);
			//~ }
			//~ else
			//~ {
				//~ j->start_time = -1;
			//~ }
			
			#ifdef PRINT
			printf("Nb of running cores after starting job %d: %d.\n", j->unique_id, nb_running_cores);
			#endif
				
			if (multiplier_nb_copy != 0 && j->start_time == t)
			{
				increment_time_or_data_nb_of_copy_specific_time_or_data(time_or_data_already_checked_nb_of_copy_list, j->data);
			}
			
			j = j->next;
		}
		else
		{
			#ifdef PRINT
			printf("There are %d/%d running cores. Break.\n", nb_running_cores, nb_cores);
			#endif
				
			/* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			while (j != NULL)
			{
				j->start_time = -1;
				j = j->next;
			}
				
			break;
		}
	}
	
	#ifdef PRINT_SCORES_DATA
	fclose(f_fcfs_score);
	#endif

	if (multiplier_nb_copy != 0)
	{
		free_time_or_data_already_checked_nb_of_copy_linked_list(&time_or_data_already_checked_nb_of_copy_list->head);
	}
}

void fcfs_with_a_score_scheduler(struct Job* head_job, struct Node_List** head_node, int t, int multiplier_file_to_load, int multiplier_file_evicted, int multiplier_nb_copy, int adaptative_multiplier, int penalty_on_job_sizes, int start_immediately_if_EAT_is_t, int mixed_strategy)
{
	#ifdef PRINT
	printf("\nFcfs_score\n");
	#endif
		
	int nb_non_available_cores = get_nb_non_available_cores(node_list, t);		
	int i = 0;
	int min_score = -1;
	int earliest_available_time = 0;
	// int // first_node_size_to_choose_from = 0;
	// int // last_node_size_to_choose_from = 0;
	float time_to_load_file = 0;
	bool is_being_loaded = false;
	float time_to_reload_evicted_files = 0;
	int nb_copy_file_to_load = 0;
	//~ int time_or_data_already_checked = 0;
	int score = 0;
	int min_time = 0;
	int choosen_time_to_load_file = 0;
	bool found = false;
	//~ float multiplier_file_to_load_increment = 0;
	
	//~ /** 1 = gives the number of running nodes as a multiplier.
	 //~ *  2 = gives the number of jobs to schedule divided by the total number of nodes as a multiplier.
	 //~ *  3 = gives the number of running nodes as a multiplier but put only 1 if the number of runing nodes is inferior to 75%.
	 //~ *  4 = gives the number of running nodes as a multiplier but put only 1 if the number of runing nodes is inferior to 75% and the queue of jobs to schedule is too important.
	 //~ **/
	//~ if (adaptative_multiplier == 1) 
	//~ {
		//~ if (multiplier_file_to_load != 0)
		//~ {
			//~ multiplier_file_to_load = running_nodes;
		//~ }
	//~ }
	//~ else if (adaptative_multiplier == 2)
	//~ {
		//~ if (multiplier_file_to_load != 0)
		//~ {
			//~ multiplier_file_to_load = (int) ceil ((float) nb_job_to_schedule/486);
		//~ }
	//~ }
	//~ else if (adaptative_multiplier == 3)
	//~ {
		//~ if (running_nodes < 454)
		//~ {
			//~ multiplier_file_to_load = 1;
			//~ multiplier_file_evicted = 0;
			//~ multiplier_nb_copy = 0;
		//~ }
		//~ else
		//~ {
			//~ if (multiplier_file_to_load != 0)
			//~ {
				//~ multiplier_file_to_load = running_nodes;
			//~ }
		//~ }
	//~ }
	//~ else if (adaptative_multiplier == 4)
	//~ {
		//~ if (486*20 - running_cores >= nb_cores_to_schedule)
		//~ {
			//~ multiplier_file_to_load = 1;
			//~ multiplier_file_evicted = 0;
			//~ multiplier_nb_copy = 0;
		//~ }
		//~ else
		//~ {
			//~ if (multiplier_file_to_load != 0)
			//~ {
				//~ multiplier_file_to_load = running_nodes;
			//~ }
		//~ }
	//~ }
			
	/* temp multiplier pour le cas avec if EAT is t start now */
	int temp_multiplier_file_to_load = multiplier_file_to_load;
	int temp_multiplier_file_evicted = multiplier_file_evicted;
	int temp_multiplier_nb_copy = multiplier_nb_copy;

	//~ if (mixed_strategy == 1)
	//~ {
		int temp_running_nodes = running_nodes;
	//~ }
	
	/* Get intervals of data. */ 
	get_current_intervals(head_node, t);
	
	#ifdef PRINT
	printf("AFTER.\n");
	print_data_intervals(head_node, t);
	#endif
	
	#ifdef PRINT_SCORES_DATA
	FILE* f_fcfs_score = fopen("outputs/Scores_data.txt", "a");
	#endif
	
	/* --- Reduced complexity nb of copy --- */	
	//~ if (multiplier_nb_copy != 0)
	//~ {
		//~ struct Time_or_Data_Already_Checked_Nb_of_Copy_List* time_or_data_already_checked_nb_of_copy_list = (struct Time_or_Data_Already_Checked_Nb_of_Copy_List*) malloc(sizeof(struct Time_or_Data_Already_Checked_Nb_of_Copy_List));
		//~ time_or_data_already_checked_nb_of_copy_list->head = NULL;
	//~ }

			/* Cas non dynamique */
			if (mixed_strategy == 2)
			{
				//~ if (temp_running_nodes < 486)
				if ((temp_running_nodes*100)/486 < busy_cluster_threshold)
				{
					multiplier_file_to_load = 1;
					multiplier_file_evicted = 0;
					multiplier_nb_copy = 0;
				}
				//~ else
				//~ {
					//~ multiplier_file_to_load = temp_multiplier_file_to_load;
					//~ multiplier_file_evicted = temp_multiplier_file_evicted;
					//~ multiplier_nb_copy = temp_multiplier_nb_copy;
				//~ }
				#ifdef PRINT
				printf("At time %d, running nodes is %d. Multiplier are %d %d %d.\n", t, temp_running_nodes, multiplier_file_to_load, multiplier_file_evicted, multiplier_nb_copy);
				#endif
			}

	/* Test complexité réduite */
	//~ int nb_non_cores_rescheduled = 0;

	/* 1. Loop on available jobs. */
	struct Job* j = head_job;
	while (j != NULL)
	{		
		/* Test complexité réduite */
		//~ if (nb_non_available_cores < nb_cores && nb_non_cores_rescheduled < 486*2)
		//~ printf("%d\n", nb_cores - nb_non_available_cores); fflush(stdout);
		//~ if (nb_non_available_cores < nb_cores) /* Vrai version */
		if (j->cores <= nb_cores - nb_non_available_cores)
		{
			#ifdef PRINT
			printf("\nNeed to schedule job %d using file %d. T = %d\n", j->unique_id, j->data, t); fflush(stdout);
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);			
			#endif
			
			/* cas if EAT is t reset multipliers */
			if (start_immediately_if_EAT_is_t == 1)
			{
				multiplier_file_to_load = temp_multiplier_file_to_load;
				multiplier_file_evicted = temp_multiplier_file_evicted;
				multiplier_nb_copy = temp_multiplier_nb_copy;
			}
			
			if (mixed_strategy == 1)
			{
				if ((temp_running_nodes*100)/486 < busy_cluster_threshold)
				{
					multiplier_file_to_load = 1;
					multiplier_file_evicted = 0;
					multiplier_nb_copy = 0;
				}
				else
				{
					multiplier_file_to_load = temp_multiplier_file_to_load;
					multiplier_file_evicted = temp_multiplier_file_evicted;
					multiplier_nb_copy = temp_multiplier_nb_copy;
				}
				
				#ifdef PRINT
				printf("At time %d, running nodes is %d. Multiplier are %d %d %d.\n", t, temp_running_nodes, multiplier_file_to_load, multiplier_file_evicted, multiplier_nb_copy);
				#endif
				
				//~ if (j->cores*64 >= j->walltime)
				//~ {
					//~ multiplier_file_to_load = 1;
					//~ multiplier_file_evicted = 0;
					//~ multiplier_nb_copy = 0;
				//~ }
			}
			
			/* 2. Choose a node. */		
			/* Reset some values. */					
			min_score = -1;
			earliest_available_time = 0;
			// first_node_size_to_choose_from = 0;
			// last_node_size_to_choose_from = 0;
			is_being_loaded = false;
			time_to_reload_evicted_files = 0;
			nb_copy_file_to_load = 0;
			
			/* In which node size I can pick. */
			//~ get_node_size_to_choose_from(j->index_node_list, &first_node_size_to_choose_from, &last_node_size_to_choose_from);

			/* --- Reduced complexity nb of copy --- */
			//~ if (multiplier_nb_copy != 0)
			//~ {
				//~ time_or_data_already_checked = was_time_or_data_already_checked_for_nb_copy(j->data, time_or_data_already_checked_nb_of_copy_list);
			//~ }

			for (i = first_node_size_to_choose_from; i <= last_node_size_to_choose_from; i++)
			{
				struct Node* n = head_node[i]->head;
				while (n != NULL)
				{
					#ifdef PRINT
					printf("On node %d?\n", n->unique_id); fflush(stdout);
					#endif
										
					/* 2.1. A = Get the earliest available time from the number of cores required by the job and add it to the score. */
					earliest_available_time = n->cores[j->cores - 1]->available_time; /* -1 because tab start at 0 */
					
					/* Test complexité réduite */
					#ifdef NB_HOUR_MAX
					if (earliest_available_time > t + 3600*nb_h_scheduled)
					{
						goto next_node;
					}
					#endif
					
					if (earliest_available_time < t) /* A core can't be available before t. This happens when a node is idling. */				
					{
						earliest_available_time = t;
					}
					
					if (start_immediately_if_EAT_is_t == 1 && earliest_available_time == t) /* Ou dans une fenêtre ? */
					{
						multiplier_file_to_load = 1;
						multiplier_file_evicted = 0;
						multiplier_nb_copy = 0;
					}
										
					#ifdef PRINT
					printf("A: EAT is: %d.\n", earliest_available_time); fflush(stdout);
					#endif
					
					if (min_score == -1 || earliest_available_time < min_score)
					{
						/* Update the dividor of the multiplier in function of the file size; */
						//~ if (adaptative_multiplier == 1)
						//~ {
							/* Ancien adapatative multiplier que j'ai testé une première fois. */
							//~ if(j->data_size <= 128)
							//~ {
								//~ div_multiplier = 500;
							//~ }
							//~ else if (j->data_size <= 256)
							//~ {
								//~ div_multiplier = 1;
							//~ }
							//~ else /* cas 1024 */
							//~ {
								//~ div_multiplier = 1;
							//~ }
						//~ }s
						//~ printf("For job %d using file of size %f, div is %d.\n", j->unique_id, j->data_size, div_multiplier);
						//~ multiplier_file_to_load_increment = 0;
						
						/* 2.2. B = Compute the time to load all data. For this look at the data that will be available at the earliest available time of the node. */
						if (j->data == 0)
						{
							time_to_load_file = 0;
						}
						else
						{
							time_to_load_file = is_my_file_on_node_at_certain_time_and_transfer_time(earliest_available_time, n, t, j->data, j->data_size, &is_being_loaded); /* Use the intervals in each data to get this info. */
							
							/* Cas avec pénalité sur les gros jobs */
							//~ if (penalty_on_job_sizes != 0 && time_to_load_file != 0)
							//~ {
								//~ multiplier_file_to_load_increment = ((double) time_to_load_file*5)/10240; /* Max de chargement, echelle de 0 à 5 */
							//~ }
						}
						
						#ifdef PRINT
						printf("B: Time to load file: %f. Is being loaded? %d.\n", time_to_load_file, is_being_loaded); fflush(stdout);
						#endif
											
						if (min_score == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file < min_score)
						{
							/* 2.5. Get the amount of files that will be lost because of this load by computing the amount of data that end at the earliest time only on the supposely choosen cores, excluding current file of course. */
							if (multiplier_file_evicted == 0)
							{
								time_to_reload_evicted_files = 0;
							}
							else
							{
								time_to_reload_evicted_files = time_to_reload_percentage_of_files_ended_at_certain_time(earliest_available_time, n, j->data, (float) j->cores/20);
							}
							
							#ifdef PRINT
							printf("C: Time to reload evicted files %f.\n", time_to_reload_evicted_files); fflush(stdout);
							#endif
							
							if (min_score == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files < min_score)
							{
								/* Comenté car pas utilisé dans mon cas */
								/* 2.5bis Get number of copy of the file we want to load on other nodes (if you need to load a file that is) at the time that is predicted to be used. So if a file is already loaded on a lot of node, you have a penalty if you want to load it on a new node. */
								//~ if (time_to_load_file != 0 && is_being_loaded == false && multiplier_nb_copy != 0)
								//~ {
									//~ /* --- Reduced complexity nb of copy --- */
									//~ if (time_or_data_already_checked == -1)
									//~ {
										//~ #ifdef PRINT
										//~ printf("Need to compute nb of copy it was never done.\n");
										//~ #endif
										//~ nb_copy_file_to_load = get_nb_valid_copy_of_a_file(t, head_node, j->data);
										//~ create_and_insert_head_time_or_data_already_checked_nb_of_copy_list(time_or_data_already_checked_nb_of_copy_list, j->data, nb_copy_file_to_load);
										//~ time_or_data_already_checked = nb_copy_file_to_load;
										//~ #ifdef PRINT
										//~ printf("Compute nb of copy done, it's %d.\n", nb_copy_file_to_load);
										//~ #endif
									//~ }
									//~ else
									//~ {
										//~ nb_copy_file_to_load = time_or_data_already_checked;
										//~ #ifdef PRINT
										//~ printf("Already done for job %d at time %d so nb of copy is %d.\n", j->unique_id, t, nb_copy_file_to_load);
										//~ #endif
									//~ }
								//~ }
								//~ else
								//~ {
									nb_copy_file_to_load = 0;
								//~ }
								
								#ifdef PRINT
								printf("Nb of copy for data %d at time %d on node %d is %d.\n", j->data, earliest_available_time, n->unique_id, nb_copy_file_to_load); fflush(stdout);
								#endif
								
								/* Compute node's score. */
								score = earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files + nb_copy_file_to_load*time_to_load_file*multiplier_nb_copy;
																
								#ifdef PRINT	
								printf("Score for job %d is %d (EAT: %d + TL %d*%f + TRL %d*%f + NCP %d*%d*%f) with node %d.\n", j->unique_id, score, earliest_available_time, multiplier_file_to_load, time_to_load_file, multiplier_file_evicted, time_to_reload_evicted_files, nb_copy_file_to_load, multiplier_nb_copy, time_to_load_file, n->unique_id); fflush(stdout);
								#endif
																					
								/* 2.6. Get minimum score/ */
								/* TODO : simpliefier la complexité: si EAT est t et TL et TLE == 0 alors break */
								if (min_score == -1)
								{
									min_time = earliest_available_time;
									min_score = score;
									j->node_used = n;
									choosen_time_to_load_file = time_to_load_file;
								}
								else if (min_score > score)
								{
									min_time = earliest_available_time;
									min_score = score;
									j->node_used = n;
									choosen_time_to_load_file = time_to_load_file;
								}
							}
						}
					}
					
					#ifdef PRINT_SCORES_DATA
					fprintf(f_fcfs_score, "Node: %d EAT: %d C: %f CxX: %f Score: %f\n", n->unique_id, earliest_available_time, time_to_reload_evicted_files, time_to_reload_evicted_files*multiplier_file_evicted, earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files);
					#endif
					
					#ifdef NB_HOUR_MAX
					/* Test complexité réduite */
					next_node: ;
					#endif
					
					n = n->next;
				}
			}
			
			
			/* Test complexité réduite */
			/* Update info if job was successfully scheduled. Else I put -1 at start_time to avoid any issue. */
			if (min_score != -1)
			{
				/* Test complexité réduite */
				//~ nb_non_cores_rescheduled += j->cores;
				
				j->transfer_time = choosen_time_to_load_file;
						
				/* Get start time and update available times of the cores. */
				j->start_time = min_time;
				j->end_time = min_time + j->walltime;
				
				/* Cas mixte pour LEO */
				if (mixed_strategy == 1 && j->node_used->n_available_cores == 20 && j->start_time == t)
				{
					#ifdef PRINT
					printf("Used node is %d.\n", j->node_used->unique_id);
					#endif
					
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
				
				for (int k = 0; k < j->cores; k++)
				{
					j->cores_used[k] = j->node_used->cores[k]->unique_id;
					if (j->node_used->cores[k]->available_time <= t)
					{
						nb_non_available_cores += 1;
					}
					j->node_used->cores[k]->available_time = min_time + j->walltime;
					
					/* Maybe I need job queue or not not sure. TODO. */
				}

				/* Need to add here intervals for current scheduling. */
				found = false;
				
				#ifdef DATA_PERSISTENCE
				struct Data* d = j->node_used->temp_data->head;
				#else
				struct Data* d = j->node_used->data->head;
				#endif
							
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
					#ifdef PRINT
					printf("Need to create a data and intervals for the node %d data %d.\n", j->node_used->unique_id, j->data); fflush(stdout);
					#endif
					
					/* Create a class Data for this node. */
					struct Data* new = (struct Data*) malloc(sizeof(struct Data));
					new->next = NULL;
					new->unique_id = j->data;
					new->start_time = -1;
					new->end_time = -1;
					
					#ifndef DATA_PERSISTENCE
					new->nb_task_using_it = 0;
					#endif
					
					new->intervals = (struct Interval_List*) malloc(sizeof(struct Interval_List));
					new->intervals->head = NULL;
					new->intervals->tail = NULL;
					create_and_insert_tail_interval_list(new->intervals, j->start_time);
					create_and_insert_tail_interval_list(new->intervals, j->start_time + j->transfer_time);
					create_and_insert_tail_interval_list(new->intervals, j->end_time);
					new->size = j->data_size;
					
					#ifdef DATA_PERSISTENCE
					insert_tail_data_list(j->node_used->temp_data, new);
					#else
					insert_tail_data_list(j->node_used->data, new);
					#endif
				}			
				
				/* Need to sort cores after each schedule of a job. */
				sort_cores_by_available_time_in_specific_node(j->node_used);
											
				#ifdef PRINT
				print_decision_in_scheduler(j);
				#endif
										
				/* Insert in start times. */
				insert_next_time_in_sorted_list(start_times, j->start_time);
							
				/* Je le commente car je n'utilise pas ce multiplier et ça fais hgagner un if */
				/* Increment nb of copy for current file if we scheduled at time t the current job. */
				//~ if (multiplier_nb_copy != 0 && j->start_time == t)
				//~ {
					//~ increment_time_or_data_nb_of_copy_specific_time_or_data(time_or_data_already_checked_nb_of_copy_list, j->data);
				//~ }
			}
			/* Test complexité réduite */
			else /* Could not schedule the job in the time frame t + 1h */
			{
				j->start_time = -1;
			}
			j = j->next;
		}				
		else
		{
			#ifdef PRINT
			printf("No more available cores.\n"); fflush(stdout);
			#endif
			
			/* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			while (j != NULL)
			{
				j->start_time = -1;
				j = j->next;
			}
			
			break;
		}
	}
	
	/* Je le commente car je n'utilise pas ce multiplier et ça fais hgagner un if */
	/* --- Reduced complexity nb of copy --- */
	/* Free time already checked. */
	//~ if (multiplier_nb_copy != 0)
	//~ {
		//~ free_time_or_data_already_checked_nb_of_copy_linked_list(&time_or_data_already_checked_nb_of_copy_list->head);
	//~ }

	#ifdef PRINT_SCORES_DATA
	fclose(f_fcfs_score);
	#endif
}

double fake_fcfs_with_a_score_scheduler(struct Job* head_job, struct Node_List** head_node, int t, int multiplier_file_to_load, int multiplier_file_evicted, int multiplier_nb_copy, int adaptative_multiplier, int penalty_on_job_sizes)
{
	#ifdef PRINT
	printf("\nFcfs_score\n");
	#endif
	
	double total_flow = 0;
	//~ int nb_non_available_cores = get_nb_non_available_cores(node_list, t);		
	int i = 0;
	int min_score = -1;
	int earliest_available_time = 0;
	// int // first_node_size_to_choose_from = 0;
	// int // last_node_size_to_choose_from = 0;
	float time_to_load_file = 0;
	bool is_being_loaded = false;
	float time_to_reload_evicted_files = 0;
	int nb_copy_file_to_load = 0;
	int time_or_data_already_checked = 0;
	int score = 0;
	int min_time = 0;
	int choosen_time_to_load_file = 0;
	bool found = false;
	int min_between_delay_and_walltime = 0;
	//~ float multiplier_file_to_load_increment = 0;
	double nb_jobs_scheduled = 0;
	
	if (adaptative_multiplier == 1)
	{
		if (multiplier_file_to_load != 0)
		{
			multiplier_file_to_load = running_nodes;
		}
		//~ if (multiplier_file_evicted != 0)
		//~ {
			//~ multiplier_file_evicted = 1;
		//~ }
		//~ if (multiplier_nb_copy != 0)
		//~ {
			//~ multiplier_nb_copy = 1;
		//~ }
	}
	
	//~ printf("Multiplier are %d %d %d.\n", multiplier_file_to_load, multiplier_file_evicted, multiplier_nb_copy);
					
	/* Get intervals of data. */ 
	get_current_intervals(head_node, t);
	
	//~ #ifdef PRINT
	//~ print_data_intervals(head_node, t);
	//~ #endif
	
	#ifdef PRINT_SCORES_DATA
	FILE* f_fcfs_score = fopen("outputs/Scores_data.txt", "a");
	#endif
	
	/* --- Reduced complexity nb of copy --- */	
	//~ if (multiplier_nb_copy != 0)
	//~ {
		struct Time_or_Data_Already_Checked_Nb_of_Copy_List* time_or_data_already_checked_nb_of_copy_list = (struct Time_or_Data_Already_Checked_Nb_of_Copy_List*) malloc(sizeof(struct Time_or_Data_Already_Checked_Nb_of_Copy_List));
		time_or_data_already_checked_nb_of_copy_list->head = NULL;
	//~ }

	/* 1. Loop on available jobs. */
	struct Job* j = head_job;
	struct Node* node_used = (struct Node*) malloc(sizeof(struct Node));	
	while (j != NULL)
	{		
		//~ if (nb_non_available_cores < nb_cores)
		//~ {
			//~ #ifdef PRINT
			//~ printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);			
			//~ printf("\nNeed to schedule job %d using file %d. T = %d\n", j->unique_id, j->data, t); fflush(stdout);
			//~ #endif
			
			/* 2. Choose a node. */		
			/* Reset some values. */					
			min_score = -1;
			earliest_available_time = 0;
			// first_node_size_to_choose_from = 0;
			// last_node_size_to_choose_from = 0;
			is_being_loaded = false;
			time_to_reload_evicted_files = 0;
			nb_copy_file_to_load = 0;
			
			/* In which node size I can pick. */
			// get_node_size_to_choose_from(j->index_node_list, &first_node_size_to_choose_from, &last_node_size_to_choose_from);
			//~ if (j->index_node_list == 0)
			//~ {
				//~ // first_node_size_to_choose_from = 0;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 1)
			//~ {
				//~ first_node_size_to_choose_from = 1;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 2)
			//~ {
				//~ first_node_size_to_choose_from = 2;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else
			//~ {
				//~ printf("Error index value in schedule_job_on_earliest_available_cores.\n");  fflush(stdout);
				//~ exit(EXIT_FAILURE);
			//~ }
			
			/* --- Normal complexity nb of copy --- */		
			/* For the number of valid copy of a data on other nodes. I add in this list the times I already checked for current job. */
			//~ struct Time_or_Data_Already_Checked_Nb_of_Copy_List* time_or_data_already_checked_nb_of_copy_list = (struct Time_or_Data_Already_Checked_Nb_of_Copy_List*) malloc(sizeof(struct Time_or_Data_Already_Checked_Nb_of_Copy_List));
			//~ time_or_data_already_checked_nb_of_copy_list->head = NULL;
			
			/* --- Reduced complexity nb of copy --- */
			if (multiplier_nb_copy != 0)
			{
				time_or_data_already_checked = was_time_or_data_already_checked_for_nb_copy(j->data, time_or_data_already_checked_nb_of_copy_list);
			}

			for (i = first_node_size_to_choose_from; i <= last_node_size_to_choose_from; i++)
			{
				struct Node* n = head_node[i]->head;
				while (n != NULL)
				{
					#ifdef PRINT
					printf("On node %d?\n", n->unique_id); fflush(stdout);
					#endif
										
					/* 2.1. A = Get the earliest available time from the number of cores required by the job and add it to the score. */
					earliest_available_time = n->cores[j->cores - 1]->available_time; /* -1 because tab start at 0 */
					if (earliest_available_time < t) /* A core can't be available before t. This happens when a node is idling. */				
					{
						earliest_available_time = t;
					}
										
					#ifdef PRINT
					printf("A: EAT is: %d.\n", earliest_available_time); fflush(stdout);
					#endif
					
					if (min_score == -1 || earliest_available_time < min_score)
					{						
						/* 2.2. B = Compute the time to load all data. For this look at the data that will be available at the earliest available time of the node. */
						if (j->data == 0)
						{
							time_to_load_file = 0;
						}
						else
						{
							time_to_load_file = is_my_file_on_node_at_certain_time_and_transfer_time(earliest_available_time, n, t, j->data, j->data_size, &is_being_loaded); /* Use the intervals in each data to get this info. */
						}
						
						#ifdef PRINT
						printf("B: Time to load file: %f. Is being loaded? %d.\n", time_to_load_file, is_being_loaded); fflush(stdout);
						#endif
											
						if (min_score == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file < min_score)
						{
							/* 2.5. Get the amount of files that will be lost because of this load by computing the amount of data that end at the earliest time only on the supposely choosen cores, excluding current file of course. */
							if (multiplier_file_evicted == 0)
							{
								time_to_reload_evicted_files = 0;
							}
							else
							{
								time_to_reload_evicted_files = time_to_reload_percentage_of_files_ended_at_certain_time(earliest_available_time, n, j->data, (float) j->cores/20);
							}
							
							#ifdef PRINT
							printf("C: Time to reload evicted files %f.\n", time_to_reload_evicted_files); fflush(stdout);
							#endif
							
							if (min_score == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files < min_score)
							{
								/* 2.5bis Get number of copy of the file we want to load on other nodes (if you need to load a file that is) at the time that is predicted to be used. So if a file is already loaded on a lot of node, you have a penalty if you want to load it on a new node. */
								if (time_to_load_file != 0 && is_being_loaded == false && multiplier_nb_copy != 0)
								{
									/* --- Reduced complexity nb of copy --- */
									if (time_or_data_already_checked == -1)
									{
										#ifdef PRINT
										printf("Need to compute nb of copy it was never done.\n");
										#endif
										nb_copy_file_to_load = get_nb_valid_copy_of_a_file(t, head_node, j->data);
										create_and_insert_head_time_or_data_already_checked_nb_of_copy_list(time_or_data_already_checked_nb_of_copy_list, j->data, nb_copy_file_to_load);
										time_or_data_already_checked = nb_copy_file_to_load;
										#ifdef PRINT
										printf("Compute nb of copy done, it's %d.\n", nb_copy_file_to_load);
										#endif
									}
									else
									{
										nb_copy_file_to_load = time_or_data_already_checked;
										#ifdef PRINT
										printf("Already done for job %d at time %d so nb of copy is %d.\n", j->unique_id, t, nb_copy_file_to_load);
										#endif
									}
								}
								else
								{
									nb_copy_file_to_load = 0;
								}
								
								#ifdef PRINT
								printf("Nb of copy for data %d at time %d on node %d is %d.\n", j->data, earliest_available_time, n->unique_id, nb_copy_file_to_load); fflush(stdout);
								#endif
								
								/* Compute node's score. */
								score = earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files + nb_copy_file_to_load*time_to_load_file*multiplier_nb_copy;
																
								#ifdef PRINT	
								printf("Score for job %d is %d (EAT: %d + TL %d*%f + TRL %d*%f + NCP %d*%d*%f) with node %d.\n", j->unique_id, score, earliest_available_time, multiplier_file_to_load, time_to_load_file, multiplier_file_evicted, time_to_reload_evicted_files, nb_copy_file_to_load, multiplier_nb_copy, time_to_load_file, n->unique_id); fflush(stdout);
								#endif
																					
								/* 2.6. Get minimum score/ */
								if (min_score == -1)
								{
									min_time = earliest_available_time;
									min_score = score;
									node_used = n;
									choosen_time_to_load_file = time_to_load_file;
								}
								else if (min_score > score)
								{
									min_time = earliest_available_time;
									min_score = score;
									node_used = n;
									choosen_time_to_load_file = time_to_load_file;
								}
							}
						}
					}
					
					#ifdef PRINT_SCORES_DATA
					fprintf(f_fcfs_score, "Node: %d EAT: %d C: %f CxX: %f Score: %f\n", n->unique_id, earliest_available_time, time_to_reload_evicted_files, time_to_reload_evicted_files*multiplier_file_evicted, earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files);
					#endif
					
					n = n->next;
				}
			}
			
			//~ j->transfer_time = choosen_time_to_load_file;
					
			/* Get start time and update available times of the cores. */
			//~ j->start_time = min_time;
			//~ j->end_time = min_time + j->walltime;
			
			if (j->delay + choosen_time_to_load_file < j->walltime)
			{
				min_between_delay_and_walltime = j->delay + choosen_time_to_load_file;
			}
			else
			{
				min_between_delay_and_walltime = j->walltime;
			}
			
			/* Update total flow that we return at the end. */
			total_flow += min_time + min_between_delay_and_walltime - j->subtime;
			nb_jobs_scheduled += 1;
			
			for (int k = 0; k < j->cores; k++)
			{
				//~ j->cores_used[k] = j->node_used->cores[k]->unique_id;
				//~ if (j->node_used->cores[k]->available_time <= t)
				//~ {
					//~ nb_non_available_cores += 1;
				//~ }
				node_used->cores[k]->available_time = min_time + j->walltime;
				
				/* Maybe I need job queue or not not sure. TODO. */
			}

			/* Need to add here intervals for current scheduling. */
			found = false;
			struct Data* d = node_used->data->head;
			while (d != NULL)
			{
				if (d->unique_id == j->data)
				{
					found = true;
					create_and_insert_tail_interval_list(d->intervals, min_time);
					create_and_insert_tail_interval_list(d->intervals, min_time + choosen_time_to_load_file);
					create_and_insert_tail_interval_list(d->intervals, min_time + j->walltime);
					break;
				}
				d = d->next;
			}
			
			if (found == false)
			{
				#ifdef PRINT
				printf("Need to create a data and intervals for the node %d data %d.\n", node_used->unique_id, j->data); fflush(stdout);
				#endif
				
				/* Create a class Data for this node. */
				struct Data* new = (struct Data*) malloc(sizeof(struct Data));
				new->next = NULL;
				new->unique_id = j->data;
				new->start_time = -1;
				new->end_time = -1;
				
				#ifndef DATA_PERSISTENCE
				new->nb_task_using_it = 0;
				#endif
				
				new->intervals = (struct Interval_List*) malloc(sizeof(struct Interval_List));
				new->intervals->head = NULL;
				new->intervals->tail = NULL;
				create_and_insert_tail_interval_list(new->intervals, min_time);
				create_and_insert_tail_interval_list(new->intervals, min_time + choosen_time_to_load_file);
				create_and_insert_tail_interval_list(new->intervals, min_time + j->walltime);
				new->size = j->data_size;
				insert_tail_data_list(node_used->data, new);
			}			
			
			//~ #ifdef PRINT
			//~ printf("After add interval are:\n"); fflush(stdout);
			//~ print_data_intervals(head_node, t);
			//~ #endif
			
			/* Need to sort cores after each schedule of a job. */
			sort_cores_by_available_time_in_specific_node(node_used);
										
			//~ #ifdef PRINT
			//~ print_decision_in_scheduler(j);
			//~ #endif
			
			//~ if (j->node_used->unique_id == 28 || j->unique_id == 968)
			//~ {
				//~ printf("T = %d | ", t);
				//~ print_decision_in_scheduler(j);
			//~ }
						
			/* Insert in start times. */
			//~ insert_next_time_in_sorted_list(start_times, j->start_time);
			
			/* --- Normal complexity nb of copy --- */
			/* Free time already checked. */
			//~ free_time_or_data_already_checked_nb_of_copy_linked_list(&time_or_data_already_checked_nb_of_copy_list->head);
			
			/* --- Normal complexity nb of copy --- */
			/* Increment nb of copy for current file if we scheduled at time t the current job. */
			if (multiplier_nb_copy != 0 && j->start_time == t)
			{
				//~ printf("Need to increment for job %d Multi is %d.\n", j->unique_id, multiplier_nb_copy); fflush(stdout);
				increment_time_or_data_nb_of_copy_specific_time_or_data(time_or_data_already_checked_nb_of_copy_list, j->data);
				//~ printf("Increment ok for job %d.\n", j->unique_id); fflush(stdout);
			}
			
			j = j->next;
		//~ }				
		//~ else
		//~ {
			//~ #ifdef PRINT
			//~ printf("No more available cores.\n"); fflush(stdout);
			//~ #endif
			
			//~ /* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			//~ while (j != NULL)
			//~ {
				//~ j->start_time = -1;
				//~ j = j->next;
			//~ }
			
			//~ break;
		//~ }
	}
	
	/* --- Reduced complexity nb of copy --- */
	/* Free time already checked. */
	if (multiplier_nb_copy != 0)
	{
		free_time_or_data_already_checked_nb_of_copy_linked_list(&time_or_data_already_checked_nb_of_copy_list->head);
	}

	#ifdef PRINT_SCORES_DATA
	fclose(f_fcfs_score);
	#endif
	
	return total_flow/nb_jobs_scheduled;
}

/** Utilise la variable globale busy_cluster pour
 *  adapter sa stratégie. Version localité içi
 *  mixed_strategy_version = 2 veut dire qu'on prend le temps moyen perdu par une donnée qu'on charge sur des noeuds différents.
 **/
void locality_scheduler(struct Job* head_job, struct Node_List** head_node, int t)
{
	#ifdef PRINT
	printf("LOCALITY\n");
	#endif
	
	int nb_non_available_cores = get_nb_non_available_cores(node_list, t);
	int min_score_locality = -1;
	int i = 0;
	bool best_score_is_zero = false;
	int earliest_available_time = 0;
	// int // first_node_size_to_choose_from = 0;
	// int // last_node_size_to_choose_from = 0;
	float time_to_load_file = 0;
	bool is_being_loaded = false;
	int score = 0;
	int min_time = 0;
	int choosen_time_to_load_file = 0;
	bool found = false;
	float time_to_reload_evicted_files = 0;
	
	/* Get intervals of data. */ 
	get_current_intervals(head_node, t);
	
	/* 1. Loop on available jobs. */
	struct Job* j = head_job;
	while (j != NULL)
	{
		if (nb_non_available_cores < nb_cores)
		{
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);			
			printf("\nNeed to schedule job %d using file %d. T = %d\n", j->unique_id, j->data, t); fflush(stdout);
			#endif
			
			/* 2. Choose a node. */		
			/* Reset some values. */					
			min_score_locality = -1;
			earliest_available_time = 0;
			is_being_loaded = false;
			time_to_load_file = 0;
			time_to_reload_evicted_files = 0;
			best_score_is_zero = false;
			
			/* In which node size I can pick. */
			// get_node_size_to_choose_from(j->index_node_list, &first_node_size_to_choose_from, &last_node_size_to_choose_from);
			//~ if (j->index_node_list == 0)
			//~ {
				//~ // first_node_size_to_choose_from = 0;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 1)
			//~ {
				//~ first_node_size_to_choose_from = 1;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 2)
			//~ {
				//~ first_node_size_to_choose_from = 2;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else
			//~ {
				//~ printf("Error index value in schedule_job_on_earliest_available_cores.\n");  fflush(stdout);
				//~ exit(EXIT_FAILURE);
			//~ }
			
			/* NEW */
			/* Calcul du threshold dans lequel on regarde le EAT */
			//~ if (mixed_strategy_version == 2)
			//~ {
				//~ time_to_load_penalty = ((5.830780553511172 - 1)/2 + 1)*(j->data_size/0.1); /* Base mean 3* */
				//~ time_to_load_penalty = ((200 - 1)/2 + 1)*(j->data_size/0.1); /* Exagerated mean 100* */
				//~ time_to_load_penalty = ((400 - 1)/2 + 1)*(j->data_size/0.1); /* Very exagerated mean 200* */
			//~ }
			//~ else
			//~ {
				//~ time_to_load_penalty = 0;
			//~ }
			
			//~ #ifdef PRINT
			//~ printf("Time to load penalty is %f.\n", time_to_load_penalty);
			//~ #endif
			
			for (i = first_node_size_to_choose_from; i <= last_node_size_to_choose_from; i++)
			{
				struct Node* n = head_node[i]->head;
				while (n != NULL)
				{
					#ifdef PRINT
					printf("On node %d?\n", n->unique_id); fflush(stdout);
					#endif
					
					/* 2.1. A = Get the earliest available time from the number of cores required by the job. */
					earliest_available_time = n->cores[j->cores - 1]->available_time; /* -1 because tab start at 0 */
					if (earliest_available_time < t) /* A core can't be available before t. This happens when a node is idling. */				
					{
						earliest_available_time = t;
					}
					
					#ifdef PRINT
					printf("EAT is: %d.\n", earliest_available_time); fflush(stdout);
					#endif
					
					if (min_score_locality == -1 || best_score_is_zero == false || earliest_available_time < min_time)
					{
						/* 2.2. Compute the time to load all data. For this look at the data that will be available at the earliest available time of the node. */
						if (j->data == 0)
						{
							time_to_load_file = 0;
						}
						else
						{
							time_to_load_file = is_my_file_on_node_at_certain_time_and_transfer_time(earliest_available_time, n, t, j->data, j->data_size, &is_being_loaded); /* Use the intervals in each data to get this info. */
						}
						
						//~ #ifdef PRINT
						//~ printf("Time to load file is %f. Amount of file to load is %f. Is being loaded? %d.\n", time_to_load_file, amount_of_file_remaining_to_load, is_being_loaded); fflush(stdout);
						//~ #endif
						
						if (min_score_locality == -1 || time_to_load_file <= min_score_locality)
						{
							/* 2.5. Get the amount of files that will be lost because of this load by computing the amount of data that end at the earliest time only on the supposely choosen cores, excluding current file of course. */
							time_to_reload_evicted_files = time_to_reload_percentage_of_files_ended_at_certain_time(earliest_available_time, n, j->data, (float) j->cores/20);
							
							#ifdef PRINT
							printf("Time to reload evicted files %f.\n", time_to_reload_evicted_files); fflush(stdout);
							#endif
															
								/* Compute node's score. */
								score = time_to_load_file + time_to_reload_evicted_files;
								
								#ifdef PRINT	
								printf("Score for job %d is %d (TL %f + TLE %f) with node %d.\n", j->unique_id, score, time_to_load_file, time_to_reload_evicted_files, n->unique_id); fflush(stdout);
								#endif
								
								/* 2.6. Get minimum score/ */
								if (min_score_locality == -1 || min_score_locality > score || (min_score_locality == score && min_time > earliest_available_time))
								{
									min_time = earliest_available_time;
									min_score_locality = score;
									j->node_used = n;
									choosen_time_to_load_file = time_to_load_file;
									
									if (score == 0) /* To try and reduce complexity by not computing time to load if the best is 0 and our EAT is worse. */
									{
										best_score_is_zero = true;
									}
								}
							//~ }
						}
					}
					n = n->next;
				}
			}
			
			j->transfer_time = choosen_time_to_load_file;
					
			/* Get start time and update available times of the cores. */
			j->start_time = min_time;
			j->end_time = min_time + j->walltime;
			
			for (int k = 0; k < j->cores; k++)
			{
				j->cores_used[k] = j->node_used->cores[k]->unique_id;
				if (j->node_used->cores[k]->available_time <= t)
				{
					nb_non_available_cores += 1;
				}
				j->node_used->cores[k]->available_time = min_time + j->walltime;
				
				/* Maybe I need job queue or not not sure. TODO. */
			}

			/* Need to add here intervals for current scheduling. */
			found = false;
			
			#ifdef DATA_PERSISTENCE
			struct Data* d = j->node_used->temp_data->head;
			#else
			struct Data* d = j->node_used->data->head;
			#endif

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
				#ifdef PRINT
				printf("Need to create a data and intervals for the node %d data %d.\n", j->node_used->unique_id, j->data); fflush(stdout);
				#endif
				
				/* Create a class Data for this node. */
				struct Data* new = (struct Data*) malloc(sizeof(struct Data));
				new->next = NULL;
				new->unique_id = j->data;
				new->start_time = -1;
				new->end_time = -1;
				
				#ifndef DATA_PERSISTENCE
				new->nb_task_using_it = 0;
				#endif
				
				new->intervals = (struct Interval_List*) malloc(sizeof(struct Interval_List));
				new->intervals->head = NULL;
				new->intervals->tail = NULL;
				create_and_insert_tail_interval_list(new->intervals, j->start_time);
				create_and_insert_tail_interval_list(new->intervals, j->start_time + j->transfer_time);
				create_and_insert_tail_interval_list(new->intervals, j->end_time);
				new->size = j->data_size;
				
				#ifdef DATA_PERSISTENCE
				insert_tail_data_list(j->node_used->temp_data, new);
				#else
				insert_tail_data_list(j->node_used->data, new);
				#endif
			}	
			
			#ifdef PRINT
			printf("After add interval are:\n"); fflush(stdout);
			print_data_intervals(head_node, t);
			#endif
			
			/* Need to sort cores after each schedule of a job. */
			sort_cores_by_available_time_in_specific_node(j->node_used);
										
			#ifdef PRINT
			print_decision_in_scheduler(j);
			#endif

			/* Insert in start times. */
			insert_next_time_in_sorted_list(start_times, j->start_time);
						
			j = j->next;
		}				
		else
		{
			#ifdef PRINT
			printf("No more available cores.\n"); fflush(stdout);
			#endif
			
			/* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			while (j != NULL)
			{
				j->start_time = -1;
				j = j->next;
			}

			break;
		}
	}
}

int locality_scheduler_single_job(struct Job* j, struct Node_List** head_node, int t, int nb_non_available_cores, int mode)
{
	#ifdef PRINT
	printf("LOCALITY SINGLE JOB\n");
	#endif
	
	//~ int nb_non_available_cores = get_nb_non_available_cores(node_list, t);
	int min_score_locality = -1;
	int i = 0;
	bool best_score_is_zero = false;
	int earliest_available_time = 0;
	// int // first_node_size_to_choose_from = 0;
	// int // last_node_size_to_choose_from = 0;
	float time_to_load_file = 0;
	bool is_being_loaded = false;
	int score = 0;
	int min_time = 0;
	int choosen_time_to_load_file = 0;
	//~ bool found = false;
	float time_to_reload_evicted_files = 0;
	
	/* Get intervals of data. */ 
	//~ get_current_intervals(head_node, t);
	
	/* 1. Loop on available jobs. */
	//~ struct Job* j = head_job;
	//~ while (j != NULL)
	//~ {
		//~ if (nb_non_available_cores < nb_cores)
		//~ {
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);			
			printf("\nNeed to schedule job %d using file %d. T = %d\n", j->unique_id, j->data, t); fflush(stdout);
			#endif
			
			/* 2. Choose a node. */		
			/* Reset some values. */					
			//~ min_score_locality = -1;
			//~ earliest_available_time = 0;
			//~ is_being_loaded = false;
			//~ time_to_load_file = 0;
			//~ time_to_reload_evicted_files = 0;
			//~ best_score_is_zero = false;
			
			/* In which node size I can pick. */
			// get_node_size_to_choose_from(j->index_node_list, &first_node_size_to_choose_from, &last_node_size_to_choose_from);
			//~ if (j->index_node_list == 0)
			//~ {
				//~ // first_node_size_to_choose_from = 0;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 1)
			//~ {
				//~ first_node_size_to_choose_from = 1;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 2)
			//~ {
				//~ first_node_size_to_choose_from = 2;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else
			//~ {
				//~ printf("Error index value in schedule_job_on_earliest_available_cores.\n");  fflush(stdout);
				//~ exit(EXIT_FAILURE);
			//~ }
			
			/* NEW */
			/* Calcul du threshold dans lequel on regarde le EAT */
			//~ if (mixed_strategy_version == 2)
			//~ {
				//~ time_to_load_penalty = ((5.830780553511172 - 1)/2 + 1)*(j->data_size/0.1); /* Base mean 3* */
				//~ time_to_load_penalty = ((200 - 1)/2 + 1)*(j->data_size/0.1); /* Exagerated mean 100* */
				//~ time_to_load_penalty = ((400 - 1)/2 + 1)*(j->data_size/0.1); /* Very exagerated mean 200* */
			//~ }
			//~ else
			//~ {
				//~ time_to_load_penalty = 0;
			//~ }
			
			//~ #ifdef PRINT
			//~ printf("Time to load penalty is %f.\n", time_to_load_penalty);
			//~ #endif
			
			for (i = first_node_size_to_choose_from; i <= last_node_size_to_choose_from; i++)
			{
				struct Node* n = head_node[i]->head;
				while (n != NULL)
				{
					#ifdef PRINT
					printf("On node %d?\n", n->unique_id); fflush(stdout);
					#endif
					
					/* 2.1. A = Get the earliest available time from the number of cores required by the job. */
					earliest_available_time = n->cores[j->cores - 1]->available_time; /* -1 because tab start at 0 */
					if (earliest_available_time < t) /* A core can't be available before t. This happens when a node is idling. */				
					{
						earliest_available_time = t;
					}
					
					#ifdef PRINT
					printf("EAT is: %d.\n", earliest_available_time); fflush(stdout);
					#endif
					
					if (min_score_locality == -1 || best_score_is_zero == false || earliest_available_time < min_time)
					{
						/* 2.2. Compute the time to load all data. For this look at the data that will be available at the earliest available time of the node. */
						if (j->data == 0)
						{
							time_to_load_file = 0;
						}
						else
						{
							time_to_load_file = is_my_file_on_node_at_certain_time_and_transfer_time(earliest_available_time, n, t, j->data, j->data_size, &is_being_loaded); /* Use the intervals in each data to get this info. */
						}
						
						//~ #ifdef PRINT
						//~ printf("Time to load file is %f. Amount of file to load is %f. Is being loaded? %d.\n", time_to_load_file, amount_of_file_remaining_to_load, is_being_loaded); fflush(stdout);
						//~ #endif
						
						if (min_score_locality == -1 || time_to_load_file <= min_score_locality)
						{
							/* 2.5. Get the amount of files that will be lost because of this load by computing the amount of data that end at the earliest time only on the supposely choosen cores, excluding current file of course. */
							time_to_reload_evicted_files = time_to_reload_percentage_of_files_ended_at_certain_time(earliest_available_time, n, j->data, (float) j->cores/20);
							
							#ifdef PRINT
							printf("Time to reload evicted files %f.\n", time_to_reload_evicted_files); fflush(stdout);
							#endif
															
								/* Compute node's score. */
								if (mode == 0)
								{
									score = time_to_load_file + time_to_reload_evicted_files;
								}
								else if (mode == 1)
								{
									score = time_to_load_file;
								}
								else
								{
									perror("error locality scheduler single job");
									exit(1);
								}
								
								#ifdef PRINT	
								printf("Score for job %d is %d (TL %f + TLE %f) with node %d.\n", j->unique_id, score, time_to_load_file, time_to_reload_evicted_files, n->unique_id); fflush(stdout);
								#endif
								
								/* 2.6. Get minimum score/ */
								if (min_score_locality == -1 || min_score_locality > score || (min_score_locality == score && min_time > earliest_available_time))
								{
									min_time = earliest_available_time;
									min_score_locality = score;
									j->node_used = n;
									choosen_time_to_load_file = time_to_load_file;
									
									if (score == 0) /* To try and reduce complexity by not computing time to load if the best is 0 and our EAT is worse. */
									{
										best_score_is_zero = true;
									}
								}
							//~ }
						}
					}
					n = n->next;
				}
			}
			
			j->transfer_time = choosen_time_to_load_file;
					
			/* Get start time and update available times of the cores. */
			j->start_time = min_time;
			j->end_time = min_time + j->walltime;
			
			for (int k = 0; k < j->cores; k++)
			{
				j->cores_used[k] = j->node_used->cores[k]->unique_id;
				if (j->node_used->cores[k]->available_time <= t)
				{
					nb_non_available_cores += 1;
				}
				j->node_used->cores[k]->available_time = min_time + j->walltime;
				
				/* Maybe I need job queue or not not sure. TODO. */
			}

			//~ /* Need to add here intervals for current scheduling. */
			//~ found = false;
			//~ struct Data* d = j->node_used->data->head;
			//~ while (d != NULL)
			//~ {
				//~ if (d->unique_id == j->data)
				//~ {
					//~ found = true;
					//~ create_and_insert_tail_interval_list(d->intervals, j->start_time);
					//~ create_and_insert_tail_interval_list(d->intervals, j->start_time + j->transfer_time);
					//~ create_and_insert_tail_interval_list(d->intervals, j->end_time);
					//~ break;
				//~ }
				//~ d = d->next;
			//~ }
			
			//~ if (found == false)
			//~ {
				//~ #ifdef PRINT
				//~ printf("Need to create a data and intervals for the node %d data %d.\n", j->node_used->unique_id, j->data); fflush(stdout);
				//~ #endif
				
				//~ /* Create a class Data for this node. */
				//~ struct Data* new = (struct Data*) malloc(sizeof(struct Data));
				//~ new->next = NULL;
				//~ new->unique_id = j->data;
				//~ new->start_time = -1;
				//~ new->end_time = -1;
				//~ new->nb_task_using_it = 0;
				//~ new->intervals = (struct Interval_List*) malloc(sizeof(struct Interval_List));
				//~ new->intervals->head = NULL;
				//~ new->intervals->tail = NULL;
				//~ create_and_insert_tail_interval_list(new->intervals, j->start_time);
				//~ create_and_insert_tail_interval_list(new->intervals, j->start_time + j->transfer_time);
				//~ create_and_insert_tail_interval_list(new->intervals, j->end_time);
				//~ new->size = j->data_size;
				//~ insert_tail_data_list(j->node_used->data, new);
			//~ }	
			
			//~ #ifdef PRINT
			//~ printf("After add interval are:\n"); fflush(stdout);
			//~ print_data_intervals(head_node, t);
			//~ #endif
			
			/* Need to sort cores after each schedule of a job. */
			sort_cores_by_available_time_in_specific_node(j->node_used);
										
			//~ #ifdef PRINT
			//~ print_decision_in_scheduler(j);
			//~ #endif

			/* Insert in start times. */
			insert_next_time_in_sorted_list(start_times, j->start_time);
						
			//~ j = j->next;
		//~ }				
		//~ else
		//~ {
			//~ #ifdef PRINT
			//~ printf("No more available cores.\n"); fflush(stdout);
			//~ #endif
			
			//~ /* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			//~ while (j != NULL)
			//~ {
				//~ j->start_time = -1;
				//~ j = j->next;
			//~ }

			//~ break;
		//~ }
	//~ }
	return nb_non_available_cores;
}

/** Return mean flow it would have gotten **/
double fake_locality_scheduler(struct Job* head_job, struct Node_List** head_node, int t)
{
	#ifdef PRINT
	printf("LOCALITY\n");
	#endif
	
	int min_between_delay_and_walltime = 0;
	double total_flow = 0;
	//~ int nb_non_available_cores = get_nb_non_available_cores(node_list, t);
	int min_score_locality = -1;
	int i = 0;
	bool best_score_is_zero = false;
	int earliest_available_time = 0;
	// int // first_node_size_to_choose_from = 0;
	// int // last_node_size_to_choose_from = 0;
	float time_to_load_file = 0;
	bool is_being_loaded = false;
	int score = 0;
	int min_time = 0;
	int choosen_time_to_load_file = 0;
	bool found = false;
	float time_to_reload_evicted_files = 0;
	double nb_jobs_scheduled = 0;			
	
	/* Get intervals of data. */ 
	get_current_intervals(head_node, t);
	
	/* 1. Loop on available jobs. */
	struct Node *node_used = (struct Node*) malloc(sizeof(struct Node));	
	struct Job* j = head_job;
	while (j != NULL)
	{
		//~ if (nb_non_available_cores < nb_cores)
		//~ {
			//~ #ifdef PRINT
			//~ printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);			
			//~ printf("\nNeed to schedule job %d using file %d. T = %d\n", j->unique_id, j->data, t); fflush(stdout);
			//~ #endif
			
			/* 2. Choose a node. */		
			/* Reset some values. */					
			min_score_locality = -1;
			earliest_available_time = 0;
			is_being_loaded = false;
			time_to_load_file = 0;
			time_to_reload_evicted_files = 0;
			best_score_is_zero = false;
			
			/* In which node size I can pick. */
			// get_node_size_to_choose_from(j->index_node_list, &first_node_size_to_choose_from, &last_node_size_to_choose_from);
			//~ if (j->index_node_list == 0)
			//~ {
				//~ // first_node_size_to_choose_from = 0;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 1)
			//~ {
				//~ first_node_size_to_choose_from = 1;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 2)
			//~ {
				//~ first_node_size_to_choose_from = 2;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else
			//~ {
				//~ printf("Error index value in schedule_job_on_earliest_available_cores.\n");  fflush(stdout);
				//~ exit(EXIT_FAILURE);
			//~ }
			
			
			//~ #ifdef PRINT
			//~ printf("Time to load penalty is %f.\n", time_to_load_penalty);
			//~ #endif
			
			for (i = first_node_size_to_choose_from; i <= last_node_size_to_choose_from; i++)
			{
				struct Node* n = head_node[i]->head;
				while (n != NULL)
				{
					#ifdef PRINT
					printf("On node %d?\n", n->unique_id); fflush(stdout);
					#endif
					
					/* 2.1. A = Get the earliest available time from the number of cores required by the job. */
					earliest_available_time = n->cores[j->cores - 1]->available_time; /* -1 because tab start at 0 */
					if (earliest_available_time < t) /* A core can't be available before t. This happens when a node is idling. */				
					{
						earliest_available_time = t;
					}
					
					#ifdef PRINT
					printf("EAT is: %d.\n", earliest_available_time); fflush(stdout);
					#endif
					
					if (min_score_locality == -1 || best_score_is_zero == false || earliest_available_time < min_time)
					{
						/* 2.2. Compute the time to load all data. For this look at the data that will be available at the earliest available time of the node. */
						if (j->data == 0)
						{
							time_to_load_file = 0;
						}
						else
						{
							time_to_load_file = is_my_file_on_node_at_certain_time_and_transfer_time(earliest_available_time, n, t, j->data, j->data_size, &is_being_loaded); /* Use the intervals in each data to get this info. */
						}
						
						//~ #ifdef PRINT
						//~ printf("Time to load file is %f. Amount of file to load is %f. Is being loaded? %d.\n", time_to_load_file, amount_of_file_remaining_to_load, is_being_loaded); fflush(stdout);
						//~ #endif
						
						if (min_score_locality == -1 || time_to_load_file <= min_score_locality)
						{
							/* 2.5. Get the amount of files that will be lost because of this load by computing the amount of data that end at the earliest time only on the supposely choosen cores, excluding current file of course. */
							time_to_reload_evicted_files = time_to_reload_percentage_of_files_ended_at_certain_time(earliest_available_time, n, j->data, (float) j->cores/20);
							
							#ifdef PRINT
							printf("Time to reload evicted files %f.\n", time_to_reload_evicted_files); fflush(stdout);
							#endif
															
								/* Compute node's score. */
								score = time_to_load_file + time_to_reload_evicted_files;
								
								#ifdef PRINT	
								printf("Score for job %d is %d (TL %f + TLE %f) with node %d.\n", j->unique_id, score, time_to_load_file, time_to_reload_evicted_files, n->unique_id); fflush(stdout);
								#endif
								
								/* 2.6. Get minimum score/ */
								if (min_score_locality == -1 || min_score_locality > score || (min_score_locality == score && min_time > earliest_available_time))
								{
									min_time = earliest_available_time;
									min_score_locality = score;
									//~ j->node_used = n;
									node_used = n;
									choosen_time_to_load_file = time_to_load_file;
									
									if (score == 0) /* To try and reduce complexity by not computing time to load if the best is 0 and our EAT is worse. */
									{
										best_score_is_zero = true;
									}
								}
							//~ }
						}
					}
					n = n->next;
				}
			}
			
			if (j->delay + choosen_time_to_load_file < j->walltime)
			{
				min_between_delay_and_walltime = j->delay + choosen_time_to_load_file;
			}
			else
			{
				min_between_delay_and_walltime = j->walltime;
			}
			
			/* Update total flow that we return at the end. */
			total_flow += min_time + min_between_delay_and_walltime - j->subtime;
			nb_jobs_scheduled += 1;
			
			//~ j->transfer_time = choosen_time_to_load_file;
					
			//~ /* Get start time and update available times of the cores. */
			//~ j->start_time = min_time;
			//~ j->end_time = min_time + j->walltime;
			
			for (int k = 0; k < j->cores; k++)
			{
				//~ j->cores_used[k] = j->node_used->cores[k]->unique_id;
				//~ if (j->node_used->cores[k]->available_time <= t)
				//~ {
					//~ nb_non_available_cores += 1;
				//~ }
				node_used->cores[k]->available_time = min_time + j->walltime;
				
				/* Maybe I need job queue or not not sure. TODO. */
			}

			/* Need to add here intervals for current scheduling. */
			found = false;
			struct Data* d = node_used->data->head;
			while (d != NULL)
			{
				if (d->unique_id == j->data)
				{
					found = true;
					create_and_insert_tail_interval_list(d->intervals, min_time);
					create_and_insert_tail_interval_list(d->intervals, min_time + choosen_time_to_load_file);
					create_and_insert_tail_interval_list(d->intervals, min_time + j->walltime);
					break;
				}
				d = d->next;
			}
			
			if (found == false)
			{
				#ifdef PRINT
				printf("Need to create a data and intervals for the node %d data %d.\n", j->node_used->unique_id, j->data); fflush(stdout);
				#endif
				
				/* Create a class Data for this node. */
				struct Data* new = (struct Data*) malloc(sizeof(struct Data));
				new->next = NULL;
				new->unique_id = j->data;
				new->start_time = -1;
				new->end_time = -1;
				
				#ifndef DATA_PERSISTENCE
				new->nb_task_using_it = 0;
				#endif
				
				new->intervals = (struct Interval_List*) malloc(sizeof(struct Interval_List));
				new->intervals->head = NULL;
				new->intervals->tail = NULL;
				create_and_insert_tail_interval_list(new->intervals, min_time);
				create_and_insert_tail_interval_list(new->intervals, min_time + choosen_time_to_load_file);
				create_and_insert_tail_interval_list(new->intervals, min_time + j->walltime);
				new->size = j->data_size;
				insert_tail_data_list(node_used->data, new);
			}	
			
			//~ #ifdef PRINT
			//~ printf("After add interval are:\n"); fflush(stdout);
			//~ print_data_intervals(head_node, t);
			//~ #endif
			
			/* Need to sort cores after each schedule of a job. */
			sort_cores_by_available_time_in_specific_node(node_used);
										
			//~ #ifdef PRINT
			//~ print_decision_in_scheduler(j);
			//~ #endif

			/* Insert in start times. */
			//~ insert_next_time_in_sorted_list(start_times, j->start_time);
						
			j = j->next;
		//~ }				
		//~ else
		//~ {
			//~ #ifdef PRINT
			//~ printf("No more available cores.\n"); fflush(stdout);
			//~ #endif
			
			//~ /* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			//~ while (j != NULL)
			//~ {
				//~ j->start_time = -1;
				//~ j = j->next;
			//~ }

			//~ break;
		//~ }
	}
	return total_flow/nb_jobs_scheduled;
}

void eft_scheduler(struct Job* head_job, struct Node_List** head_node, int t)
{
	#ifdef PRINT
	printf("\nEFT\n");
	#endif
	
	int nb_non_available_cores = get_nb_non_available_cores(node_list, t);
	int min_score = -1;
	int i = 0;
	int earliest_available_time = 0;
	// int // first_node_size_to_choose_from = 0;
	// int // last_node_size_to_choose_from = 0;
	float time_to_load_file = 0;
	bool is_being_loaded = false;
	int score = 0;
	int min_time = 0;
	int choosen_time_to_load_file = 0;
	bool found = false;	
					
	/* Get intervals of data. */ 
	get_current_intervals(head_node, t);
	
	#ifdef PRINT
	print_data_intervals(head_node, t);
	#endif

	/* 1. Loop on available jobs. */
	struct Job* j = head_job;
	while (j != NULL)
	{
		if (nb_non_available_cores < nb_cores)
		{
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);			
			printf("\nNeed to schedule job %d using file %d. T = %d\n", j->unique_id, j->data, t); fflush(stdout);
			#endif
			
			/* 2. Choose a node. */		
			/* Reset some values. */					
			min_score = -1;
			earliest_available_time = 0;
			is_being_loaded = false;
			
			/* In which node size I can pick. */
			// get_node_size_to_choose_from(j->index_node_list, &first_node_size_to_choose_from, &last_node_size_to_choose_from);
			//~ if (j->index_node_list == 0)
			//~ {
				//~ // first_node_size_to_choose_from = 0;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 1)
			//~ {
				//~ first_node_size_to_choose_from = 1;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 2)
			//~ {
				//~ first_node_size_to_choose_from = 2;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else
			//~ {
				//~ printf("Error index value in schedule_job_on_earliest_available_cores.\n");  fflush(stdout);
				//~ exit(EXIT_FAILURE);
			//~ }
						
			for (i = first_node_size_to_choose_from; i <= last_node_size_to_choose_from; i++)
			{
				struct Node* n = head_node[i]->head;
				while (n != NULL)
				{
					#ifdef PRINT
					printf("On node %d?\n", n->unique_id); fflush(stdout);
					#endif
					
					/* 2.1. A = Get the earliest available time from the number of cores required by the job. 
					 * It's used in case of a tie on the locality, as well as to look at the intervals of
					 * available data at the predicted start time. */
					earliest_available_time = n->cores[j->cores - 1]->available_time; /* -1 because tab start at 0 */
					if (earliest_available_time < t) /* A core can't be available before t. This happens when a node is idling. */				
					{
						earliest_available_time = t;
					}
					
					#ifdef PRINT
					printf("EAT is: %d.\n", earliest_available_time); fflush(stdout);
					#endif
					
					if (min_score == -1 || earliest_available_time < min_score)
					{						
						/* 2.2. Compute the time to load all data. For this look at the data that will be available at the earliest available time of the node. */
						if (j->data == 0)
						{
							time_to_load_file = 0;
						}
						else
						{
							time_to_load_file = is_my_file_on_node_at_certain_time_and_transfer_time(earliest_available_time, n, t, j->data, j->data_size, &is_being_loaded); /* Use the intervals in each data to get this info. */
						}
						
						#ifdef PRINT
						printf("Time to load file: %f. Is being loaded? %d.\n", time_to_load_file, is_being_loaded); fflush(stdout);
						#endif
						
						/* To test with 1 1 instead of EFT. To remove. J'ai testé, c'est pas ça qui explique lesm oins bonnes perf que fcfs score adaptative multiplier 500 500*/
						//~ if (min_score == -1 || earliest_available_time + time_to_load_file < min_score)
						//~ {
							/* To test with 1 1 instead of EFT. To remove. */
							//~ time_to_reload_evicted_files = time_to_reload_percentage_of_files_ended_at_certain_time(earliest_available_time, n, j->data, (float) j->cores/20);
							
						
								/* Compute node's score. */
								score = earliest_available_time + time_to_load_file;
								
								/* To test with 1 1 instead of EFT. To remove. */
								//~ score = earliest_available_time + time_to_load_file + time_to_reload_evicted_files;
																
								#ifdef PRINT	
								printf("Score for job %d is %d with node %d.\n", j->unique_id, score, n->unique_id); fflush(stdout);
								#endif
													
								/* 2.6. Get minimum score/ */
								if (min_score == -1)
								{
									min_time = earliest_available_time;
									min_score = score;
									j->node_used = n;
									choosen_time_to_load_file = time_to_load_file;
								}
								else if (min_score > score)
								{
									min_time = earliest_available_time;
									min_score = score;
									j->node_used = n;
									choosen_time_to_load_file = time_to_load_file;
								}
						//~ }
					}
					n = n->next;
				}
			}
			
			j->transfer_time = choosen_time_to_load_file;
					
			/* Get start time and update available times of the cores. */
			j->start_time = min_time;
			j->end_time = min_time + j->walltime;
			
			for (int k = 0; k < j->cores; k++)
			{
				j->cores_used[k] = j->node_used->cores[k]->unique_id;
				if (j->node_used->cores[k]->available_time <= t)
				{
					nb_non_available_cores += 1;
				}
				j->node_used->cores[k]->available_time = min_time + j->walltime;
				
				/* Maybe I need job queue or not not sure. TODO. */
			}

			/* Need to add here intervals for current scheduling. */
			found = false;
			
			#ifdef DATA_PERSISTENCE
			struct Data* d = j->node_used->temp_data->head;
			#else
			struct Data* d = j->node_used->data->head;
			#endif

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
				#ifdef PRINT
				printf("Need to create a data and intervals for the node %d data %d.\n", j->node_used->unique_id, j->data); fflush(stdout);
				#endif
				
				/* Create a class Data for this node. */
				struct Data* new = (struct Data*) malloc(sizeof(struct Data));
				new->next = NULL;
				new->unique_id = j->data;
				new->start_time = -1;
				new->end_time = -1;
				
				#ifndef DATA_PERSISTENCE
				new->nb_task_using_it = 0;
				#endif
				
				new->intervals = (struct Interval_List*) malloc(sizeof(struct Interval_List));
				new->intervals->head = NULL;
				new->intervals->tail = NULL;
				create_and_insert_tail_interval_list(new->intervals, j->start_time);
				create_and_insert_tail_interval_list(new->intervals, j->start_time + j->transfer_time);
				create_and_insert_tail_interval_list(new->intervals, j->end_time);
				new->size = j->data_size;
				
				#ifdef DATA_PERSISTENCE
				insert_tail_data_list(j->node_used->temp_data, new);
				#else
				insert_tail_data_list(j->node_used->data, new);
				#endif
			}	
			
			#ifdef PRINT
			printf("After add interval are:\n"); fflush(stdout);
			print_data_intervals(head_node, t);
			#endif
			
			/* Need to sort cores after each schedule of a job. */
			sort_cores_by_available_time_in_specific_node(j->node_used);
										
			#ifdef PRINT
			print_decision_in_scheduler(j);
			#endif
						
			/* Insert in start times. */
			insert_next_time_in_sorted_list(start_times, j->start_time);
						
			j = j->next;
		}				
		else
		{
			#ifdef PRINT
			printf("No more available cores.\n"); fflush(stdout);
			#endif
			
			/* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			while (j != NULL)
			{
				j->start_time = -1;
				j = j->next;
			}

			break;
		}
	}
}

int eft_scheduler_single_job(struct Job* j, struct Node_List** head_node, int t, int nb_non_available_cores)
{
	#ifdef PRINT
	printf("EFT SINGLE JOB\n");
	#endif
	
	//~ int nb_non_available_cores = get_nb_non_available_cores(node_list, t);
	int min_score = -1;
	int i = 0;
	int earliest_available_time = 0;
	// int // first_node_size_to_choose_from = 0;
	// int // last_node_size_to_choose_from = 0;
	float time_to_load_file = 0;
	bool is_being_loaded = false;
	int score = 0;
	int min_time = 0;
	int choosen_time_to_load_file = 0;
	//~ bool found = false;	
					
	/* Get intervals of data. */ 
	//~ get_current_intervals(head_node, t);
	
	#ifdef PRINT
	print_data_intervals(head_node, t);
	#endif

	/* 1. Loop on available jobs. */
	//~ struct Job* j = head_job;
	//~ while (j != NULL)
	//~ {
		//~ if (nb_non_available_cores < nb_cores)
		//~ {
			//~ #ifdef PRINT
			//~ printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);			
			//~ printf("\nNeed to schedule job %d using file %d. T = %d\n", j->unique_id, j->data, t); fflush(stdout);
			//~ #endif
			
			/* 2. Choose a node. */		
			/* Reset some values. */					
			//~ min_score = -1;
			//~ earliest_available_time = 0;
			//~ is_being_loaded = false;
			
			/* In which node size I can pick. */
			// get_node_size_to_choose_from(j->index_node_list, &first_node_size_to_choose_from, &last_node_size_to_choose_from);
			//~ if (j->index_node_list == 0)
			//~ {
				//~ // first_node_size_to_choose_from = 0;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 1)
			//~ {
				//~ first_node_size_to_choose_from = 1;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 2)
			//~ {
				//~ first_node_size_to_choose_from = 2;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else
			//~ {
				//~ printf("Error index value in schedule_job_on_earliest_available_cores.\n");  fflush(stdout);
				//~ exit(EXIT_FAILURE);
			//~ }
						
			for (i = first_node_size_to_choose_from; i <= last_node_size_to_choose_from; i++)
			{
				struct Node* n = head_node[i]->head;
				while (n != NULL)
				{
					#ifdef PRINT
					printf("On node %d?\n", n->unique_id); fflush(stdout);
					#endif
					
					/* 2.1. A = Get the earliest available time from the number of cores required by the job. 
					 * It's used in case of a tie on the locality, as well as to look at the intervals of
					 * available data at the predicted start time. */
					earliest_available_time = n->cores[j->cores - 1]->available_time; /* -1 because tab start at 0 */
					if (earliest_available_time < t) /* A core can't be available before t. This happens when a node is idling. */				
					{
						earliest_available_time = t;
					}
					
					#ifdef PRINT
					printf("EAT is: %d.\n", earliest_available_time); fflush(stdout);
					#endif
					
					if (min_score == -1 || earliest_available_time < min_score)
					{						
						/* 2.2. Compute the time to load all data. For this look at the data that will be available at the earliest available time of the node. */
						if (j->data == 0)
						{
							time_to_load_file = 0;
						}
						else
						{
							time_to_load_file = is_my_file_on_node_at_certain_time_and_transfer_time(earliest_available_time, n, t, j->data, j->data_size, &is_being_loaded); /* Use the intervals in each data to get this info. */
						}
						
						#ifdef PRINT
						printf("Time to load file: %f. Is being loaded? %d.\n", time_to_load_file, is_being_loaded); fflush(stdout);
						#endif
						
						/* To test with 1 1 instead of EFT. To remove. J'ai testé, c'est pas ça qui explique lesm oins bonnes perf que fcfs score adaptative multiplier 500 500*/
						//~ if (min_score == -1 || earliest_available_time + time_to_load_file < min_score)
						//~ {
							/* To test with 1 1 instead of EFT. To remove. */
							//~ time_to_reload_evicted_files = time_to_reload_percentage_of_files_ended_at_certain_time(earliest_available_time, n, j->data, (float) j->cores/20);
							
						
								/* Compute node's score. */
								score = earliest_available_time + time_to_load_file;
								
								/* To test with 1 1 instead of EFT. To remove. */
								//~ score = earliest_available_time + time_to_load_file + time_to_reload_evicted_files;
																
								#ifdef PRINT	
								printf("Score for job %d is %d with node %d.\n", j->unique_id, score, n->unique_id); fflush(stdout);
								#endif
													
								/* 2.6. Get minimum score/ */
								if (min_score == -1)
								{
									min_time = earliest_available_time;
									min_score = score;
									j->node_used = n;
									choosen_time_to_load_file = time_to_load_file;
								}
								else if (min_score > score)
								{
									min_time = earliest_available_time;
									min_score = score;
									j->node_used = n;
									choosen_time_to_load_file = time_to_load_file;
								}
						//~ }
					}
					n = n->next;
				}
			}
			
			j->transfer_time = choosen_time_to_load_file;
					
			/* Get start time and update available times of the cores. */
			j->start_time = min_time;
			j->end_time = min_time + j->walltime;
			
			for (int k = 0; k < j->cores; k++)
			{
				j->cores_used[k] = j->node_used->cores[k]->unique_id;
				if (j->node_used->cores[k]->available_time <= t)
				{
					nb_non_available_cores += 1;
				}
				j->node_used->cores[k]->available_time = min_time + j->walltime;
				
				/* Maybe I need job queue or not not sure. TODO. */
			}

			/* Need to add here intervals for current scheduling. */
			//~ found = false;
			//~ struct Data* d = j->node_used->data->head;
			//~ while (d != NULL)
			//~ {
				//~ if (d->unique_id == j->data)
				//~ {
					//~ found = true;
					//~ create_and_insert_tail_interval_list(d->intervals, j->start_time);
					//~ create_and_insert_tail_interval_list(d->intervals, j->start_time + j->transfer_time);
					//~ create_and_insert_tail_interval_list(d->intervals, j->end_time);
					//~ break;
				//~ }
				//~ d = d->next;
			//~ }
			
			//~ if (found == false)
			//~ {
				//~ #ifdef PRINT
				//~ printf("Need to create a data and intervals for the node %d data %d.\n", j->node_used->unique_id, j->data); fflush(stdout);
				//~ #endif
				
				//~ /* Create a class Data for this node. */
				//~ struct Data* new = (struct Data*) malloc(sizeof(struct Data));
				//~ new->next = NULL;
				//~ new->unique_id = j->data;
				//~ new->start_time = -1;
				//~ new->end_time = -1;
				//~ new->nb_task_using_it = 0;
				//~ new->intervals = (struct Interval_List*) malloc(sizeof(struct Interval_List));
				//~ new->intervals->head = NULL;
				//~ new->intervals->tail = NULL;
				//~ create_and_insert_tail_interval_list(new->intervals, j->start_time);
				//~ create_and_insert_tail_interval_list(new->intervals, j->start_time + j->transfer_time);
				//~ create_and_insert_tail_interval_list(new->intervals, j->end_time);
				//~ new->size = j->data_size;
				//~ insert_tail_data_list(j->node_used->data, new);
			//~ }	
			
			//~ #ifdef PRINT
			//~ printf("After add interval are:\n"); fflush(stdout);
			//~ print_data_intervals(head_node, t);
			//~ #endif
			
			/* Need to sort cores after each schedule of a job. */
			sort_cores_by_available_time_in_specific_node(j->node_used);
										
			//~ #ifdef PRINT
			//~ print_decision_in_scheduler(j);
			//~ #endif
						
			/* Insert in start times. */
			insert_next_time_in_sorted_list(start_times, j->start_time);
						
			//~ j = j->next;
		//~ }				
		//~ else
		//~ {
			//~ #ifdef PRINT
			//~ printf("No more available cores.\n"); fflush(stdout);
			//~ #endif
			
			//~ /* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			//~ while (j != NULL)
			//~ {
				//~ j->start_time = -1;
				//~ j = j->next;
			//~ }

			//~ break;
		//~ }
	//~ }
	return nb_non_available_cores;
}

/** Return the mean flow on all scheduled tasks. Does not actually schedule the
 *  tasks and uses a fake set of nodes. 
 *  If break_condition_if_not_started_at_t is set to 1, then return -1 when a job can't be scheduled at t. 
 *  Not used for the calssic Flow_adaptation_scheduler and oly implemented in fake EFT scheduler. **/
double fake_eft_scheduler(struct Job* head_job, struct Node_List** head_node, int t, int break_condition_if_not_started_at_t)
{
	#ifdef PRINT
	printf("\nFAKE EFT\n");
	#endif
	
	double total_flow = 0;
	//~ int nb_non_available_cores = get_nb_non_available_cores(node_list, t);
	int min_score = -1;
	int i = 0;
	int min_between_delay_and_walltime = 0;
	int earliest_available_time = 0;
	// int // first_node_size_to_choose_from = 0;
	// int // last_node_size_to_choose_from = 0;
	float time_to_load_file = 0;
	bool is_being_loaded = false;
	int score = 0;
	int min_time = 0;
	int choosen_time_to_load_file = 0;
	bool found = false;	
	double nb_jobs_scheduled = 0;			
	/* Get intervals of data. */ 
	get_current_intervals(head_node, t);
	
	//~ #ifdef PRINT
	//~ print_data_intervals(head_node, t);
	//~ #endif

	/* 1. Loop on available jobs. */
	struct Job* j = head_job;
	struct Node *node_used = (struct Node*) malloc(sizeof(struct Node));	
	while (j != NULL)
	{
		//~ if (nb_non_available_cores < nb_cores)
		//~ {
			//~ #ifdef PRINT
			//~ printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);			
			//~ printf("\nNeed to schedule job %d using file %d. T = %d\n", j->unique_id, j->data, t); fflush(stdout);
			//~ #endif
			
			/* 2. Choose a node. */		
			/* Reset some values. */					
			min_score = -1;
			earliest_available_time = 0;
			is_being_loaded = false;
			node_used = NULL;
			
			/* In which node size I can pick. */
			// get_node_size_to_choose_from(j->index_node_list, &first_node_size_to_choose_from, &last_node_size_to_choose_from);
			//~ if (j->index_node_list == 0)
			//~ {
				//~ // first_node_size_to_choose_from = 0;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 1)
			//~ {
				//~ first_node_size_to_choose_from = 1;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 2)
			//~ {
				//~ first_node_size_to_choose_from = 2;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else
			//~ {
				//~ printf("Error index value in schedule_job_on_earliest_available_cores.\n");  fflush(stdout);
				//~ exit(EXIT_FAILURE);
			//~ }
						
			for (i = first_node_size_to_choose_from; i <= last_node_size_to_choose_from; i++)
			{
				struct Node* n = head_node[i]->head;
				while (n != NULL)
				{
					//~ #ifdef PRINT
					//~ printf("On node %d?\n", n->unique_id); fflush(stdout);
					//~ #endif
					
					/* 2.1. A = Get the earliest available time from the number of cores required by the job. 
					 * It's used in case of a tie on the locality, as well as to look at the intervals of
					 * available data at the predicted start time. */
					earliest_available_time = n->cores[j->cores - 1]->available_time; /* -1 because tab start at 0 */
					if (earliest_available_time < t) /* A core can't be available before t. This happens when a node is idling. */				
					{
						earliest_available_time = t;
					}
					
					//~ #ifdef PRINT
					//~ printf("EAT is: %d.\n", earliest_available_time); fflush(stdout);
					//~ #endif
					
					if (min_score == -1 || earliest_available_time < min_score)
					{						
						/* 2.2. Compute the time to load all data. For this look at the data that will be available at the earliest available time of the node. */
						if (j->data == 0)
						{
							time_to_load_file = 0;
						}
						else
						{
							time_to_load_file = is_my_file_on_node_at_certain_time_and_transfer_time(earliest_available_time, n, t, j->data, j->data_size, &is_being_loaded); /* Use the intervals in each data to get this info. */
						}
						
						//~ #ifdef PRINT
						//~ printf("Time to load file: %f. Is being loaded? %d.\n", time_to_load_file, is_being_loaded); fflush(stdout);
						//~ #endif							
						
						/* Compute node's score. */
						score = earliest_available_time + time_to_load_file;
								
						//~ #ifdef PRINT	
						//~ printf("Score for job %d is %d with node %d.\n", j->unique_id, score, n->unique_id); fflush(stdout);
						//~ #endif
													
						/* 2.6. Get minimum score/ */
						if (min_score == -1)
						{
							min_time = earliest_available_time;
							min_score = score;
							node_used = n;
							choosen_time_to_load_file = time_to_load_file;
						}
						else if (min_score > score)
						{
							min_time = earliest_available_time;
							min_score = score;
							node_used = n;
							choosen_time_to_load_file = time_to_load_file;
						}
					}
					n = n->next;
				}
			}
			
			/* Here for the test EFT else do score scheduler, not flow adaptation. */
			if (break_condition_if_not_started_at_t == 1 && min_time != t)
			{
				return -1;
			}
			
			if (j->delay + choosen_time_to_load_file < j->walltime)
			{
				min_between_delay_and_walltime = j->delay + choosen_time_to_load_file;
			}
			else
			{
				min_between_delay_and_walltime = j->walltime;
			}
			
			/* Update total flow that we return at the end. */
			total_flow += min_time + min_between_delay_and_walltime - j->subtime;
			nb_jobs_scheduled += 1;
			//~ j->transfer_time = choosen_time_to_load_file;
					
			/* Get start time and update available times of the cores. */
			//~ j->start_time = min_time;
			//~ j->end_time = min_time + j->walltime;
			
			for (int k = 0; k < j->cores; k++)
			{
				//~ j->cores_used[k] = j->node_used->cores[k]->unique_id;
				//~ if (j->node_used->cores[k]->available_time <= t)
				//~ {
					//~ nb_non_available_cores += 1;
				//~ }
				node_used->cores[k]->available_time = min_time + j->walltime;				
			}

			/* Need to add here intervals for current scheduling. */
			found = false;
			struct Data* d = node_used->data->head;
			while (d != NULL)
			{
				if (d->unique_id == j->data)
				{
					found = true;
					create_and_insert_tail_interval_list(d->intervals, min_time);
					create_and_insert_tail_interval_list(d->intervals, min_time + choosen_time_to_load_file);
					create_and_insert_tail_interval_list(d->intervals, min_time + j->walltime);
					break;
				}
				d = d->next;
			}
			//~ printf("1.\n"); fflush(stdout);
			if (found == false)
			{
				//~ #ifdef PRINT
				//~ printf("Need to create a data and intervals for the node %d data %d.\n", node_used->unique_id, j->data); fflush(stdout);
				//~ #endif
				
				/* Create a class Data for this node. */
				struct Data* new = (struct Data*) malloc(sizeof(struct Data));
				new->next = NULL;
				new->unique_id = j->data;
				new->start_time = -1;
				new->end_time = -1;
				
				#ifndef DATA_PERSISTENCE
				new->nb_task_using_it = 0;
				#endif
				
				new->intervals = (struct Interval_List*) malloc(sizeof(struct Interval_List));
				new->intervals->head = NULL;
				new->intervals->tail = NULL;
				create_and_insert_tail_interval_list(new->intervals, min_time);
				create_and_insert_tail_interval_list(new->intervals, min_time + choosen_time_to_load_file);
				create_and_insert_tail_interval_list(new->intervals, min_time + j->walltime);
				new->size = j->data_size;
				insert_tail_data_list(node_used->data, new);
			}
			//~ printf("2.\n"); fflush(stdout);
			//~ #ifdef PRINT
			//~ printf("After add interval are:\n"); fflush(stdout);
			//~ print_data_intervals(head_node, t);
			//~ #endif
			
			/* Need to sort cores after each schedule of a job. */
			sort_cores_by_available_time_in_specific_node(node_used);

			/* Insert in start times. */
			//~ insert_next_time_in_sorted_list(start_times, j->start_time);
						
			j = j->next;
		//~ }				
		//~ else
		//~ {
			//~ #ifdef PRINT
			//~ printf("No more available cores.\n"); fflush(stdout);
			//~ #endif
			
			//~ /* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			//~ while (j != NULL)
			//~ {
				//~ j->start_time = -1;
				//~ j = j->next;
			//~ }

			//~ break;
		//~ }
	}
	return total_flow/nb_jobs_scheduled;
}

/* TODO : pas besoin de sort a chaque fois. Do I do it ? */
void fcfs_scheduler_backfill_big_nodes(struct Job* head_job, struct Node_List** head_node, int t, int backfill_big_node_mode, int total_queue_time, int nb_finished_jobs)
{
	#ifdef PRINT
	printf("Start fcfs_scheduler_backfill_big_nodes. Mode is: %d.\n", backfill_big_node_mode);
	#endif
	
	int nb_non_available_cores = get_nb_non_available_cores(node_list, t);
	struct Job* j = head_job;
	bool result = false;
	int i = 0;
	
	while (j != NULL)
	{
		if (nb_non_available_cores < nb_cores)
		{
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);
			#endif
			
			result = false;
			if (j->index_node_list != 2) /* TODO : pas sûr ça. */
			{
				i = j->index_node_list;
				while (result == false && i != 3)
				{
					#ifdef PRINT
					printf("Try to start immedialy (T=%d) job %d on node of category %d.\n", t, j->unique_id, i);
					#endif
					
					nb_non_available_cores = schedule_job_to_start_immediatly_on_specific_node_size(j, head_node[i], t, backfill_big_node_mode, total_queue_time, nb_finished_jobs, nb_non_available_cores, &result);
					i += 1;
				}
			}
			if (result == false)
			{
				#ifdef PRINT
				printf("Just schedule later job %d.\n", j->unique_id);
				#endif
				
				/* If we are here it means we failed to start the job anywhere or it's a job necessating the biggest nodes, so we need to schedule it now on it's corresponding node size (so the smallest one on which it fits). */
				nb_non_available_cores = schedule_job_on_earliest_available_cores_specific_sublist_node(j, head_node[j->index_node_list], t, nb_non_available_cores);
			}
			//~ exit(1);
			insert_next_time_in_sorted_list(start_times, j->start_time);
			
			j = j->next;
		}
		else
		{
			#ifdef PRINT
			printf("There are %d/%d available cores. Break.\n", nb_cores - nb_non_available_cores, nb_cores);
			#endif
			
			/* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			while (j != NULL)
			{
				j->start_time = -1;
				j = j->next;
			}
			
			break;
		}
	}
}
	
//~ void fcfs_scheduler_planned_area_filling(struct Job* head_job, struct Node_List** head_node, int t, long long Planned_Area[3][3])
void fcfs_scheduler_planned_area_filling(struct Job* head_job, struct Node_List** head_node, int t)
{
	// Care use long long for area!!
	#ifdef PRINT
	printf("Start of planned area filling.\n");
	printf("Planned areas are: [%lld, %lld, %lld] [%lld, %lld, %lld] [%lld, %lld, %lld]\n", Planned_Area[0][0], Planned_Area[0][1], Planned_Area[0][2], Planned_Area[1][0], Planned_Area[1][1], Planned_Area[1][2], Planned_Area[2][0], Planned_Area[2][1], Planned_Area[2][2]);
	#endif
	
	long long Temp_Planned_Area[3][3];
	int i = 0;
	int k = 0;
	
	for (i = 0; i < 3; i++)
	{
		for (k = 0; k < 3; k++)
		{
			Temp_Planned_Area[i][k] = Planned_Area[i][k];
		}
	}
		
	int next_size = 0;
	long long Area_j;
	int EAT = 0;
	int min_time = -1;
	int nb_non_available_cores = get_nb_non_available_cores(node_list, t);
	struct Job* j = head_job;
	int choosen_size = 0;
	struct Node* choosen_node = NULL; /* TODO: malloc ? */
	while (j != NULL)
	{
		if (nb_non_available_cores < nb_cores)
		{
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);
			printf("Scheduling job %d.\n", j->unique_id);
			#endif
			
			min_time = -1;
			choosen_node = NULL;
			Area_j = j->cores*j->walltime;
			
			/* Get EAT on each node size. */
			for (next_size = j->index_node_list; next_size < 3; next_size++)
			{
				if (next_size == j->index_node_list || Temp_Planned_Area[next_size][j->index_node_list] - Area_j >= 0)
				{
					EAT = get_earliest_available_time_specific_sublist_node(j->cores, head_node[next_size], &choosen_node, t);
					
					#ifdef PRINT
					printf("EAT on node of size %d is %d.\n", next_size, EAT);
					#endif
					
					if (EAT == t)
					{
						#ifdef PRINT
						printf("EAT == t can break.\n");
						#endif
						
						min_time = EAT;
						choosen_size = next_size;
						j->node_used = choosen_node;
						break;
					}
					else if (EAT < min_time || min_time == -1)
					{
						min_time = EAT;
						choosen_size = next_size;
						j->node_used = choosen_node;
					}
				}
			}
			
			/* Schedule the job on said node size */
			/* Update infos on the job and on cores. */
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
				
				/* Maybe I need job queue or not not sure. TODO. */
				// copy_job_and_insert_tail_job_list(n->cores[i]->job_queue, j);
			}
			
			/* Reduced corresponding Planned_Area */
			//~ if (next_size != j->index_node_list)
			if (choosen_size > j->index_node_list)
			{
				/* TODO do temp ... and real in start jobs ... */
				Temp_Planned_Area[choosen_size][j->index_node_list] -= Area_j;
			}
				
			#ifdef PRINT
			print_decision_in_scheduler(j);
			#endif
			
			/* Need to sort cores after each schedule of a job. */
			sort_cores_by_available_time_in_specific_node(j->node_used);
		
			insert_next_time_in_sorted_list(start_times, j->start_time);
			
			j = j->next;
		}
		else
		{
			#ifdef PRINT
			printf("There are %d/%d available cores. Break.\n", nb_cores - nb_non_available_cores, nb_cores);
			#endif
			
			/* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			while (j != NULL)
			{
				j->start_time = -1;
				j = j->next;
			}
			
			break;
		}
	}
}

void fcfs_scheduler_ratio_area_filling(struct Job* head_job, struct Node_List** head_node, int t, float Ratio_Area[3][3])
{
	// Care use long long for area!!
	#ifdef PRINT
	printf("Start of ratio area filling.\n");
	printf("Ratio areas are: [%f, %f, %f] [%f, %f, %f] [%f, %f, %f]\n", Ratio_Area[0][0], Ratio_Area[0][1], Ratio_Area[0][2], Ratio_Area[1][0], Ratio_Area[1][1], Ratio_Area[1][2], Ratio_Area[2][0], Ratio_Area[2][1], Ratio_Area[2][2]);
	printf("Allocated areas are: [%lld, %lld, %lld] [%lld, %lld, %lld] [%lld, %lld, %lld]\n", Allocated_Area[0][0], Allocated_Area[0][1], Allocated_Area[0][2], Allocated_Area[1][0], Allocated_Area[1][1], Allocated_Area[1][2], Allocated_Area[2][0], Allocated_Area[2][1], Allocated_Area[2][2]);
	#endif
	
	long long Temp_Allocated_Area[3][3];
	int i = 0;
	int k = 0;
	
	for (i = 0; i < 3; i++)
	{
		for (k = 0; k < 3; k++)
		{
			Temp_Allocated_Area[i][k] = Allocated_Area[i][k];
		}
	}
	
	int next_size = 0;
	long long Area_j;
	int EAT = 0;
	int min_time = -1;
	int nb_non_available_cores = get_nb_non_available_cores(node_list, t);
	struct Job* j = head_job;
	int choosen_size = 0;
	struct Node* choosen_node = NULL; /* TODO: malloc ? */
	while (j != NULL)
	{
		if (nb_non_available_cores < nb_cores)
		{
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);
			printf("Scheduling job %d.\n", j->unique_id);			
			printf("Temp allocated areas are: [%lld, %lld, %lld] [%lld, %lld, %lld] [%lld, %lld, %lld]\n", Temp_Allocated_Area[0][0], Temp_Allocated_Area[0][1], Temp_Allocated_Area[0][2], Temp_Allocated_Area[1][0], Temp_Allocated_Area[1][1], Temp_Allocated_Area[1][2], Temp_Allocated_Area[2][0], Temp_Allocated_Area[2][1], Temp_Allocated_Area[2][2]);
			#endif
			
			min_time = -1;
			choosen_node = NULL;
			Area_j = j->cores*j->walltime;
			
			/* Get EAT on each node size. */
			for (next_size = j->index_node_list; next_size < 3; next_size++)
			{
				#ifdef PRINT
				printf("%f*%d - %lld - %lld >= 0 ?\n", Ratio_Area[next_size][j->index_node_list], t, Temp_Allocated_Area[next_size][j->index_node_list], Area_j);
				#endif
				
				if (next_size == j->index_node_list || Ratio_Area[next_size][j->index_node_list]*t - Temp_Allocated_Area[next_size][j->index_node_list] - Area_j >= 0)
				{
					EAT = get_earliest_available_time_specific_sublist_node(j->cores, head_node[next_size], &choosen_node, t);
					
					#ifdef PRINT
					printf("EAT on node of size %d is %d.\n", next_size, EAT);
					#endif
					
					if (EAT == t)
					{
						#ifdef PRINT
						printf("EAT == t can break.\n");
						#endif
						
						min_time = EAT;
						choosen_size = next_size;
						j->node_used = choosen_node;
						break;
					}
					else if (EAT < min_time || min_time == -1)
					{
						min_time = EAT;
						choosen_size = next_size;
						j->node_used = choosen_node;
					}
				}
			}
			
			/* Schedule the job on said node size */
			/* Update infos on the job and on cores. */
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
				
				/* Maybe I need job queue or not not sure. TODO. */
				// copy_job_and_insert_tail_job_list(n->cores[i]->job_queue, j);
			}
			
			/* increment temp allocated area */
			if (choosen_size > j->index_node_list)
			{
				Temp_Allocated_Area[choosen_size][j->index_node_list] += Area_j;
			}
				
			#ifdef PRINT
			print_decision_in_scheduler(j);
			#endif
			
			/* Need to sort cores after each schedule of a job. */
			sort_cores_by_available_time_in_specific_node(j->node_used);
		
			insert_next_time_in_sorted_list(start_times, j->start_time);
			
			j = j->next;
		}
		else
		{
			#ifdef PRINT
			printf("There are %d/%d available cores. Break.\n", nb_cores - nb_non_available_cores, nb_cores);
			#endif
			
			/* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			while (j != NULL)
			{
				j->start_time = -1;
				j = j->next;
			}
			
			break;
		}
	}
}

/** Différence avec fcfs with a score classique:
 * Je regarde le score de tous les noeuds pour calculer le percentile, donc pas de if pour voir si le score en cours à une chance d'être meilleur.
 * Tableau pour ranger les score de chaque noeud calculé
 * Calcul du 95th percentile
 * Récupérer le best score dans chaque catégorie de taille de noeuds.
 * Plus besoin de get le minimum score après chaque calcul de score
 **/
void fcfs_with_a_score_backfill_big_nodes_95th_percentile_scheduler(struct Job* head_job, struct Node_List** head_node, int t, int multiplier_file_to_load, int multiplier_file_evicted, int multiplier_nb_copy, int number_node_size_128_and_more, int number_node_size_256_and_more, int number_node_size_1024)
{
	int nb_non_available_cores = get_nb_non_available_cores(node_list, t);		
	int i = 0;
	//~ int min_score = -1;
	int earliest_available_time = 0;
	// int // first_node_size_to_choose_from = 0;
	// int // last_node_size_to_choose_from = 0;
	float time_to_load_file = 0;
	bool is_being_loaded = false;
	float time_to_reload_evicted_files = 0;
	int nb_copy_file_to_load = 0;
	int time_or_data_already_checked = 0;
	int score = 0;
	//~ int min_time = 0;
	bool found = false;
	
	//~ long long min_score_nodes[3];
	//~ int min_time_nodes[3];
	//~ struct Node* node_used_nodes[3];
	//~ int choosen_time_to_load_file_nodes[3];
	long long min_score = -1;
	int min_time = 0;
	struct Node* node_used = NULL; /* TODO pas utile a suppr je crois, on peut maj directement dans le job au moment du nouveau meilleur score */
	int choosen_time_to_load_file = 0;

	/* Get intervals of data. */ 
	get_current_intervals(head_node, t);
	
	#ifdef PRINT
	print_data_intervals(head_node, t);
	#endif
	
	#ifdef PRINT_SCORES_DATA
	FILE* f_fcfs_score = fopen("outputs/Scores_data.txt", "a");
	#endif
	
	/* --- Reduced complexity nb of copy --- */	
	struct Time_or_Data_Already_Checked_Nb_of_Copy_List* time_or_data_already_checked_nb_of_copy_list = (struct Time_or_Data_Already_Checked_Nb_of_Copy_List*) malloc(sizeof(struct Time_or_Data_Already_Checked_Nb_of_Copy_List));
	time_or_data_already_checked_nb_of_copy_list->head = NULL;

	/* 1. Loop on available jobs. */
	struct Job* j = head_job;
	while (j != NULL)
	{
		if (nb_non_available_cores < nb_cores)
		{
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores); fflush(stdout);	
			printf("\nNeed to schedule job %d using file %d.\n", j->unique_id, j->data); fflush(stdout);
			#endif
			
			/* 2. Choose a node. */		
			/* Reset some values. */					
			//~ min_score = -1;
			earliest_available_time = 0;
			// first_node_size_to_choose_from = 0;
			// last_node_size_to_choose_from = 0;
			is_being_loaded = false;
			time_to_reload_evicted_files = 0;
			nb_copy_file_to_load = 0;
			
			/* In which node size I can pick. */
			// get_node_size_to_choose_from(j->index_node_list, &first_node_size_to_choose_from, &last_node_size_to_choose_from);
			//~ if (j->index_node_list == 0)
			//~ {
				//~ // first_node_size_to_choose_from = 0;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 1)
			//~ {
				//~ first_node_size_to_choose_from = 1;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 2)
			//~ {
				//~ first_node_size_to_choose_from = 2;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else
			//~ {
				//~ printf("Error index value in schedule_job_on_earliest_available_cores.\n");  fflush(stdout);
				//~ exit(EXIT_FAILURE);
			//~ }
						
			/* --- Reduced complexity nb of copy --- */
			if (multiplier_nb_copy != 0)
			{
				time_or_data_already_checked = was_time_or_data_already_checked_for_nb_copy(j->data, time_or_data_already_checked_nb_of_copy_list);
			}
			
			/* Tab pour ranger tout les scores */
			int nb_nodes_evaluated = 0;
			min_score = -1;
			min_time = 0;
			node_used = NULL;
			choosen_time_to_load_file = 0;
			
			if (j->index_node_list == 0)
			{
				nb_nodes_evaluated = number_node_size_128_and_more;
			}
			else if (j->index_node_list == 1)
			{
				nb_nodes_evaluated = number_node_size_256_and_more;
			}
			else
			{
				nb_nodes_evaluated = number_node_size_1024;
			}
			
			long long* tab_scores_all_nodes = malloc(sizeof(long long)*nb_nodes_evaluated);
			for (i = 0; i < nb_nodes_evaluated; i++)
			{
				tab_scores_all_nodes[i] = -1;
			}
			
			/* Compteur score calculé actuel */
			int index_current_evaluated_node = 0;
						
			for (i = first_node_size_to_choose_from; i <= last_node_size_to_choose_from; i++)
			{
				/* Computing 95th percentile */
				//~ double result_percentile_computation = 0;
				double result_percentile_computation = -1;
				if (j->index_node_list != 2)
				{
					sort_tab_of_int_decreasing_order(tab_scores_all_nodes, nb_nodes_evaluated);
					//~ printf("Tab of scores after sort by decreasing order is: ");
					//~ print_tab_of_int(tab_scores_all_nodes, nb_nodes_evaluated);
						
					//~ double percentile = 95; /* TODO a changer */
					double percentile = 95; /* TODO a changer */
					double fractional_rank = 0;
					fractional_rank = (percentile/100.0)*(nb_nodes_evaluated);
					//~ printf("fractional_rank: %f = (%f/100.0)*(%d)\n", fractional_rank, percentile, nb_nodes_evaluated); fflush(stdout);
					int index = ceil(fractional_rank) - 1;
					//~ printf("Index of the tab for the 95yh percentile is %d.\n", index); fflush(stdout);
					result_percentile_computation = tab_scores_all_nodes[index];
					#ifdef PRINT
					printf("result_percentile_computation for job %d size %d: %f\n", j->unique_id, i, result_percentile_computation); fflush(stdout);
					#endif
				}
				
				if (result_percentile_computation != -1 && min_score <= result_percentile_computation)
				{
					#ifdef PRINT
					printf("Break earlier because my best score (%lld) will be in the 95th percentile!\n", min_score); fflush(stdout);
					#endif
					break;
				}
								
				struct Node* n = head_node[i]->head;
				while (n != NULL)
				{
					earliest_available_time = 0;
					time_to_load_file = 0;
					time_to_reload_evicted_files = 0;
					nb_copy_file_to_load = 0;
					
					#ifdef PRINT
					printf("On node %d?\n", n->unique_id); fflush(stdout);
					#endif
					
					/* 2.1. A = Get the earliest available time from the number of cores required by the job and add it to the score. */
					earliest_available_time = n->cores[j->cores - 1]->available_time; /* -1 because tab start at 0 */
					if (earliest_available_time < t) /* A core can't be available before t. This happens when a node is idling. */				
					{
						earliest_available_time = t;
					}
					
					#ifdef PRINT
					printf("A: EAT is: %d.\n", earliest_available_time); fflush(stdout);
					#endif
					
					//~ if (min_score == -1 || (earliest_available_time < result_percentile_computation && j->index_node_list != 2) || (earliest_available_time < min_score && j->index_node_list == 2))
					if (min_score == -1 || earliest_available_time < min_score)
					{
						/* 2.2. B = Compute the time to load all data. For this look at the data that will be available at the earliest available time of the node. */
						if (j->data == 0)
						{
							time_to_load_file = 0;
						}
						else
						{
							time_to_load_file = is_my_file_on_node_at_certain_time_and_transfer_time(earliest_available_time, n, t, j->data, j->data_size, &is_being_loaded); /* Use the intervals in each data to get this info. */
						}
						
						#ifdef PRINT
						printf("B: Time to load file: %f. Is being loaded? %d.\n", time_to_load_file, is_being_loaded); fflush(stdout);
						#endif
											
						//~ if (min_score == -1 || (earliest_available_time + multiplier_file_to_load*time_to_load_file < result_percentile_computation && j->index_node_list != 2) || (earliest_available_time + multiplier_file_to_load*time_to_load_file < min_score && j->index_node_list == 2))
						if (min_score == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file < min_score)
						{
							/* 2.5. Get the amount of files that will be lost because of this load by computing the amount of data that end at the earliest time only on the supposely choosen cores, excluding current file of course. */
							if (multiplier_file_evicted == 0)
							{
								time_to_reload_evicted_files = 0;
							}
							else
							{
								time_to_reload_evicted_files = time_to_reload_percentage_of_files_ended_at_certain_time(earliest_available_time, n, j->data, (float) j->cores/20);
							}
							
							#ifdef PRINT
							printf("C: Time to reload evicted files %f.\n", time_to_reload_evicted_files); fflush(stdout);
							#endif
							
							//~ if (min_score == -1 || (earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files < result_percentile_computation && j->index_node_list != 2) || (earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files < min_score && j->index_node_list == 2))
							if (min_score == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files < min_score)
							{
								/* 2.5bis Get number of copy of the file we want to load on other nodes (if you need to load a file that is) at the time that is predicted to be used. So if a file is already loaded on a lot of node, you have a penalty if you want to load it on a new node. */
								if (time_to_load_file != 0 && is_being_loaded == false && multiplier_nb_copy != 0)
								{									
									/* --- Reduced complexity nb of copy --- */
									if (time_or_data_already_checked == -1)
									{
										#ifdef PRINT
										printf("Need to compute nb of copy it was never done.\n");
										#endif
										nb_copy_file_to_load = get_nb_valid_copy_of_a_file(t, head_node, j->data);
										create_and_insert_head_time_or_data_already_checked_nb_of_copy_list(time_or_data_already_checked_nb_of_copy_list, j->data, nb_copy_file_to_load);
										time_or_data_already_checked = nb_copy_file_to_load;
										#ifdef PRINT
										printf("Compute nb of copy done, it's %d.\n", nb_copy_file_to_load);
										#endif
									}
									else
									{
										nb_copy_file_to_load = time_or_data_already_checked;
										#ifdef PRINT
										printf("Already done for job %d at time %d so nb of copy is %d.\n", j->unique_id, t, nb_copy_file_to_load);
										#endif
									}
								}
								else
								{
									nb_copy_file_to_load = 0;
								}
								
								#ifdef PRINT
								printf("Nb of copy for data %d at time %d on node %d is %d.\n", j->data, earliest_available_time, n->unique_id, nb_copy_file_to_load); fflush(stdout);
								#endif
								
								/* Compute node's score. */
								score = earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files + nb_copy_file_to_load*time_to_load_file*multiplier_nb_copy;
								
								#ifdef PRINT		
								printf("Score for job %d is %d (EAT: %d + TL %f + TRL %f +NCP %f) with node %d.\n", j->unique_id, score, earliest_available_time, multiplier_file_to_load*time_to_load_file, multiplier_file_evicted*time_to_reload_evicted_files, nb_copy_file_to_load*time_to_load_file*multiplier_nb_copy, n->unique_id); fflush(stdout);
								#endif
													
								//~ /* 2.6. Get minimum score/ */
								/* update by category */
								if (score < min_score || min_score == -1)
								{
									#ifdef PRINT
									printf("New min score: %d.\n", score);
									#endif
									min_score = score;
									min_time = earliest_available_time;
									node_used = n;
									choosen_time_to_load_file = time_to_load_file;
								}
							}
						}
					}
					
					#ifdef PRINT_SCORES_DATA
					fprintf(f_fcfs_score, "Node: %d EAT: %d C: %f CxX: %f Score: %f\n", n->unique_id, earliest_available_time, time_to_reload_evicted_files, time_to_reload_evicted_files*multiplier_file_evicted, earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files);
					#endif
					
					/* Add score in tab with all scores */
					score = earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files + nb_copy_file_to_load*time_to_load_file*multiplier_nb_copy;
					#ifdef PRINT
					printf("Adding %d to the score tab.\n", score);
					#endif
					tab_scores_all_nodes[index_current_evaluated_node] = score;
					index_current_evaluated_node += 1;
					
					n = n->next;
				}

			}
			free(tab_scores_all_nodes); /* ? */
			
			j->node_used = node_used;
			
			/* Updating values for the job once the node was choosen. */
			j->transfer_time = choosen_time_to_load_file;
					
			/* Get start time and update available times of the cores. */
			j->start_time = min_time;
			j->end_time = min_time + j->walltime;
			
			for (int k = 0; k < j->cores; k++)
			{
				j->cores_used[k] = j->node_used->cores[k]->unique_id;
				if (j->node_used->cores[k]->available_time <= t)
				{
					nb_non_available_cores += 1;
				}
				j->node_used->cores[k]->available_time = min_time + j->walltime;
				
				/* Maybe I need job queue or not not sure. TODO. */
			}

			/* Need to add here intervals for current scheduling. */
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
				#ifdef PRINT
				printf("Need to create a data and intervals for the node %d data %d.\n", j->node_used->unique_id, j->data); fflush(stdout);
				#endif
				
				/* Create a class Data for this node. */
				struct Data* new = (struct Data*) malloc(sizeof(struct Data));
				new->next = NULL;
				new->unique_id = j->data;
				new->start_time = -1;
				new->end_time = -1;
				
				#ifndef DATA_PERSISTENCE
				new->nb_task_using_it = 0;
				#endif
				
				new->intervals = (struct Interval_List*) malloc(sizeof(struct Interval_List));
				new->intervals->head = NULL;
				new->intervals->tail = NULL;
				create_and_insert_tail_interval_list(new->intervals, j->start_time);
				create_and_insert_tail_interval_list(new->intervals, j->start_time + j->transfer_time);
				create_and_insert_tail_interval_list(new->intervals, j->end_time);
				new->size = j->data_size;
				insert_tail_data_list(j->node_used->data, new);
			}		
			
			#ifdef PRINT
			printf("After add interval are:\n"); fflush(stdout);
			print_data_intervals(head_node, t);
			#endif
			
			/* Need to sort cores after each schedule of a job. */
			sort_cores_by_available_time_in_specific_node(j->node_used);
										
			#ifdef PRINT
			print_decision_in_scheduler(j);
			#endif
						
			/* Insert in start times. */
			insert_next_time_in_sorted_list(start_times, j->start_time);
			
			/* --- Normal complexity nb of copy --- */
			/* Increment nb of copy for current file if we scheduled at time t the current job. */
			if (multiplier_nb_copy != 0 && j->start_time == t)
			{
				//~ printf("Need to increment for job %d Multi is %d.\n", j->unique_id, multiplier_nb_copy); fflush(stdout);
				increment_time_or_data_nb_of_copy_specific_time_or_data(time_or_data_already_checked_nb_of_copy_list, j->data);
				//~ printf("Increment ok for job %d.\n", j->unique_id); fflush(stdout);
			}
			
			j = j->next;
		}				
		else
		{
			#ifdef PRINT
			printf("No more available cores.\n"); fflush(stdout);
			#endif
			
			/* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			while (j != NULL)
			{
				j->start_time = -1;
				j = j->next;
			}
			
			break;
		}
	}
	
	/* --- Reduced complexity nb of copy --- */
	/* Free time already checked. */
	if (multiplier_nb_copy != 0)
	{
		free_time_or_data_already_checked_nb_of_copy_linked_list(&time_or_data_already_checked_nb_of_copy_list->head);
	}

	#ifdef PRINT_SCORES_DATA
	fclose(f_fcfs_score);
	#endif
}

/** What is unique here:
 * Tab of min score for each node size
 * Tab of min time used
 * Tab of node used created
 * Tab of choosen time to load file
 */
void fcfs_with_a_score_backfill_big_nodes_weighted_random_scheduler(struct Job* head_job, struct Node_List** head_node, int t, int multiplier_file_to_load, int multiplier_file_evicted, int multiplier_nb_copy)
{
	int nb_non_available_cores = get_nb_non_available_cores(node_list, t);		
	int i = 0;
	int random = 0;
	int node_size_choosen = 0;
	long long sum_best_scores = 0;
	long long* tab_min_score = malloc(sizeof(long long)*3);
	tab_min_score[0] = -1;
	tab_min_score[1] = -1;
	tab_min_score[2] = -1;
	int earliest_available_time = 0;
	// int // first_node_size_to_choose_from = 0;
	// int // last_node_size_to_choose_from = 0;
	float time_to_load_file = 0;
	bool is_being_loaded = false;
	float time_to_reload_evicted_files = 0;
	int nb_copy_file_to_load = 0;
	int time_or_data_already_checked = 0;
	long long score = 0;
	int* tab_min_time = malloc(sizeof(int)*3);
	tab_min_time[0] = 0;
	tab_min_time[1] = 0;
	tab_min_time[2] = 0;
	int* tab_choosen_time_to_load_file = malloc(sizeof(int)*3);
	tab_choosen_time_to_load_file[0] = 0;
	tab_choosen_time_to_load_file[1] = 0;
	tab_choosen_time_to_load_file[2] = 0;
	bool found = false;
	struct Node** tab_node_used = malloc(sizeof(struct Node)*3);
	tab_node_used[0] = NULL;
	tab_node_used[1] = NULL;
	tab_node_used[2] = NULL;
	
	/* Get intervals of data. */ 
	get_current_intervals(head_node, t);
	
	#ifdef PRINT
	print_data_intervals(head_node, t);
	#endif
	
	#ifdef PRINT_SCORES_DATA
	FILE* f_fcfs_score = fopen("outputs/Scores_data.txt", "a");
	#endif
	
	/* --- Reduced complexity nb of copy --- */	
	struct Time_or_Data_Already_Checked_Nb_of_Copy_List* time_or_data_already_checked_nb_of_copy_list = (struct Time_or_Data_Already_Checked_Nb_of_Copy_List*) malloc(sizeof(struct Time_or_Data_Already_Checked_Nb_of_Copy_List));
	time_or_data_already_checked_nb_of_copy_list->head = NULL;

	/* 1. Loop on available jobs. */
	struct Job* j = head_job;
	while (j != NULL)
	{
		if (nb_non_available_cores < nb_cores)
		{
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);			
			printf("\nNeed to schedule job %d using file %d category %d.\n", j->unique_id, j->data, j->index_node_list); fflush(stdout);
			#endif
			
			/* 2. Choose a node. */		
			/* Reset some values. */					
			tab_min_score[0] = -1;
			tab_min_score[1] = -1;
			tab_min_score[2] = -1;
			tab_min_time[0] = 0;
			tab_min_time[1] = 0;
			tab_min_time[2] = 0;
			tab_choosen_time_to_load_file[0] = 0;
			tab_choosen_time_to_load_file[1] = 0;
			tab_choosen_time_to_load_file[2] = 0;
			earliest_available_time = 0;
			// first_node_size_to_choose_from = 0;
			// last_node_size_to_choose_from = 0;
			is_being_loaded = false;
			time_to_reload_evicted_files = 0;
			nb_copy_file_to_load = 0;
			tab_node_used[0] = NULL;
			tab_node_used[1] = NULL;
			tab_node_used[2] = NULL;
					
			/* In which node size I can pick. */
			// get_node_size_to_choose_from(j->index_node_list, &first_node_size_to_choose_from, &last_node_size_to_choose_from);
			//~ if (j->index_node_list == 0)
			//~ {
				//~ // first_node_size_to_choose_from = 0;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 1)
			//~ {
				//~ first_node_size_to_choose_from = 1;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 2)
			//~ {
				//~ first_node_size_to_choose_from = 2;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else
			//~ {
				//~ printf("Error index value in schedule_job_on_earliest_available_cores.\n");  fflush(stdout);
				//~ exit(EXIT_FAILURE);
			//~ }			
						
			/* --- Reduced complexity nb of copy --- */
			if (multiplier_nb_copy != 0)
			{
				time_or_data_already_checked = was_time_or_data_already_checked_for_nb_copy(j->data, time_or_data_already_checked_nb_of_copy_list);
			}

			for (i = first_node_size_to_choose_from; i <= last_node_size_to_choose_from; i++)
			{
				struct Node* n = head_node[i]->head;
				while (n != NULL)
				{
					#ifdef PRINT
					printf("On node %d?\n", n->unique_id); fflush(stdout);
					#endif
					
					/* 2.1. A = Get the earliest available time from the number of cores required by the job and add it to the score. */
					earliest_available_time = n->cores[j->cores - 1]->available_time; /* -1 because tab start at 0 */
					if (earliest_available_time < t) /* A core can't be available before t. This happens when a node is idling. */				
					{
						earliest_available_time = t;
					}
					
					#ifdef PRINT
					printf("A: EAT is: %d.\n", earliest_available_time); fflush(stdout);
					#endif
					
					if (tab_min_score[i] == -1 || earliest_available_time < tab_min_score[i])
					{
						/* 2.2. B = Compute the time to load all data. For this look at the data that will be available at the earliest available time of the node. */
						if (j->data == 0)
						{
							time_to_load_file = 0;
						}
						else
						{
							time_to_load_file = is_my_file_on_node_at_certain_time_and_transfer_time(earliest_available_time, n, t, j->data, j->data_size, &is_being_loaded); /* Use the intervals in each data to get this info. */
						}
						
						#ifdef PRINT
						printf("B: Time to load file: %f. Is being loaded? %d.\n", time_to_load_file, is_being_loaded); fflush(stdout);
						#endif
											
						if (tab_min_score[i] == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file < tab_min_score[i])
						{
							/* 2.5. Get the amount of files that will be lost because of this load by computing the amount of data that end at the earliest time only on the supposely choosen cores, excluding current file of course. */
							if (multiplier_file_evicted == 0)
							{
								time_to_reload_evicted_files = 0;
							}
							else
							{
								time_to_reload_evicted_files = time_to_reload_percentage_of_files_ended_at_certain_time(earliest_available_time, n, j->data, (float) j->cores/20);
							}
							
							#ifdef PRINT
							printf("C: Time to reload evicted files %f.\n", time_to_reload_evicted_files); fflush(stdout);
							#endif
							
							if (tab_min_score[i] == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files < tab_min_score[i])
							{
								/* 2.5bis Get number of copy of the file we want to load on other nodes (if you need to load a file that is) at the time that is predicted to be used. So if a file is already loaded on a lot of node, you have a penalty if you want to load it on a new node. */
								if (time_to_load_file != 0 && is_being_loaded == false && multiplier_nb_copy != 0)
								{							
									/* --- Reduced complexity nb of copy --- */
									if (time_or_data_already_checked == -1)
									{
										#ifdef PRINT
										printf("Need to compute nb of copy it was never done.\n");
										#endif
										nb_copy_file_to_load = get_nb_valid_copy_of_a_file(t, head_node, j->data);
										create_and_insert_head_time_or_data_already_checked_nb_of_copy_list(time_or_data_already_checked_nb_of_copy_list, j->data, nb_copy_file_to_load);
										time_or_data_already_checked = nb_copy_file_to_load;
										#ifdef PRINT
										printf("Compute nb of copy done, it's %d.\n", nb_copy_file_to_load);
										#endif
									}
									else
									{
										nb_copy_file_to_load = time_or_data_already_checked;
										#ifdef PRINT
										printf("Already done for job %d at time %d so nb of copy is %d.\n", j->unique_id, t, nb_copy_file_to_load);
										#endif
									}
								}
								else
								{
									nb_copy_file_to_load = 0;
								}
								
								#ifdef PRINT
								printf("Nb of copy for data %d at time %d on node %d is %d.\n", j->data, earliest_available_time, n->unique_id, nb_copy_file_to_load); fflush(stdout);
								#endif
								
								/* Compute node's score. */
								score = earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files + nb_copy_file_to_load*time_to_load_file*multiplier_nb_copy;
								
								#ifdef PRINT		
								printf("Score for job %d is %lld (EAT: %d + TL %f + TRL %f +NCP %f) with node %d.\n", j->unique_id, score, earliest_available_time, multiplier_file_to_load*time_to_load_file, multiplier_file_evicted*time_to_reload_evicted_files, nb_copy_file_to_load*time_to_load_file*multiplier_nb_copy, n->unique_id); fflush(stdout);
								#endif
													
								/* 2.6. Get minimum score/ */
								if (tab_min_score[i] == -1)
								{
									tab_min_time[i] = earliest_available_time;
									tab_min_score[i] = score;
									tab_node_used[i] = n;
									tab_choosen_time_to_load_file[i] = time_to_load_file;
								}
								else if (tab_min_score[i] > score)
								{
									tab_min_time[i] = earliest_available_time;
									tab_min_score[i] = score;
									tab_node_used[i] = n;
									tab_choosen_time_to_load_file[i] = time_to_load_file;
								}
							}
						}
					}
					
					#ifdef PRINT_SCORES_DATA
					fprintf(f_fcfs_score, "Node: %d EAT: %d C: %f CxX: %f Score: %f\n", n->unique_id, earliest_available_time, time_to_reload_evicted_files, time_to_reload_evicted_files*multiplier_file_evicted, earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files);
					#endif
					
					n = n->next;
				}
			}
			
			/* Choix aléatoire pondéré du meilleur noeud */
			sum_best_scores = 0;
			if (tab_min_score[0] != - 1)
			{
				sum_best_scores += tab_min_score[0];
			}
			if (tab_min_score[1] != - 1)
			{
				sum_best_scores += tab_min_score[1];
			}
			if (tab_min_score[2] != - 1)
			{
				sum_best_scores += tab_min_score[2];
			}
			
			#ifdef PRINT
			printf("Sum of best scores (ignoring -1): %lld = %lld + %lld + %lld.\n", sum_best_scores, tab_min_score[0], tab_min_score[1], tab_min_score[2]);
			#endif
			
			random = rand()%sum_best_scores;
			#ifdef PRINT
			printf("Random 1 = %d.\n", random);
			#endif 
			if (random < tab_min_score[0] || tab_min_score[0] == -1)
			{
				#ifdef PRINT
				printf("Eliminate size 0.\n");
				#endif
				sum_best_scores = 0;
				if (tab_min_score[1] != - 1)
				{
					sum_best_scores += tab_min_score[1];
				}
				if (tab_min_score[2] != - 1)
				{
					sum_best_scores += tab_min_score[2];
				}
				#ifdef PRINT
				printf("Sum of best scores (ignoring -1): %lld = %lld + %lld.\n", sum_best_scores, tab_min_score[1], tab_min_score[2]);
				#endif
				random = rand()%sum_best_scores;
				#ifdef PRINT
				printf("Random 2 = %d.\n", random);
				#endif 
				if (random < tab_min_score[1] || tab_min_score[1] == -1)
				{
					node_size_choosen = 2;
				}
				else
				{
					node_size_choosen = 1;
				}
			}
			else if (random < tab_min_score[0] + tab_min_score[1] || tab_min_score[1] == -1)
			{
				#ifdef PRINT
				printf("Eliminate size 1.\n");
				#endif
				sum_best_scores = 0;
				if (tab_min_score[0] != - 1)
				{
					sum_best_scores += tab_min_score[0];
				}
				if (tab_min_score[2] != - 1)
				{
					sum_best_scores += tab_min_score[2];
				}
				#ifdef PRINT
				printf("Sum of best scores (ignoring -1): %lld = %lld + %lld.\n", sum_best_scores, tab_min_score[0], tab_min_score[2]);
				#endif
				random = rand()%sum_best_scores;
				#ifdef PRINT
				printf("Random 2 = %d.\n", random);
				#endif
				if (random < tab_min_score[0] || tab_min_score[0] == -1)
				{
					node_size_choosen = 2;
				}
				else
				{
					node_size_choosen = 0;
				}
			}
			else
			{
				#ifdef PRINT
				printf("Eliminate size 2.\n");
				#endif
				sum_best_scores = 0;
				if (tab_min_score[0] != - 1)
				{
					sum_best_scores += tab_min_score[0];
				}
				if (tab_min_score[1] != - 1)
				{
					sum_best_scores += tab_min_score[1];
				}
				#ifdef PRINT
				printf("Sum of best scores (ignoring -1): %lld = %lld + %lld.\n", sum_best_scores, tab_min_score[0], tab_min_score[1]);
				#endif
				random = rand()%sum_best_scores;
				#ifdef PRINT
				printf("Random 2 = %d.\n", random);
				#endif
				if (random < tab_min_score[0] || tab_min_score[0] == -1)
				{
					node_size_choosen = 1;
				}
				else
				{
					node_size_choosen = 0;
				}
			}
			#ifdef PRINT
			printf("Choosen size is %d.\n", node_size_choosen);
			#endif
			
			/* Penser à update j->node_used içi car je ne le fais pas plus haut ans ce cas. */
			j->node_used = tab_node_used[node_size_choosen];
			
			j->transfer_time = tab_choosen_time_to_load_file[node_size_choosen];
					
			/* Get start time and update available times of the cores. */
			j->start_time = tab_min_time[node_size_choosen];
			j->end_time = tab_min_time[node_size_choosen] + j->walltime;
			
			for (int k = 0; k < j->cores; k++)
			{
				j->cores_used[k] = j->node_used->cores[k]->unique_id;
				if (j->node_used->cores[k]->available_time <= t)
				{
					nb_non_available_cores += 1;
				}
				j->node_used->cores[k]->available_time = tab_min_time[node_size_choosen] + j->walltime;
				
				/* Maybe I need job queue or not not sure. TODO. */
			}

			/* Need to add here intervals for current scheduling. */
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
				#ifdef PRINT
				printf("Need to create a data and intervals for the node %d data %d.\n", j->node_used->unique_id, j->data); fflush(stdout);
				#endif
				
				/* Create a class Data for this node. */
				struct Data* new = (struct Data*) malloc(sizeof(struct Data));
				new->next = NULL;
				new->unique_id = j->data;
				new->start_time = -1;
				new->end_time = -1;
				
				#ifndef DATA_PERSISTENCE
				new->nb_task_using_it = 0;
				#endif
				
				new->intervals = (struct Interval_List*) malloc(sizeof(struct Interval_List));
				new->intervals->head = NULL;
				new->intervals->tail = NULL;
				create_and_insert_tail_interval_list(new->intervals, j->start_time);
				create_and_insert_tail_interval_list(new->intervals, j->start_time + j->transfer_time);
				create_and_insert_tail_interval_list(new->intervals, j->end_time);
				new->size = j->data_size;
				insert_tail_data_list(j->node_used->data, new);
			}			
			
			#ifdef PRINT
			printf("After add interval are:\n"); fflush(stdout);
			print_data_intervals(head_node, t);
			#endif
			
			/* Need to sort cores after each schedule of a job. */
			sort_cores_by_available_time_in_specific_node(j->node_used);
										
			#ifdef PRINT
			print_decision_in_scheduler(j);
			#endif
						
			/* Insert in start times. */
			insert_next_time_in_sorted_list(start_times, j->start_time);
						
			/* --- Normal complexity nb of copy --- */
			/* Increment nb of copy for current file if we scheduled at time t the current job. */
			if (multiplier_nb_copy != 0 && j->start_time == t)
			{
				increment_time_or_data_nb_of_copy_specific_time_or_data(time_or_data_already_checked_nb_of_copy_list, j->data);
			}
			
			j = j->next;
		}				
		else
		{
			#ifdef PRINT
			printf("No more available cores.\n"); fflush(stdout);
			#endif
			
			/* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			while (j != NULL)
			{
				j->start_time = -1;
				j = j->next;
			}
			
			break;
		}
	}
	
	/* --- Reduced complexity nb of copy --- */
	/* Free time already checked. */
	if (multiplier_nb_copy != 0)
	{
		free_time_or_data_already_checked_nb_of_copy_linked_list(&time_or_data_already_checked_nb_of_copy_list->head);
	}
	
	free(tab_min_score);
	free(tab_min_time);
	free(tab_choosen_time_to_load_file);
	free(tab_node_used);

	#ifdef PRINT_SCORES_DATA
	fclose(f_fcfs_score);
	#endif
}

void fcfs_with_a_score_backfill_big_nodes_gain_loss_tradeoff_scheduler(struct Job* head_job, struct Node_List** head_node, int t, int multiplier_file_to_load, int multiplier_file_evicted, int multiplier_nb_copy)
{
	int nb_non_available_cores = get_nb_non_available_cores(node_list, t);		
	int i = 0;
	int best_loss_gain_tradeoff = 0;
	long long area_to_schedule_my_size = 0;
	long long area_to_schedule_next_size = 0;
	long long loss_my_size = 0;
	long long loss_next_size = 0;
	int node_size_choosen = 0;
	long long min_score = -1;
	int earliest_available_time = 0;
	// int // first_node_size_to_choose_from = 0;
	// int // last_node_size_to_choose_from = 0;
	float time_to_load_file = 0;
	bool is_being_loaded = false;
	float time_to_reload_evicted_files = 0;
	int nb_copy_file_to_load = 0;
	int time_or_data_already_checked = 0;
	long long score = 0;
	bool found = false;
	long long gain = 0;
	long long loss = 0;
	
	long long* tab_min_score = malloc(sizeof(int)*3);
	tab_min_score[0] = -1;
	tab_min_score[1] = -1;
	tab_min_score[2] = -1;
	int* tab_min_time = malloc(sizeof(int)*3);
	tab_min_time[0] = 0;
	tab_min_time[1] = 0;
	tab_min_time[2] = 0;
	int* tab_choosen_time_to_load_file = malloc(sizeof(int)*3);
	tab_choosen_time_to_load_file[0] = 0;
	tab_choosen_time_to_load_file[1] = 0;
	tab_choosen_time_to_load_file[2] = 0;
	struct Node** tab_node_used = malloc(sizeof(struct Node)*3);
	tab_node_used[0] = NULL;
	tab_node_used[1] = NULL;
	tab_node_used[2] = NULL;
	
	/* Get intervals of data. */ 
	get_current_intervals(head_node, t);
	
	#ifdef PRINT
	print_data_intervals(head_node, t);
	#endif
	
	#ifdef PRINT_SCORES_DATA
	FILE* f_fcfs_score = fopen("outputs/Scores_data.txt", "a");
	#endif
	
	/* --- Reduced complexity nb of copy --- */	
	struct Time_or_Data_Already_Checked_Nb_of_Copy_List* time_or_data_already_checked_nb_of_copy_list = (struct Time_or_Data_Already_Checked_Nb_of_Copy_List*) malloc(sizeof(struct Time_or_Data_Already_Checked_Nb_of_Copy_List));
	time_or_data_already_checked_nb_of_copy_list->head = NULL;

	/* 1. Loop on available jobs. */
	struct Job* j = head_job;
	while (j != NULL)
	{
		if (nb_non_available_cores < nb_cores)
		{
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores); fflush(stdout);	
			printf("\nNeed to schedule job %d using file %d category %d.\n", j->unique_id, j->data, j->index_node_list); fflush(stdout);
			#endif
			
			/* 2. Choose a node. */		
			/* Reset some values. */					
			min_score = -1;
			earliest_available_time = 0;
			// first_node_size_to_choose_from = 0;
			// last_node_size_to_choose_from = 0;
			is_being_loaded = false;
			time_to_reload_evicted_files = 0;
			nb_copy_file_to_load = 0;
			
			tab_min_score[0] = -1;
			tab_min_score[1] = -1;
			tab_min_score[2] = -1;
			tab_min_time[0] = 0;
			tab_min_time[1] = 0;
			tab_min_time[2] = 0;
			tab_choosen_time_to_load_file[0] = 0;
			tab_choosen_time_to_load_file[1] = 0;
			tab_choosen_time_to_load_file[2] = 0;
			tab_node_used[0] = NULL;
			tab_node_used[1] = NULL;
			tab_node_used[2] = NULL;
			
			/* In which node size I can pick. */
			// get_node_size_to_choose_from(j->index_node_list, &first_node_size_to_choose_from, &last_node_size_to_choose_from);
			//~ if (j->index_node_list == 0)
			//~ {
				//~ // first_node_size_to_choose_from = 0;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 1)
			//~ {
				//~ first_node_size_to_choose_from = 1;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 2)
			//~ {
				//~ first_node_size_to_choose_from = 2;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else
			//~ {
				//~ printf("Error index value in schedule_job_on_earliest_available_cores.\n");  fflush(stdout);
				//~ exit(EXIT_FAILURE);
			//~ }
									
			/* --- Reduced complexity nb of copy --- */
			if (multiplier_nb_copy != 0)
			{
				time_or_data_already_checked = was_time_or_data_already_checked_for_nb_copy(j->data, time_or_data_already_checked_nb_of_copy_list);
			}

			for (i = first_node_size_to_choose_from; i <= last_node_size_to_choose_from; i++)
			{
				struct Node* n = head_node[i]->head;
				while (n != NULL)
				{
					#ifdef PRINT
					printf("On node %d size %d?\n", n->unique_id, i); fflush(stdout);
					#endif
					
					/* 2.1. A = Get the earliest available time from the number of cores required by the job and add it to the score. */
					earliest_available_time = n->cores[j->cores - 1]->available_time; /* -1 because tab start at 0 */
					if (earliest_available_time < t) /* A core can't be available before t. This happens when a node is idling. */				
					{
						earliest_available_time = t;
					}
					
					#ifdef PRINT
					printf("A: EAT is: %d.\n", earliest_available_time); fflush(stdout);
					#endif
					
					if (min_score == -1 || earliest_available_time < min_score)
					{
						/* 2.2. B = Compute the time to load all data. For this look at the data that will be available at the earliest available time of the node. */
						if (j->data == 0)
						{
							time_to_load_file = 0;
						}
						else
						{
							time_to_load_file = is_my_file_on_node_at_certain_time_and_transfer_time(earliest_available_time, n, t, j->data, j->data_size, &is_being_loaded); /* Use the intervals in each data to get this info. */
						}
						
						#ifdef PRINT
						printf("B: Time to load file: %f. Is being loaded? %d.\n", time_to_load_file, is_being_loaded); fflush(stdout);
						#endif
											
						if (min_score == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file < min_score)
						{
							/* 2.5. Get the amount of files that will be lost because of this load by computing the amount of data that end at the earliest time only on the supposely choosen cores, excluding current file of course. */
							if (multiplier_file_evicted == 0)
							{
								time_to_reload_evicted_files = 0;
							}
							else
							{
								time_to_reload_evicted_files = time_to_reload_percentage_of_files_ended_at_certain_time(earliest_available_time, n, j->data, (float) j->cores/20);
							}
							
							#ifdef PRINT
							printf("C: Time to reload evicted files %f.\n", time_to_reload_evicted_files); fflush(stdout);
							#endif
							
							if (min_score == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files < min_score)
							{
								/* 2.5bis Get number of copy of the file we want to load on other nodes (if you need to load a file that is) at the time that is predicted to be used. So if a file is already loaded on a lot of node, you have a penalty if you want to load it on a new node. */
								if (time_to_load_file != 0 && is_being_loaded == false && multiplier_nb_copy != 0)
								{									
									/* --- Reduced complexity nb of copy --- */
									if (time_or_data_already_checked == -1)
									{
										#ifdef PRINT
										printf("Need to compute nb of copy it was never done.\n");
										#endif
										nb_copy_file_to_load = get_nb_valid_copy_of_a_file(t, head_node, j->data);
										create_and_insert_head_time_or_data_already_checked_nb_of_copy_list(time_or_data_already_checked_nb_of_copy_list, j->data, nb_copy_file_to_load);
										time_or_data_already_checked = nb_copy_file_to_load;
										#ifdef PRINT
										printf("Compute nb of copy done, it's %d.\n", nb_copy_file_to_load);
										#endif
									}
									else
									{
										nb_copy_file_to_load = time_or_data_already_checked;
										#ifdef PRINT
										printf("Already done for job %d at time %d so nb of copy is %d.\n", j->unique_id, t, nb_copy_file_to_load);
										#endif
									}
								}
								else
								{
									nb_copy_file_to_load = 0;
								}
								
								#ifdef PRINT
								printf("Nb of copy for data %d at time %d on node %d is %d.\n", j->data, earliest_available_time, n->unique_id, nb_copy_file_to_load); fflush(stdout);
								#endif
								
								/* Compute node's score. */
								score = earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files + nb_copy_file_to_load*time_to_load_file*multiplier_nb_copy;
																
								#ifdef PRINT		
								printf("Score for job %d is %lld (EAT: %d + TL %f + TRL %f +NCP %f) with node %d.\n", j->unique_id, score, earliest_available_time, multiplier_file_to_load*time_to_load_file, multiplier_file_evicted*time_to_reload_evicted_files, nb_copy_file_to_load*time_to_load_file*multiplier_nb_copy, n->unique_id); fflush(stdout);
								#endif
													
								/* 2.6. Get minimum score/ */
								if (min_score == -1)
								{
									tab_min_time[i] = earliest_available_time;
									tab_min_score[i] = score;
									tab_node_used[i] = n;
									tab_choosen_time_to_load_file[i] = time_to_load_file;
									min_score = score;
								}
								else if (min_score > score)
								{
									tab_min_time[i] = earliest_available_time;
									tab_min_score[i] = score;
									tab_node_used[i] = n;
									tab_choosen_time_to_load_file[i] = time_to_load_file;
									min_score = score;
								}
							}
						}
					}
					
					#ifdef PRINT_SCORES_DATA
					fprintf(f_fcfs_score, "Node: %d EAT: %d C: %f CxX: %f Score: %f\n", n->unique_id, earliest_available_time, time_to_reload_evicted_files, time_to_reload_evicted_files*multiplier_file_evicted, earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files);
					#endif
					
					n = n->next;
				}
			}
			
			/* Si le min c'est ma taille pas de soucis on schedule sur ma taille. Sinon je dois trouver le min et calculer le gain/loss.
			 * Dans le cas de la taille 2 on ne rentre pas dans le if naturellement. */
			node_size_choosen = j->index_node_list;
			if (min_score != tab_min_score[j->index_node_list])
			{
				#ifdef PRINT
				printf("min_score is not on my size!\n");
				#endif
				best_loss_gain_tradeoff = 0;
				
				/* Regardons la taille i */
				for (i = j->index_node_list + 1; i < 3; i++)
				{
					#ifdef PRINT
					printf("Try size %d.\n", i);
					#endif
					if (tab_min_score[j->index_node_list] > tab_min_score[i] && tab_min_score[i] != -1)
					{
						area_to_schedule_my_size = 0;
						area_to_schedule_next_size = 0;

						gain = tab_min_score[j->index_node_list] - tab_min_score[i];
						#ifdef PRINT
						printf("Size %d is better with gain = %lld.\n", i, gain);
						#endif
						/* Calculons le queue time supplémentaire si je schedule j sur sa taille x ainsi que le queue time supplémentaire si je schedule j sur la taille x + 1 */
						struct Job* j_to_simulate = j->next;
						while (j_to_simulate != NULL)
						{
							/* Let's look if I will delete a file, if yes and a future job use it I have to count this loading time. */
							//~ time_to_load_file = is_my_file_on_node_at_certain_time_and_transfer_time(earliest_available_time, n, t, j->data, j->data_size, &is_being_loaded);
							
							if (j_to_simulate->index_node_list == j->index_node_list) /* On size x */
							{
								#ifdef PRINT
								printf("Adding area %d*%d on my size for job %d.\n", j_to_simulate->walltime, j_to_simulate->cores, j_to_simulate->unique_id);
								#endif
								area_to_schedule_my_size += j_to_simulate->walltime*j_to_simulate->cores;
							}
							else if (j_to_simulate->index_node_list == i) /* On size x + 1 */
							{
								#ifdef PRINT
								printf("Adding area %d*%d on next size for job %d.\n", j_to_simulate->walltime, j_to_simulate->cores, j_to_simulate->unique_id);
								#endif
								area_to_schedule_next_size += j_to_simulate->walltime*j_to_simulate->cores;
							}
							j_to_simulate = j_to_simulate->next;
						}
						loss_my_size = (area_to_schedule_my_size + j->walltime*j->cores)/(number_node_size[j->index_node_list]*20) - area_to_schedule_my_size/(number_node_size[j->index_node_list]*20);
						loss_next_size = (area_to_schedule_next_size + j->walltime*j->cores)/(number_node_size[i]*20) - area_to_schedule_next_size/(number_node_size[i]*20);
						
						#ifdef PRINT
						printf("loss_my_size: %lld = (%lld + %d*%d)/(%d*20) - %lld/%d*20\n", loss_my_size, area_to_schedule_my_size, j->walltime, j->cores, number_node_size[j->index_node_list], area_to_schedule_my_size, number_node_size[j->index_node_list]);
						printf("loss_next_size: %lld = (%lld + %d*%d)/(%d*20) - %lld/%d*20\n", loss_next_size, area_to_schedule_next_size, j->walltime, j->cores, number_node_size[i], area_to_schedule_next_size, number_node_size[i]);
						#endif
						
						loss = loss_next_size - loss_my_size;
						#ifdef PRINT
						printf("loss is %lld (next size) - %lld (my size) = %lld.\n", loss_next_size, loss_my_size, loss);
						#endif
						if (loss < gain && best_loss_gain_tradeoff < gain - loss)
						{
							#ifdef PRINT
							printf("New best size %d!\n", i);
							#endif
							best_loss_gain_tradeoff = gain - loss;
							node_size_choosen = i;
						}
					}
				}
			}
			
			j->node_used = tab_node_used[node_size_choosen];
			j->transfer_time = tab_choosen_time_to_load_file[node_size_choosen];
					
			/* Get start time and update available times of the cores. */
			j->start_time = tab_min_time[node_size_choosen];
			j->end_time = tab_min_time[node_size_choosen] + j->walltime;
			
			for (int k = 0; k < j->cores; k++)
			{
				j->cores_used[k] = j->node_used->cores[k]->unique_id;
				if (j->node_used->cores[k]->available_time <= t)
				{
					nb_non_available_cores += 1;
				}
				j->node_used->cores[k]->available_time = tab_min_time[node_size_choosen] + j->walltime;
				
				/* Maybe I need job queue or not not sure. TODO. */
			}

			/* Need to add here intervals for current scheduling. */
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
				#ifdef PRINT
				printf("Need to create a data and intervals for the node %d data %d.\n", j->node_used->unique_id, j->data); fflush(stdout);
				#endif
				
				/* Create a class Data for this node. */
				struct Data* new = (struct Data*) malloc(sizeof(struct Data));
				new->next = NULL;
				new->unique_id = j->data;
				new->start_time = -1;
				new->end_time = -1;
				
				#ifndef DATA_PERSISTENCE
				new->nb_task_using_it = 0;
				#endif
				
				new->intervals = (struct Interval_List*) malloc(sizeof(struct Interval_List));
				new->intervals->head = NULL;
				new->intervals->tail = NULL;
				create_and_insert_tail_interval_list(new->intervals, j->start_time);
				create_and_insert_tail_interval_list(new->intervals, j->start_time + j->transfer_time);
				create_and_insert_tail_interval_list(new->intervals, j->end_time);
				new->size = j->data_size;
				insert_tail_data_list(j->node_used->data, new);
			}			
			
			#ifdef PRINT
			printf("After add interval are:\n"); fflush(stdout);
			print_data_intervals(head_node, t);
			#endif
			
			/* Need to sort cores after each schedule of a job. */
			sort_cores_by_available_time_in_specific_node(j->node_used);
										
			#ifdef PRINT
			print_decision_in_scheduler(j);
			#endif
						
			/* Insert in start times. */
			insert_next_time_in_sorted_list(start_times, j->start_time);
						
			/* --- Normal complexity nb of copy --- */
			/* Increment nb of copy for current file if we scheduled at time t the current job. */
			if (multiplier_nb_copy != 0 && j->start_time == t)
			{
				increment_time_or_data_nb_of_copy_specific_time_or_data(time_or_data_already_checked_nb_of_copy_list, j->data);
			}
			
			j = j->next;
		}				
		else
		{
			#ifdef PRINT
			printf("No more available cores.\n"); fflush(stdout);
			#endif
			
			/* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			while (j != NULL)
			{
				j->start_time = -1;
				j = j->next;
			}
			
			break;
		}
	}
	
	/* --- Reduced complexity nb of copy --- */
	/* Free time already checked. */
	if (multiplier_nb_copy != 0)
	{
		free_time_or_data_already_checked_nb_of_copy_linked_list(&time_or_data_already_checked_nb_of_copy_list->head);
	}
	
	free(tab_min_score);
	free(tab_min_time);
	free(tab_choosen_time_to_load_file);
	free(tab_node_used);

	#ifdef PRINT_SCORES_DATA
	fclose(f_fcfs_score);
	#endif
}

/* Add a malus on fcfs with a score on the index of the node you want to use depending on the allocated area you have. */
void fcfs_with_a_score_area_filling_scheduler(struct Job* head_job, struct Node_List** head_node, int t, int multiplier_file_to_load, int multiplier_file_evicted, int multiplier_nb_copy, int planned_or_ratio, float Ratio_Area[3][3])
{
	/* get area in a temp tab */
	long long Temp_Allocated_or_Planned_Area[3][3];
	int i = 0;
	int k = 0;
	
	for (i = 0; i < 3; i++)
	{
		for (k = 0; k < 3; k++)
		{
			if (planned_or_ratio == 1)
			{
				Temp_Allocated_or_Planned_Area[i][k] = Allocated_Area[i][k];
			}
			else
			{
				Temp_Allocated_or_Planned_Area[i][k] = Planned_Area[i][k];
			}
		}
	}	

	long long Area_j = 0;
	int nb_non_available_cores = get_nb_non_available_cores(node_list, t);		
	long long min_score = -1;
	int earliest_available_time = 0;
	// int // first_node_size_to_choose_from = 0;
	// int // last_node_size_to_choose_from = 0;
	float time_to_load_file = 0;
	bool is_being_loaded = false;
	float time_to_reload_evicted_files = 0;
	int nb_copy_file_to_load = 0;
	int time_or_data_already_checked = 0;
	long long score = 0;
	int min_time = 0;
	int choosen_time_to_load_file = 0;
	bool found = false;
	int choosen_size = 0;
	long long calcul_autorisation_upgrade = 0;
	
	/* Get intervals of data. */ 
	get_current_intervals(head_node, t);
	
	#ifdef PRINT
	print_data_intervals(head_node, t);
	#endif
	
	#ifdef PRINT_SCORES_DATA
	FILE* f_fcfs_score = fopen("outputs/Scores_data.txt", "a");
	#endif
	
	/* --- Reduced complexity nb of copy --- */	
	struct Time_or_Data_Already_Checked_Nb_of_Copy_List* time_or_data_already_checked_nb_of_copy_list = (struct Time_or_Data_Already_Checked_Nb_of_Copy_List*) malloc(sizeof(struct Time_or_Data_Already_Checked_Nb_of_Copy_List));
	time_or_data_already_checked_nb_of_copy_list->head = NULL;


	/* 1. Loop on available jobs. */
	struct Job* j = head_job;
	while (j != NULL)
	{
		if (nb_non_available_cores < nb_cores)
		{
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);			
			printf("\nNeed to schedule job %d using file %d.\n", j->unique_id, j->data); fflush(stdout);
			#endif
			
			/* 2. Choose a node. */		
			/* Reset some values. */					
			min_score = -1;
			earliest_available_time = 0;
			// first_node_size_to_choose_from = 0;
			// last_node_size_to_choose_from = 0;
			is_being_loaded = false;
			time_to_reload_evicted_files = 0;
			nb_copy_file_to_load = 0;
			
			/* In which node size I can pick. */
			// get_node_size_to_choose_from(j->index_node_list, &first_node_size_to_choose_from, &last_node_size_to_choose_from);
			//~ if (j->index_node_list == 0)
			//~ {
				//~ // first_node_size_to_choose_from = 0;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 1)
			//~ {
				//~ first_node_size_to_choose_from = 1;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 2)
			//~ {
				//~ first_node_size_to_choose_from = 2;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else
			//~ {
				//~ printf("Error index value in schedule_job_on_earliest_available_cores.\n");  fflush(stdout);
				//~ exit(EXIT_FAILURE);
			//~ }
			
			/* --- Reduced complexity nb of copy --- */
			if (multiplier_nb_copy != 0)
			{
				time_or_data_already_checked = was_time_or_data_already_checked_for_nb_copy(j->data, time_or_data_already_checked_nb_of_copy_list);
			}
			
			Area_j = j->cores*j->walltime;

			for (i = first_node_size_to_choose_from; i <= last_node_size_to_choose_from; i++)
			{
				if (i != j->index_node_list)
				{
					if (planned_or_ratio == 1)
					{
						calcul_autorisation_upgrade = Ratio_Area[i][j->index_node_list]*t - Temp_Allocated_or_Planned_Area[i][j->index_node_list] - Area_j;
					}
					else
					{
						calcul_autorisation_upgrade = Temp_Allocated_or_Planned_Area[i][j->index_node_list] - Area_j;
					}
					#ifdef PRINT
					printf("Calcul_autorisation_upgrade = %lld for size %d. Area was %lld.\n", calcul_autorisation_upgrade, i, Area_j);
					#endif
				}
				if (i == j->index_node_list || calcul_autorisation_upgrade >= 0)
				{
					struct Node* n = head_node[i]->head;
					while (n != NULL)
					{	
						#ifdef PRINT
						printf("On node %d?\n", n->unique_id); fflush(stdout);
						#endif
						
						/* 2.1. A = Get the earliest available time from the number of cores required by the job and add it to the score. */
						earliest_available_time = n->cores[j->cores - 1]->available_time; /* -1 because tab start at 0 */
						if (earliest_available_time < t) /* A core can't be available before t. This happens when a node is idling. */				
						{
							earliest_available_time = t;
						}
						
						#ifdef PRINT
						printf("A: EAT is: %d.\n", earliest_available_time); fflush(stdout);
						#endif
						
						if (min_score == -1 || earliest_available_time < min_score)
						{
							/* 2.2. B = Compute the time to load all data. For this look at the data that will be available at the earliest available time of the node. */
							if (j->data == 0)
							{
								time_to_load_file = 0;
							}
							else
							{
								time_to_load_file = is_my_file_on_node_at_certain_time_and_transfer_time(earliest_available_time, n, t, j->data, j->data_size, &is_being_loaded); /* Use the intervals in each data to get this info. */
							}
							
							#ifdef PRINT
							printf("B: Time to load file: %f. Is being loaded? %d.\n", time_to_load_file, is_being_loaded); fflush(stdout);
							#endif
												
							if (min_score == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file < min_score)
							{
								/* 2.5. Get the amount of files that will be lost because of this load by computing the amount of data that end at the earliest time only on the supposely choosen cores, excluding current file of course. */
								if (multiplier_file_evicted == 0)
								{
									time_to_reload_evicted_files = 0;
								}
								else
								{
									time_to_reload_evicted_files = time_to_reload_percentage_of_files_ended_at_certain_time(earliest_available_time, n, j->data, (float) j->cores/20);
								}
								
								#ifdef PRINT
								printf("C: Time to reload evicted files %f.\n", time_to_reload_evicted_files); fflush(stdout);
								#endif
								
								if (min_score == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files < min_score)
								{
									/* 2.5bis Get number of copy of the file we want to load on other nodes (if you need to load a file that is) at the time that is predicted to be used. So if a file is already loaded on a lot of node, you have a penalty if you want to load it on a new node. */
									if (time_to_load_file != 0 && is_being_loaded == false && multiplier_nb_copy != 0)
									{
										/* --- Reduced complexity nb of copy --- */
										if (time_or_data_already_checked == -1)
										{
											#ifdef PRINT
											printf("Need to compute nb of copy it was never done.\n");
											#endif
											nb_copy_file_to_load = get_nb_valid_copy_of_a_file(t, head_node, j->data);
											create_and_insert_head_time_or_data_already_checked_nb_of_copy_list(time_or_data_already_checked_nb_of_copy_list, j->data, nb_copy_file_to_load);
											time_or_data_already_checked = nb_copy_file_to_load;
											#ifdef PRINT
											printf("Compute nb of copy done, it's %d.\n", nb_copy_file_to_load);
											#endif
										}
										else
										{
											nb_copy_file_to_load = time_or_data_already_checked;
											#ifdef PRINT
											printf("Already done for job %d at time %d so nb of copy is %d.\n", j->unique_id, t, nb_copy_file_to_load);
											#endif
										}
									}
									else
									{
										nb_copy_file_to_load = 0;
									}
									
									#ifdef PRINT
									printf("Nb of copy for data %d at time %d on node %d is %d.\n", j->data, earliest_available_time, n->unique_id, nb_copy_file_to_load); fflush(stdout);
									#endif
									
									/* Compute node's score. */
									score = earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files + nb_copy_file_to_load*time_to_load_file*multiplier_nb_copy;
																	
									#ifdef PRINT		
									printf("Score for job %d is %lld (EAT: %d + TL %f + TRL %f +NCP %f) with node %d.\n", j->unique_id, score, earliest_available_time, multiplier_file_to_load*time_to_load_file, multiplier_file_evicted*time_to_reload_evicted_files, nb_copy_file_to_load*time_to_load_file*multiplier_nb_copy, n->unique_id); fflush(stdout);
									#endif
														
									/* 2.6. Get minimum score/ */
									if (min_score == -1)
									{
										min_time = earliest_available_time;
										min_score = score;
										j->node_used = n;
										choosen_time_to_load_file = time_to_load_file;
										choosen_size = i;
									}
									else if (min_score > score)
									{
										min_time = earliest_available_time;
										min_score = score;
										j->node_used = n;
										choosen_time_to_load_file = time_to_load_file;
										choosen_size = i;
									}
								}
							}
						}
						
						#ifdef PRINT_SCORES_DATA
						fprintf(f_fcfs_score, "Node: %d EAT: %d C: %f CxX: %f Score: %f\n", n->unique_id, earliest_available_time, time_to_reload_evicted_files, time_to_reload_evicted_files*multiplier_file_evicted, earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files);
						#endif
						
						n = n->next;
					}
				}
			}
			
			j->transfer_time = choosen_time_to_load_file;
					
			/* Get start time and update available times of the cores. */
			j->start_time = min_time;
			j->end_time = min_time + j->walltime;
			
			for (k = 0; k < j->cores; k++)
			{
				j->cores_used[k] = j->node_used->cores[k]->unique_id;
				if (j->node_used->cores[k]->available_time <= t)
				{
					nb_non_available_cores += 1;
				}
				j->node_used->cores[k]->available_time = min_time + j->walltime;
				
				/* Maybe I need job queue or not not sure. TODO. */
			}
			
			/* Reduced corresponding Planned_Area */
			if (choosen_size > j->index_node_list)
			{
				if (planned_or_ratio == 1)
				{
					//~ Temp_Allocated_or_Planned_Area[choosen_size][j->index_node_list] += Area_j;
				}
				else
				{
					//~ Temp_Allocated_or_Planned_Area[choosen_size][j->index_node_list] -= Area_j;
				}
			}

			/* Need to add here intervals for current scheduling. */
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
				#ifdef PRINT
				printf("Need to create a data and intervals for the node %d data %d.\n", j->node_used->unique_id, j->data); fflush(stdout);
				#endif
				
				/* Create a class Data for this node. */
				struct Data* new = (struct Data*) malloc(sizeof(struct Data));
				new->next = NULL;
				new->unique_id = j->data;
				new->start_time = -1;
				new->end_time = -1;
				
				#ifndef DATA_PERSISTENCE
				new->nb_task_using_it = 0;
				#endif
				
				new->intervals = (struct Interval_List*) malloc(sizeof(struct Interval_List));
				new->intervals->head = NULL;
				new->intervals->tail = NULL;
				create_and_insert_tail_interval_list(new->intervals, j->start_time);
				create_and_insert_tail_interval_list(new->intervals, j->start_time + j->transfer_time);
				create_and_insert_tail_interval_list(new->intervals, j->end_time);
				new->size = j->data_size;
				insert_tail_data_list(j->node_used->data, new);
			}			
			
			#ifdef PRINT
			printf("After add interval are:\n"); fflush(stdout);
			print_data_intervals(head_node, t);
			#endif
			
			/* Need to sort cores after each schedule of a job. */
			sort_cores_by_available_time_in_specific_node(j->node_used);
										
			#ifdef PRINT
			print_decision_in_scheduler(j);
			#endif
						
			/* Insert in start times. */
			insert_next_time_in_sorted_list(start_times, j->start_time);
						
			/* --- Normal complexity nb of copy --- */
			/* Increment nb of copy for current file if we scheduled at time t the current job. */
			if (multiplier_nb_copy != 0 && j->start_time == t)
			{
				increment_time_or_data_nb_of_copy_specific_time_or_data(time_or_data_already_checked_nb_of_copy_list, j->data);
			}
			
			j = j->next;
		}	
		else
		{
			#ifdef PRINT
			printf("No more available cores.\n"); fflush(stdout);
			#endif
			
			/* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			while (j != NULL)
			{
				j->start_time = -1;
				j = j->next;
			}
			
			break;
		}
	}
	
	/* --- Reduced complexity nb of copy --- */
	/* Free time already checked. */
	if (multiplier_nb_copy != 0)
	{
		free_time_or_data_already_checked_nb_of_copy_linked_list(&time_or_data_already_checked_nb_of_copy_list->head);
	}

	#ifdef PRINT_SCORES_DATA
	fclose(f_fcfs_score);
	#endif
}

void fcfs_with_a_score_area_factor_scheduler (struct Job* head_job, struct Node_List** head_node, int t, int multiplier_file_to_load, int multiplier_file_evicted, int multiplier_nb_copy, int multiplier_area_bigger_nodes, int division_by_planned_area)
{
	#ifdef PRINT
	printf("Start of fcfs with a score area factor.\n");
	printf("Planned areas are: [%lld, %lld, %lld] [%lld, %lld, %lld] [%lld, %lld, %lld]\n", Planned_Area[0][0], Planned_Area[0][1], Planned_Area[0][2], Planned_Area[1][0], Planned_Area[1][1], Planned_Area[1][2], Planned_Area[2][0], Planned_Area[2][1], Planned_Area[2][2]);
	#endif
	
	int i = 0;	
	int nb_non_available_cores = get_nb_non_available_cores(node_list, t);		
	long long min_score = -1;
	int earliest_available_time = 0;
	// int // first_node_size_to_choose_from = 0;
	// int // last_node_size_to_choose_from = 0;
	float time_to_load_file = 0;
	bool is_being_loaded = false;
	float time_to_reload_evicted_files = 0;
	int nb_copy_file_to_load = 0;
	int time_or_data_already_checked = 0;
	long long score = 0;
	int min_time = 0;
	int choosen_time_to_load_file = 0;
	bool found = false;
	float area_ratio_used = 0;
	
	/* Get intervals of data. */ 
	get_current_intervals(head_node, t);
	
	#ifdef PRINT
	print_data_intervals(head_node, t);
	#endif
	
	#ifdef PRINT_SCORES_DATA
	FILE* f_fcfs_score = fopen("outputs/Scores_data.txt", "a");
	#endif
	
	/* --- Reduced complexity nb of copy --- */	
	struct Time_or_Data_Already_Checked_Nb_of_Copy_List* time_or_data_already_checked_nb_of_copy_list = (struct Time_or_Data_Already_Checked_Nb_of_Copy_List*) malloc(sizeof(struct Time_or_Data_Already_Checked_Nb_of_Copy_List));
	time_or_data_already_checked_nb_of_copy_list->head = NULL;

	/* 1. Loop on available jobs. */
	struct Job* j = head_job;
	while (j != NULL)
	{
		if (nb_non_available_cores < nb_cores)
		{
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);			
			printf("\nNeed to schedule job %d using file %d.\n", j->unique_id, j->data); fflush(stdout);
			#endif
			
			/* 2. Choose a node. */		
			/* Reset some values. */					
			min_score = -1;
			earliest_available_time = 0;
			// first_node_size_to_choose_from = 0;
			// last_node_size_to_choose_from = 0;
			is_being_loaded = false;
			time_to_reload_evicted_files = 0;
			nb_copy_file_to_load = 0;
			area_ratio_used = 0;
			
			/* In which node size I can pick. */
			// get_node_size_to_choose_from(j->index_node_list, &first_node_size_to_choose_from, &last_node_size_to_choose_from);
			//~ if (j->index_node_list == 0)
			//~ {
				//~ // first_node_size_to_choose_from = 0;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 1)
			//~ {
				//~ first_node_size_to_choose_from = 1;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 2)
			//~ {
				//~ first_node_size_to_choose_from = 2;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else
			//~ {
				//~ printf("Error index value in schedule_job_on_earliest_available_cores.\n");  fflush(stdout);
				//~ exit(EXIT_FAILURE);
			//~ }
			
			/* --- Reduced complexity nb of copy --- */
			if (multiplier_nb_copy != 0)
			{
				time_or_data_already_checked = was_time_or_data_already_checked_for_nb_copy(j->data, time_or_data_already_checked_nb_of_copy_list);
			}

			for (i = first_node_size_to_choose_from; i <= last_node_size_to_choose_from; i++)
			{
				//~ if (Planned_Area[i][j->index_node_list] > 0 || i == first_node_size_to_choose_from)
				//~ {
					struct Node* n = head_node[i]->head;
					while (n != NULL)
					{	
						#ifdef PRINT
						printf("On node %d?\n", n->unique_id); fflush(stdout);
						#endif
						
						/* 2.1. A = Get the earliest available time from the number of cores required by the job and add it to the score. */
						earliest_available_time = n->cores[j->cores - 1]->available_time; /* -1 because tab start at 0 */
						if (earliest_available_time < t) /* A core can't be available before t. This happens when a node is idling. */				
						{
							earliest_available_time = t;
						}
						
						#ifdef PRINT
						printf("A: EAT is: %d.\n", earliest_available_time); fflush(stdout);
						#endif
						
						if (min_score == -1 || earliest_available_time < min_score)
						{
							/* 2.2. B = Compute the time to load all data. For this look at the data that will be available at the earliest available time of the node. */
							if (j->data == 0)
							{
								time_to_load_file = 0;
							}
							else
							{
								time_to_load_file = is_my_file_on_node_at_certain_time_and_transfer_time(earliest_available_time, n, t, j->data, j->data_size, &is_being_loaded); /* Use the intervals in each data to get this info. */
							}
							
							#ifdef PRINT
							printf("B: Time to load file: %f. Is being loaded? %d.\n", time_to_load_file, is_being_loaded); fflush(stdout);
							#endif
												
							if (min_score == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file < min_score)
							{
								/* 2.5. Get the amount of files that will be lost because of this load by computing the amount of data that end at the earliest time only on the supposely choosen cores, excluding current file of course. */
								if (multiplier_file_evicted == 0)
								{
									time_to_reload_evicted_files = 0;
								}
								else
								{
									time_to_reload_evicted_files = time_to_reload_percentage_of_files_ended_at_certain_time(earliest_available_time, n, j->data, (float) j->cores/20);
								}
								
								#ifdef PRINT
								printf("C: Time to reload evicted files %f.\n", time_to_reload_evicted_files); fflush(stdout);
								#endif
								
								if (min_score == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files < min_score)
								{
									/* 2.5bis Get number of copy of the file we want to load on other nodes (if you need to load a file that is) at the time that is predicted to be used. So if a file is already loaded on a lot of node, you have a penalty if you want to load it on a new node. */
									if (time_to_load_file != 0 && is_being_loaded == false && multiplier_nb_copy != 0)
									{
										/* --- Reduced complexity nb of copy --- */
										if (time_or_data_already_checked == -1)
										{
											#ifdef PRINT
											printf("Need to compute nb of copy it was never done.\n");
											#endif
											nb_copy_file_to_load = get_nb_valid_copy_of_a_file(t, head_node, j->data);
											create_and_insert_head_time_or_data_already_checked_nb_of_copy_list(time_or_data_already_checked_nb_of_copy_list, j->data, nb_copy_file_to_load);
											time_or_data_already_checked = nb_copy_file_to_load;
											#ifdef PRINT
											printf("Compute nb of copy done, it's %d.\n", nb_copy_file_to_load);
											#endif
										}
										else
										{
											nb_copy_file_to_load = time_or_data_already_checked;
											#ifdef PRINT
											printf("Already done for job %d at time %d so nb of copy is %d.\n", j->unique_id, t, nb_copy_file_to_load);
											#endif
										}
									}
									else
									{
										nb_copy_file_to_load = 0;
									}
									
									#ifdef PRINT
									printf("Nb of copy for data %d at time %d on node %d is %d.\n", j->data, earliest_available_time, n->unique_id, nb_copy_file_to_load); fflush(stdout);
									#endif
									
									if (min_score == -1 || earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files + nb_copy_file_to_load*time_to_load_file*multiplier_nb_copy < min_score)
									{
										if (i != first_node_size_to_choose_from && multiplier_area_bigger_nodes != 0)
										{											
											if (division_by_planned_area == 1)
											{
												area_ratio_used = (float)(j->cores*j->walltime)/Planned_Area[i][j->index_node_list];
												#ifdef PRINT
												printf("area_ratio_used = (%d*%d)/%lld.\n", j->cores, j->walltime, Planned_Area[i][j->index_node_list]); fflush(stdout);
												#endif
											}
											else
											{
												area_ratio_used = j->cores*j->walltime;
												#ifdef PRINT
												printf("area_ratio_used = %d*%d.\n", j->cores, j->walltime); fflush(stdout);
												#endif
											}
										}
										else
										{
											area_ratio_used = 0;
										}
									
										/* Compute node's score. */
										score = earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files + nb_copy_file_to_load*time_to_load_file*multiplier_nb_copy + multiplier_area_bigger_nodes*area_ratio_used;
										
										#ifdef PRINT
										printf("Score for job %d is %lld (EAT: %d + TL: %f + TRL: %f + NCP: %f + AREA: %f) with node %d.\n", j->unique_id, score, earliest_available_time, multiplier_file_to_load*time_to_load_file, multiplier_file_evicted*time_to_reload_evicted_files, nb_copy_file_to_load*time_to_load_file*multiplier_nb_copy, multiplier_area_bigger_nodes*area_ratio_used, n->unique_id); fflush(stdout); 
										#endif
															
										/* 2.6. Get minimum score/ */
										if (min_score == -1)
										{
											min_time = earliest_available_time;
											min_score = score;
											j->node_used = n;
											choosen_time_to_load_file = time_to_load_file;
										}
										else if (min_score > score)
										{
											min_time = earliest_available_time;
											min_score = score;
											j->node_used = n;
											choosen_time_to_load_file = time_to_load_file;
										}
									}
								}
							}
						}
						
						#ifdef PRINT_SCORES_DATA
						fprintf(f_fcfs_score, "Node: %d EAT: %d C: %f CxX: %f Score: %f\n", n->unique_id, earliest_available_time, time_to_reload_evicted_files, time_to_reload_evicted_files*multiplier_file_evicted, earliest_available_time + multiplier_file_to_load*time_to_load_file + multiplier_file_evicted*time_to_reload_evicted_files);
						#endif
						
						n = n->next;
					}
				//~ }
			}
			
			j->transfer_time = choosen_time_to_load_file;
					
			/* Get start time and update available times of the cores. */
			j->start_time = min_time;
			j->end_time = min_time + j->walltime;
			
			for (int k = 0; k < j->cores; k++)
			{
				j->cores_used[k] = j->node_used->cores[k]->unique_id;
				if (j->node_used->cores[k]->available_time <= t)
				{
					nb_non_available_cores += 1;
				}
				j->node_used->cores[k]->available_time = min_time + j->walltime;
				
				/* Maybe I need job queue or not not sure. TODO. */
			}

			/* Need to add here intervals for current scheduling. */
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
				#ifdef PRINT
				printf("Need to create a data and intervals for the node %d data %d.\n", j->node_used->unique_id, j->data); fflush(stdout);
				#endif
				
				/* Create a class Data for this node. */
				struct Data* new = (struct Data*) malloc(sizeof(struct Data));
				new->next = NULL;
				new->unique_id = j->data;
				new->start_time = -1;
				new->end_time = -1;
				
				#ifndef DATA_PERSISTENCE
				new->nb_task_using_it = 0;
				#endif
				
				new->intervals = (struct Interval_List*) malloc(sizeof(struct Interval_List));
				new->intervals->head = NULL;
				new->intervals->tail = NULL;
				create_and_insert_tail_interval_list(new->intervals, j->start_time);
				create_and_insert_tail_interval_list(new->intervals, j->start_time + j->transfer_time);
				create_and_insert_tail_interval_list(new->intervals, j->end_time);
				new->size = j->data_size;
				insert_tail_data_list(j->node_used->data, new);
			}			
			
			#ifdef PRINT
			printf("After add interval are:\n"); fflush(stdout);
			print_data_intervals(head_node, t);
			#endif
			
			/* Need to sort cores after each schedule of a job. */
			sort_cores_by_available_time_in_specific_node(j->node_used);
										
			#ifdef PRINT
			print_decision_in_scheduler(j);
			#endif
						
			/* Insert in start times. */
			insert_next_time_in_sorted_list(start_times, j->start_time);
						
			/* --- Normal complexity nb of copy --- */
			/* Increment nb of copy for current file if we scheduled at time t the current job. */
			if (multiplier_nb_copy != 0 && j->start_time == t)
			{
				increment_time_or_data_nb_of_copy_specific_time_or_data(time_or_data_already_checked_nb_of_copy_list, j->data);
			}
			
			j = j->next;
		}				
		else
		{
			#ifdef PRINT
			printf("No more available cores.\n"); fflush(stdout);
			#endif
			
			/* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			while (j != NULL)
			{
				j->start_time = -1;
				j = j->next;
			}
			
			break;
		}
	}
	
	/* --- Reduced complexity nb of copy --- */
	/* Free time already checked. */
	if (multiplier_nb_copy != 0)
	{
		free_time_or_data_already_checked_nb_of_copy_linked_list(&time_or_data_already_checked_nb_of_copy_list->head);
	}

	#ifdef PRINT_SCORES_DATA
	fclose(f_fcfs_score);
	#endif
}

void mixed_if_EAT_is_t_scheduler(struct Job* head_job, struct Node_List** head_node, int t, int mode)
{
	#ifdef PRINT
	printf("Mix if EAT is t\n");
	#endif
	
	int nb_non_available_cores = get_nb_non_available_cores(node_list, t);
	//~ int min_score_locality = -1;
	//~ int i = 0;
	//~ bool best_score_is_zero = false;
	int earliest_available_time = 0;
	// int // first_node_size_to_choose_from = 0;
	// int // last_node_size_to_choose_from = 0;
	//~ float time_to_load_file = 0;
	//~ bool is_being_loaded = false;
	//~ int score = 0;
	//~ int min_time = 0;
	//~ int choosen_time_to_load_file = 0;
	bool found = false;
	//~ float time_to_reload_evicted_files = 0;
	
	/* Get intervals of data. */ 
	get_current_intervals(head_node, t);
	
	/* 1. Loop on available jobs. */
	struct Job* j = head_job;
	while (j != NULL)
	{
		if (nb_non_available_cores < nb_cores)
		{
			#ifdef PRINT
			printf("There are %d/%d available cores.\n", nb_cores - nb_non_available_cores, nb_cores);			
			printf("\nNeed to schedule job %d using file %d. T = %d\n", j->unique_id, j->data, t); fflush(stdout);
			#endif
		
			// get_node_size_to_choose_from(j->index_node_list, &first_node_size_to_choose_from, &last_node_size_to_choose_from);
			//~ if (j->index_node_list == 0)
			//~ {
				//~ // first_node_size_to_choose_from = 0;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 1)
			//~ {
				//~ first_node_size_to_choose_from = 1;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else if (j->index_node_list == 2)
			//~ {
				//~ first_node_size_to_choose_from = 2;
				//~ last_node_size_to_choose_from = 2;
			//~ }
			//~ else
			//~ {
				//~ printf("Error index value in schedule_job_on_earliest_available_cores.\n");  fflush(stdout);
				//~ exit(EXIT_FAILURE);
			//~ }
			
			earliest_available_time = get_min_EAT(head_node, first_node_size_to_choose_from, last_node_size_to_choose_from, j->cores, t);
			//~ printf("EAT is %d.\n", earliest_available_time);
				
			//~ fct a ajouter au .h, les 3 scheduler et le get min eat
					
			if (earliest_available_time == t) /* EFT */
			{
				#ifdef PLOT_STATS
				j->last_choosen_method = 0;
				#endif
				
				nb_non_available_cores = eft_scheduler_single_job(j, head_node, t, nb_non_available_cores);
			}
			else /* LOCALITY */
			{
				#ifdef PLOT_STATS
				j->last_choosen_method = 1;
				#endif
				
				nb_non_available_cores = locality_scheduler_single_job(j, head_node, t, nb_non_available_cores, mode);
			}

			
			/* 2. Choose a node. */		
			/* Reset some values. */					
			//~ min_score_locality = -1;
			//~ earliest_available_time = 0;
			//~ is_being_loaded = false;
			//~ time_to_load_file = 0;
			//~ time_to_reload_evicted_files = 0;
			//~ best_score_is_zero = false;
			
			/* In which node size I can pick. */
			
			/* NEW */
			/* Calcul du threshold dans lequel on regarde le EAT */
			//~ if (mixed_strategy_version == 2)
			//~ {
				//~ time_to_load_penalty = ((5.830780553511172 - 1)/2 + 1)*(j->data_size/0.1); /* Base mean 3* */
				//~ time_to_load_penalty = ((200 - 1)/2 + 1)*(j->data_size/0.1); /* Exagerated mean 100* */
				//~ time_to_load_penalty = ((400 - 1)/2 + 1)*(j->data_size/0.1); /* Very exagerated mean 200* */
			//~ }
			//~ else
			//~ {
				//~ time_to_load_penalty = 0;
			//~ }
			
			//~ #ifdef PRINT
			//~ printf("Time to load penalty is %f.\n", time_to_load_penalty);
			//~ #endif
			
			//~ for (i = first_node_size_to_choose_from; i <= last_node_size_to_choose_from; i++)
			//~ {
				//~ struct Node* n = head_node[i]->head;
				//~ while (n != NULL)
				//~ {
					//~ #ifdef PRINT
					//~ printf("On node %d?\n", n->unique_id); fflush(stdout);
					//~ #endif
					
					/* 2.1. A = Get the earliest available time from the number of cores required by the job. */
					//~ earliest_available_time = n->cores[j->cores - 1]->available_time; /* -1 because tab start at 0 */
					//~ if (earliest_available_time < t) /* A core can't be available before t. This happens when a node is idling. */				
					//~ {
						//~ earliest_available_time = t;
					//~ }					

					//~ }
					//~ n = n->next;
				//~ }
			//~ }
			
			//~ j->transfer_time = choosen_time_to_load_file;
					
			//~ /* Get start time and update available times of the cores. */
			//~ j->start_time = min_time;
			//~ j->end_time = min_time + j->walltime;
			
			//~ for (int k = 0; k < j->cores; k++)
			//~ {
				//~ j->cores_used[k] = j->node_used->cores[k]->unique_id;
				//~ if (j->node_used->cores[k]->available_time <= t)
				//~ {
					//~ nb_non_available_cores += 1;
				//~ }
				//~ j->node_used->cores[k]->available_time = min_time + j->walltime;
				
				//~ /* Maybe I need job queue or not not sure. TODO. */
			//~ }

			/* Need to add here intervals for current scheduling. */
			found = false;
			#ifdef DATA_PERSISTENCE
			struct Data* d = j->node_used->temp_data->head;
			#else
			struct Data* d = j->node_used->data->head;
			#endif
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
				#ifdef PRINT
				printf("Need to create a data and intervals for the node %d data %d.\n", j->node_used->unique_id, j->data); fflush(stdout);
				#endif
				
				/* Create a class Data for this node. */
				struct Data* new = (struct Data*) malloc(sizeof(struct Data));
				new->next = NULL;
				new->unique_id = j->data;
				new->start_time = -1;
				new->end_time = -1;
				
				#ifndef DATA_PERSISTENCE
				new->nb_task_using_it = 0;
				#endif
				
				new->intervals = (struct Interval_List*) malloc(sizeof(struct Interval_List));
				new->intervals->head = NULL;
				new->intervals->tail = NULL;
				create_and_insert_tail_interval_list(new->intervals, j->start_time);
				create_and_insert_tail_interval_list(new->intervals, j->start_time + j->transfer_time);
				create_and_insert_tail_interval_list(new->intervals, j->end_time);
				new->size = j->data_size;
				#ifdef DATA_PERSISTENCE
				insert_tail_data_list(j->node_used->temp_data, new);
				#else
				insert_tail_data_list(j->node_used->data, new);
				#endif
			}
			
			
			/* Need to sort cores after each schedule of a job. */
			//~ sort_cores_by_available_time_in_specific_node(j->node_used);
										
			#ifdef PRINT
			print_decision_in_scheduler(j);
			#endif

			/* Insert in start times. */
			//~ insert_next_time_in_sorted_list(start_times, j->start_time);
						
			j = j->next;
		}				
		else
		{
			#ifdef PRINT
			printf("No more available cores.\n"); fflush(stdout);
			#endif
			
			/* Need to put -1 at remaining start times of jobs to avoid error in n_vailable_cores. */
			while (j != NULL)
			{
				j->start_time = -1;
				j = j->next;
			}

			break;
		}
	}
}
