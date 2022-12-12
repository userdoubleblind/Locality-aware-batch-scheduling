/*
 * Copyright (C) - Anonymous for double blind submission.
 */

#include <main.h>

int nb_cores;
int nb_job_to_evaluate;
int finished_jobs;
int total_number_jobs;
int total_number_nodes;
struct Job_List* job_list; /* All jobs not available yet */
struct Job_List* new_job_list; /* New available jobs */
struct Job_List* job_list_to_start_from_history; /* With -2 and before start */
struct Job_List* scheduled_job_list; /* Scheduled or available */
struct Job_List* running_jobs; /* Started */
struct Node_List** node_list;
struct To_Print_List* jobs_to_print_list;
int running_cores;
int running_nodes;
#ifdef PRINT_CLUSTER_USAGE
int running_cores_from_workload_1;
int running_nodes_workload_1;
int mixed_mode;
#endif
int total_queue_time;
int first_subtime_day_0;
int nb_job_to_schedule;
int nb_cores_to_schedule;
char* scheduler;
char* output_file;
struct Next_Time_List* end_times;
struct Next_Time_List* start_times;
int nb_job_to_evaluate_started;
long long Allocated_Area[3][3];
long long Planned_Area[3][3];
int number_node_size[3];
int busy_cluster;
int backfill_mode;
int biggest_hole;
int biggest_hole_unique_id;
int global_nb_non_available_cores_at_time_t;
int nb_data_reuse;
int busy_cluster_threshold;

