/*
 * Copyright (C) - Anonymous for double blind submission.
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <limits.h>

/* Global variables */
extern int nb_cores;
extern int nb_job_to_evaluate;
extern int finished_jobs;
extern int total_number_jobs;
extern int total_number_nodes;
extern struct Job_List* job_list; /* All jobs not available yet */
extern struct Job_List* new_job_list; /* New available jobs */
extern struct Job_List* job_list_to_start_from_history; /* jobs started from logs */
extern struct Job_List* scheduled_job_list; /* Jobs scheduled or available to be scheduled */
extern struct Job_List* running_jobs; /* Jobs started */
extern struct Node_List** node_list; /* Set of nodes */
extern struct To_Print_List* jobs_to_print_list; /* To store the evaluated jobs that terminated and later plot their statistics in a file */
extern int running_cores;
extern int running_nodes;
extern int nb_job_to_schedule; /* Jobs ready but not running */
extern int nb_cores_to_schedule; /* Cores ready but not running */
extern int total_queue_time; /* For statistics */
extern int first_subtime_day_0; /* First time of Day 0 */
extern char* scheduler; /* Used scheduler */
extern char* output_file; /* Output file to write the results */
extern int number_node_size[3];
extern int busy_cluster; /* 0 = no, 1 = yes. Used for fcfs with a score adaptative multiplier that modify multiplier based on cluster contention. */
extern int backfill_mode; /* 0 = no, 1 = yes. Used for fcfs with a score adaptative multiplier that modify multiplier based on cluster contention. */
extern int biggest_hole;
extern int biggest_hole_unique_id;
extern int global_nb_non_available_cores_at_time_t;
extern int nb_data_reuse; /* For statistics */
extern int busy_cluster_threshold; /* Threshold used to switch between EFT and LEA by LEM. Set to 80% */
extern int nb_job_to_evaluate_started; /* When we started all the jobs from the evaluated day we can stop the schedule. This variable help us track our progression. */

#ifdef PRINT_CLUSTER_USAGE /* Define used to print the cluster' usage as seen in the visualizations of the article. */
extern int running_nodes_workload_1;
extern int mixed_mode;
extern int running_cores_from_workload_1;
#endif

/* List of start and end times of the jobs. Used to only call the function to start and end jobs when needed. */
extern struct Next_Time_List* end_times;
extern struct Next_Time_List* start_times;

struct Next_Time_List
{
	struct Next_Time* head;
};

struct Next_Time
{
	struct Next_Time* next;
	int time;
};

struct Job_List 
{
	struct Job* head;
	struct Job* tail;
};

struct Node_List
{
	struct Node* head;
	struct Node* tail;
};

struct Data_List
{
	struct Data* head;
	struct Data* tail;
};

struct Job
{
	struct Job* next;
	int unique_id;
    int subtime;
    int delay;
    int walltime;
    int cores;
    int data;
    float data_size;
    int index_node_list;
    int start_time;
    int end_time;
    bool end_before_walltime;
    struct Node* node_used;
    int* cores_used;
    int transfer_time;
    int waiting_for_a_load_time;
    int workload;
    int start_time_from_history;
    int node_from_history;    
};

struct Node
{
	struct Node* next;
    int unique_id;
    int memory;
    float bandwidth;
    struct Data_List* data;
    struct Core** cores;
    int n_available_cores;
    int index_node_list;
    int number_cores_in_a_hole;
    struct Core_in_a_hole_List* cores_in_a_hole;
    
    #ifdef PRINT_CLUSTER_USAGE
    int nb_jobs_workload_1;
    #endif
    
    int end_of_file_load;
};

struct Core_in_a_hole_List
{
	struct Core_in_a_hole* head;
	struct Core_in_a_hole* tail;
};

struct Core_in_a_hole
{
	struct Core_in_a_hole* next;
	int unique_id;
	int start_time_of_the_hole;
};

struct Data
{
	struct Data* next;
    int unique_id;
    int start_time;
    int end_time;
    int nb_task_using_it;
    
    struct Interval_List* intervals;
    float size;
};

struct Core
{
    int unique_id;
    int available_time;
    bool running_job;
    int running_job_end;
    
    #ifdef PRINT_CLUSTER_USAGE
    int end_of_file_load; /* Used to then get the total number of cores loading a file. */
    #endif
};

struct To_Print_List
{
	struct To_Print* head;
	struct To_Print* tail;
};

struct To_Print
{
	struct To_Print* next;
	int job_unique_id;
    int job_subtime;
    int time;
    int time_used;
    int transfer_time;
    int job_start_time;
    int job_end_time;
    int job_cores;
    int waiting_for_a_load_time;
    float empty_cluster_time;
    int data_type;
    float job_data_size;
    int upgraded;
};

struct Interval_List
{
	struct Interval* head;
	struct Interval* tail;
};

struct Interval
{
	struct Interval* next;
	int time;
};

/* From read_input_files.c */
void read_cluster(char* input_node_file);
void read_workload(char* input_job_file);
int get_nb_job_to_evaluate(struct Job* l);
int get_first_time_day_0(struct Job* l);
void write_in_file_first_times_all_day(struct Job* l, int first_subtime_day_0);

/* From print_functions.c */
void print_node_list(struct Node_List** list);
void print_job_list(struct Job* list);
void print_single_node(struct Node* n);
void print_decision_in_scheduler(struct Job* j);
void print_cores_in_specific_node(struct Node* n);
void print_time_list(struct Next_Time* list, int end_or_start);
void to_print_job_csv(struct Job* job, int time);
void print_csv(struct To_Print* head_to_print);
void print_data_intervals(struct Node_List** list, int t);
void print_tab_of_int (int arr[], int n);
void print_holes(struct Node_List** head_node);
void print_holes_specific_node(struct Node* n);
void save_state(int t, int old_finished_jobs, int next_submit_time, char* input_job_file);
void resume_state(int* t, int* old_finished_jobs, int* next_submit_time, char* input_job_file);
void print_job_to_print(struct To_Print* tp);

/* From basic_functions.c */
int get_min_EAT(struct Node_List** head_node, int first_node_size_to_choose_from, int last_node_size_to_choose_from, int nb_cores, int t);
void schedule_job_specific_node_at_earliest_available_time(struct Job* j, struct Node* n, int t);
void start_jobs(int t, struct Job* scheduled);
void end_jobs(struct Job* job_list_head, int t);
void add_data_in_node (int data_unique_id, float data_size, struct Node* node_used, int t, int end_time, int* transfer_time, int* waiting_for_a_load_time, int delay, int walltime, int start_time, int cores);
int get_nb_non_available_cores(struct Node_List** n, int t);
int schedule_job_on_earliest_available_cores(struct Job* j, struct Node_List** head_node, int t, int nb_non_available_cores);
void reset_cores(struct Node_List** l, int t);
void remove_data_from_node(struct Job* j, int t);
void get_current_intervals(struct Node_List** head_node, int t);
float is_my_file_on_node_at_certain_time_and_transfer_time(int predicted_time, struct Node* n, int t, int current_data, float current_data_size, bool* is_being_loaded);
float time_to_reload_percentage_of_files_ended_at_certain_time(int predicted_time, struct Node* n, int current_data, float percentage_occupied);
void schedule_job_on_earliest_available_cores_with_conservative_backfill(struct Job* j, struct Node_List** head_node, int t, int backfill_mode, int* nb_non_available_cores, int* nb_non_available_cores_at_time_t);
void schedule_job_fcfs_score_with_conservative_backfill(struct Job* j, struct Node_List** head_node, int t, int multiplier_file_to_load, int multiplier_file_evicted, int start_immediately_if_EAT_is_t, int* nb_non_available_cores, int* nb_non_available_cores_at_time_t, int mixed_strategy, int* temp_running_nodes);
void get_nb_nodes_and_cores_loading_a_file(struct Node_List** head_node, int t, int* nodes_loading_a_file, int* cores_loading_a_file);