int main(int argc, char *argv[])
{
	nb_data_reuse = 0;
	
	if (argc != 5 && argc != 6 && argc != 7)
	{
		printf("Error: args must be 5, 6 or 7!\n");
		exit(EXIT_FAILURE);
	}
	
	/* By default I don't save/resume */
	bool need_to_save_state = false;
	bool need_to_resume_state = false;
	int time_to_save = 0;
		
	/* If you add save or resume as the last argument */
	if (argc > 5)
	{
		if (strcmp(argv[5], "save") == 0)
		{
			need_to_save_state = true;
			time_to_save = atoi(argv[6]);
			printf("Time to save is %d.\n", time_to_save);
		}
		else if (strcmp(argv[5], "save_and_resume") == 0)
		{
			need_to_resume_state = true;
			need_to_save_state = true;
			time_to_save = atoi(argv[6]);
			printf("Time to save_and_resume is %d.\n", time_to_save);
		}
		else if (strcmp(argv[5], "resume") == 0)
		{
			need_to_resume_state = true;
			if (argc > 6)
			{
				printf("Error: no arg must be after resume.\n");
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			printf("Error: argv[5] must be save or resume.\n");
			exit(EXIT_FAILURE);
		}
	}
	
	new_job_list = (struct Job_List*) malloc(sizeof(struct Job_List));
	new_job_list->head = NULL;
	new_job_list->tail = NULL;
		
	/* random seed init. */
	busy_cluster = 0; /* Not busy initially */
	srand(time(NULL));
	/* Init global variables */
	finished_jobs = 0;
	total_number_jobs = 0;
	running_cores = 0;
	running_nodes = 0;
	
	#ifdef PRINT_CLUSTER_USAGE
	running_nodes_workload_1 = 0;
	mixed_mode = 0;
	int nodes_loading_a_file = 0;
	int cores_loading_a_file = 0;
	#endif
	
	nb_job_to_evaluate_started = 0;
	total_queue_time = 0;
	//~ struct Job* first_job_in_queue = NULL; /* TODO: need to malloc ? Just if I use backfill. Useless else. */
	struct Job* job_pointer = (struct Job*) malloc(sizeof(struct Job));
	struct Job* temp = (struct Job*) malloc(sizeof(struct Job));
	
	end_times = malloc(sizeof(*end_times));
	end_times->head = NULL;
	start_times = malloc(sizeof(*start_times));
	start_times->head = NULL;
	jobs_to_print_list = malloc(sizeof(*jobs_to_print_list));
	jobs_to_print_list->head = NULL;
	jobs_to_print_list->tail = NULL;
	
	int old_finished_jobs = 0;
	
	/** Args **/
	char* input_job_file = argv[1];
	char* input_node_file = argv[2];
	scheduler = argv[3]; /* malloc ? */
	output_file = argv[4];
	if (output_file == NULL)
	{
		printf("Need file output\n");
		exit(1);
	}
	
	backfill_mode = 2;
	
	printf("Workload: %s\n", input_job_file);
	printf("Scheduler: %s\n", scheduler);
	
	/* Read cluster */
	if (need_to_resume_state == false)
	{
		read_cluster(input_node_file);
	}
	
	/* Read workload */
	if (need_to_resume_state == false)
	{
		read_workload(input_job_file);	
		nb_job_to_evaluate = get_nb_job_to_evaluate(job_list->head);
		first_subtime_day_0 = get_first_time_day_0(job_list->head);
	}
	
	#ifdef PRINT_CLUSTER_USAGE
	write_in_file_first_times_all_day(job_list->head, first_subtime_day_0);
	#endif

	int next_submit_time = first_subtime_day_0;
	int t = first_subtime_day_0;

	/* First start jobs from rackham's history. First need to sort it by start time */
	if (need_to_resume_state == false)
	{
		if (job_list_to_start_from_history->head != NULL)
		{
			get_state_before_day_0_scheduler(job_list_to_start_from_history->head, node_list, t);
		}
	}

	/* Getting the number of jobs we need to schedule */
	if (need_to_resume_state == false)
	{
		job_pointer = scheduled_job_list->head;
		nb_job_to_schedule = 0;
		nb_cores_to_schedule = 0;
		while(job_pointer != NULL)
		{
			nb_job_to_schedule += 1;
			nb_cores_to_schedule += job_pointer->cores;
			job_pointer = job_pointer->next;
		}
	}
	
	if (need_to_resume_state == false)
	{
		if (scheduled_job_list->head != NULL)
		{
			start_jobs(t, scheduled_job_list->head);
		}
	}
		
	#ifdef PRINT_CLUSTER_USAGE
	char* title = malloc(100*sizeof(char));
	strcpy(title, "outputs/Stats_");
	strcat(title, scheduler);
	strcat(title, ".csv");
	FILE* f_stats = fopen(title, "w");
	if (!f_stats)
	{
		perror("fopen in main");
        exit(EXIT_FAILURE);
	}
	fprintf(f_stats, "Time,Used cores,Used nodes,Scheduled jobs,Used nodes workload 1,Cores required in queue,Cores required from evaluated jobs in queue, Nodes loading a file, Used cores from workload 1, Cores loading a file\n");
	int nb_jobs_in_queue = 0;
	int nb_cores_in_queue = 0;
	int nb_cores_from_workload_1_in_queue = 0;
	free(title);
	#endif
	
	int multiplier_file_to_load = 0;
	int multiplier_file_evicted = 0;
	int start_immediately_if_EAT_is_t = 0;
	int mixed_strategy = 0;
	
	/* Getting informations for certain schedulers. */
	if (strcmp(scheduler, "LEA") == 0 || strcmp(scheduler, "LEM") == 0 || strcmp(scheduler, "LEO") == 0 || strcmp(scheduler, "LEA_BF") == 0 || strcmp(scheduler, "LEM_BF") == 0 || strcmp(scheduler, "LEO_BF") == 0)
	{
		if (strcmp(scheduler, "LEM") == 0)
		{
			mixed_strategy = 1;
		}
		else if (strcmp(scheduler, "LEO") == 0)
		{
			start_immediately_if_EAT_is_t = 1;
		}
		else if (strcmp(scheduler, "LEO_BF") == 0)
		{
			start_immediately_if_EAT_is_t = 1;
		}
		else if (strcmp(scheduler, "LEM_BF") == 0)
		{
			mixed_strategy = 1;
		}
		
		multiplier_file_to_load = 500;
		multiplier_file_evicted = 1;
	}
	
	busy_cluster_threshold = 80;
	
		
	bool new_jobs = false;
		
	if (need_to_resume_state == true)
	{
		need_to_resume_state = false;
		resume_state(&t, &old_finished_jobs, &next_submit_time, input_job_file);
	}
	
	/** START OF SIMULATION **/
	printf("Start simulation.\n"); fflush(stdout);
	
	#ifdef PRINT_CLUSTER_USAGE
	while (finished_jobs != total_number_jobs)
	#else
	while (nb_job_to_evaluate != nb_job_to_evaluate_started)
	#endif
	{
		if (need_to_save_state == true && t >= time_to_save)
		{			
			save_state(t, old_finished_jobs, next_submit_time, input_job_file);
			exit(1);
		}
				
		/* Get ended job. */
		old_finished_jobs = finished_jobs;
		if (end_times->head != NULL && end_times->head->time == t)
		{
			end_jobs(running_jobs->head, t);
		}
		
		/* Get started jobs. */
		if (start_times->head != NULL)
		{
			if (start_times->head->time == t)
			{
				start_jobs(t, scheduled_job_list->head);
			}
		}
				
		if ((running_nodes*100)/486 >= busy_cluster_threshold)
		{
			busy_cluster = 1;
		}
		else
		{
			busy_cluster = 0;
		}
		
		new_jobs = false;
		/* Get the set of available jobs at time t */
		/* Jobs are already sorted by subtime so I can simply stop with a break */
		if (next_submit_time == t) /* We have new jobs need to schedule them. */
		{
			new_jobs = true;
			
			#ifdef PRINT
			printf("We have new jobs at time %d.\n", t);
			#endif
			
			/* Copy in scheduled_job_list jobs and delete from job_list. */
			job_pointer = job_list->head;
			while (job_pointer != NULL)
			{
				if (job_pointer->subtime <= t)
				{
					#ifdef PRINT
					printf("New job %d.\n", job_pointer->unique_id);
					#endif
					
					/* Update nb of jobs to schedule (not running but available) */
					nb_job_to_schedule += 1;
					nb_cores_to_schedule += job_pointer->cores;
					
					temp = job_pointer->next;
						if (old_finished_jobs == finished_jobs) /* Que des nouveaux jobs, donc liste séparé */
						{
							#ifdef PRINT
							printf("Copy in new job list\n");
							#endif
							
							copy_delete_insert_job_list(job_list, new_job_list, job_pointer);
						}
						else
						{
							copy_delete_insert_job_list(job_list, scheduled_job_list, job_pointer);
						}
					
					job_pointer = temp;
				}
				else
				{
					break;
				}
			}
							
			if (job_pointer != NULL)
			{
				next_submit_time = job_pointer->subtime;
			}
			else
			{
				next_submit_time = -1;
			}
		}

		if (old_finished_jobs < finished_jobs && scheduled_job_list->head != NULL) /* Avec new job list */
		{
			#ifdef PRINT
			printf("Core(s) liberated. Need to free them.\n"); fflush(stdout);
			#endif
			
			/* Reset all cores and jobs. */
			reset_cores(node_list, t);
			
			/* Reset planned starting times. */
			free_next_time_linked_list(&start_times->head);
						
			call_scheduler(scheduler, scheduled_job_list, t, multiplier_file_to_load, multiplier_file_evicted, start_immediately_if_EAT_is_t, mixed_strategy);
							
			/* Get started jobs. */
			if (start_times->head != NULL)
			{
				if (start_times->head->time == t)
				{
					start_jobs(t, scheduled_job_list->head);
				}
			}
		}
		else if (new_jobs == true) /* No finished jobs but new jobs are available to be scheduled. */
		{			
			call_scheduler(scheduler, new_job_list, t, multiplier_file_to_load, multiplier_file_evicted, start_immediately_if_EAT_is_t, mixed_strategy);

			job_pointer = new_job_list->head;			
				while (job_pointer != NULL)
				{
					temp = job_pointer->next;
					copy_delete_insert_job_list(new_job_list, scheduled_job_list, job_pointer);
					job_pointer = temp;
				}	
			
			/* Get started jobs. */
			if (start_times->head != NULL)
			{
				if (start_times->head->time == t)
				{
					start_jobs(t, scheduled_job_list->head);
				}
			}
		}
						
		#ifdef PRINT_CLUSTER_USAGE
		get_length_job_list(scheduled_job_list->head, &nb_jobs_in_queue, &nb_cores_in_queue, &nb_cores_from_workload_1_in_queue);
				
		get_nb_nodes_and_cores_loading_a_file(node_list, t, &nodes_loading_a_file, &cores_loading_a_file);
		
		fprintf(f_stats, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n", t, running_cores, running_nodes*20, nb_jobs_in_queue, running_nodes_workload_1*20, nb_cores_in_queue + 486*20, nb_cores_from_workload_1_in_queue + 486*20, nodes_loading_a_file*20, running_cores_from_workload_1, cores_loading_a_file);		
		#endif
				
		if (start_times->head != NULL && t > start_times->head->time)
		{
			printf("ERROR in main.c, next start time is %d and t is %d.\n", start_times->head->time, t); fflush(stdout);
			exit(EXIT_FAILURE);
		}
		
		/* Time is advancing. */
		t += 1;
		
		if (t > 2000000000)
		{
			exit(EXIT_FAILURE);
		}
	}
	
	#ifdef PRINT_CLUSTER_USAGE
	fclose(f_stats);
	#endif
		
	/* Delete all running and scheduled jobs */
	job_pointer = running_jobs->head;
	while (job_pointer != NULL)
	{
		struct Job* temp = job_pointer->next;
		delete_job_linked_list(running_jobs, job_pointer->unique_id);
		job_pointer = temp;
	}
	job_pointer = scheduled_job_list->head;
	while (job_pointer != NULL)
	{
		struct Job* temp = job_pointer->next;
		delete_job_linked_list(scheduled_job_list, job_pointer->unique_id);
		job_pointer = temp;
	}

	/* Printing the results in an output file. */
	print_csv(jobs_to_print_list->head);

	return 1;
}