/* From linked_list_functions.c */
void insert_head_job_list(struct Job_List* liste, struct Job* j);
void insert_tail_job_list(struct Job_List* liste, struct Job* j);
void insert_tail_node_list(struct Node_List* liste, struct Node* n);
void insert_tail_data_list(struct Data_List* liste, struct Data* d);
void delete_job_linked_list(struct Job_List* liste, int unique_id_to_delete);
void copy_delete_insert_job_list(struct Job_List* to_delete_from, struct Job_List* to_append_to, struct Job* j);
void copy_delete_insert_job_list_sorted_by_file_size(struct Job_List* to_delete_from, struct Job_List* to_append_to, struct Job* j);
void get_length_job_list(struct Job* head, int* nb_jobs_in_queue, int* nb_cores_in_queue, int* nb_cores_from_workload_1_in_queue);
void insert_next_time_in_sorted_list(struct Next_Time_List* liste, int time_to_insert);
void delete_next_time_linked_list(struct Next_Time_List* liste, int time_to_delete);
void free_next_time_linked_list(struct Next_Time** head_ref);
void insert_tail_to_print_list(struct To_Print_List* liste, struct To_Print* tp);
void insert_job_in_sorted_list(struct Job_List* liste, struct Job* j);
void create_and_insert_tail_interval_list(struct Interval_List* liste, int time_to_insert);
void free_interval_linked_list(struct Interval** head_ref, struct Interval** tail_ref);
void sort_cores_by_available_time_in_specific_node(struct Node* n);
void insert_job_sorted_by_decreasing_file_size(struct Job** head, struct Job* newNode);
void sort_job_list_by_file_size(struct Job** head);
void initialize_cores_in_a_hole(struct Core_in_a_hole_List* liste, struct Core_in_a_hole* c);
void insert_cores_in_a_hole_list_sorted_decreasing_order(struct Core_in_a_hole_List* liste, struct Core_in_a_hole* c);
void delete_core_in_hole_from_head(struct Core_in_a_hole_List* liste, int nb_cores_to_delete);
void free_cores_in_a_hole(struct Core_in_a_hole** head_ref);
void delete_core_in_hole_specific_core(struct Core_in_a_hole_List* liste, int unique_id_to_delete);
void delete_specific_data_from_node(struct Data_List* liste, int unique_id_to_delete);
void sort_cores_by_unique_id_in_specific_node(struct Node* n);
void insert_cores_in_a_hole_list_sorted_increasing_order(struct Core_in_a_hole_List* liste, struct Core_in_a_hole* c);

/* From scheduler.c */
void get_state_before_day_0_scheduler(struct Job* j, struct Node_List** n, int t);
void FCFS(struct Job* head_job, struct Node_List** head_node, int t);
void FCFS_BF(struct Job* head_job, struct Node_List** head_node, int t);
void LEA(struct Job* head_job, struct Node_List** head_node, int t, int multiplier_file_to_load, int multiplier_file_evicted, int start_immediately_if_EAT_is_t, int mixed_strategy);
void LEA_BF(struct Job* head_job, struct Node_List** head_node, int t, int multiplier_file_to_load, int multiplier_file_evicted, int start_immediately_if_EAT_is_t, int mixed_strategy);

/* From backfill_functions.c */
bool can_it_get_backfilled (struct Job* j, struct Node* n, int t, int* nb_cores_from_hole, int* nb_cores_from_outside);
void update_cores_for_backfilled_job(struct Job* j, int t, int nb_cores_from_hole, int nb_cores_from_outside);
void fill_cores_minimize_holes (struct Job* j, bool backfill_activated, int backfill_mode, int t, int* nb_non_available_cores);
bool only_check_conservative_backfill(struct Job* j, struct Node_List** head_node, int t, int backfill_mode, int* nb_non_available_cores_at_time_t);
bool only_check_conservative_backfill_with_a_score(struct Job* j, struct Node_List** head_node, int t, int backfill_mode, int* nb_non_available_cores_at_time_t, int multiplier_file_to_load, int multiplier_file_evicted, int start_immediately_if_EAT_is_t);
void get_new_biggest_hole(struct Node_List** head_node);

/* from scheduler_calling.c */
void call_scheduler(char* scheduler, struct Job_List* liste, int t, int multiplier_file_to_load, int multiplier_file_evicted, int start_immediately_if_EAT_is_t, int mixed_strategy);
