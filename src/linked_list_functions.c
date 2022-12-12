/*
 * Copyright (C) - Anonymous for double blind submission.
 */
 
/**
 * Functions to manage our linked list.
 * We have linked list for jobs, nodes, cores, data, intervals of data use and 
 * list of next start and end times of a job.
 **/
 
#include <main.h>

void insert_head_job_list(struct Job_List* liste, struct Job* j)
{
	if (liste->head == NULL)
	{
		liste->head = j;
		liste->tail = j;
	}
	else
	{
		j->next = liste->head;
		liste->head = j;
	}
}

void insert_tail_job_list(struct Job_List* liste, struct Job* j)
{
    if(liste->head == NULL)
    {
         liste->head = j;
	 }
    else
    {
        struct Job *lastNode = liste->head;
        while(lastNode->next != NULL)
        {
            lastNode = lastNode->next;
        }
        lastNode->next = j;
    }
}

void insert_tail_node_list(struct Node_List* liste, struct Node* n)
{
	if (liste->head == NULL)
	{
		liste->head = n;
		liste->tail = n;
	}
	else
	{
		liste->tail->next = n;
		liste->tail = n;
	}
}

void initialize_cores_in_a_hole(struct Core_in_a_hole_List* liste, struct Core_in_a_hole* c)
{
	liste = (struct Core_in_a_hole_List*) malloc(sizeof(struct Core_in_a_hole_List));
	liste->head = c;
}

void insert_cores_in_a_hole_list_sorted_decreasing_order(struct Core_in_a_hole_List* liste, struct Core_in_a_hole* c)
{
	if (liste->head == NULL || liste->head->start_time_of_the_hole < c->start_time_of_the_hole)
	{
		c->next = liste->head;
        liste->head = c;
    }
    else if (liste->head->start_time_of_the_hole == c->start_time_of_the_hole && liste->head->unique_id > c->unique_id)
    {
		c->next = liste->head;
        liste->head = c;
	}
    else
    {
		 struct Core_in_a_hole* current;
        current = liste->head;
        while (current->next != NULL && current->next->start_time_of_the_hole >= c->start_time_of_the_hole)
        {
            current = current->next;
        }
        c->next = current->next;
        current->next = c;
    }
}

void insert_cores_in_a_hole_list_sorted_increasing_order(struct Core_in_a_hole_List* liste, struct Core_in_a_hole* c)
{
	if (liste->head == NULL || liste->head->start_time_of_the_hole > c->start_time_of_the_hole)
	{
		c->next = liste->head;
        liste->head = c;
    }
    else if (liste->head->start_time_of_the_hole == c->start_time_of_the_hole && liste->head->unique_id > c->unique_id)
    {
		c->next = liste->head;
        liste->head = c;
	}
    else
    {
		struct Core_in_a_hole* current;
        current = liste->head;
        while (current->next != NULL && current->next->start_time_of_the_hole <= c->start_time_of_the_hole)
        {
            current = current->next;
        }
        c->next = current->next;
        current->next = c;
    }
}

void delete_core_in_hole_from_head(struct Core_in_a_hole_List* liste, int nb_cores_to_delete)
{
    if (liste == NULL)
    {
		printf("Error list empty.\n"); fflush(stdout);
        exit(EXIT_FAILURE);
    }
	struct Core_in_a_hole* temp = NULL;
	int i = 0;
    for (i = 0; i < nb_cores_to_delete; i++)
    {
		temp = liste->head;
        liste->head = temp->next;
        free(temp);
    }
}

void delete_core_in_hole_specific_core(struct Core_in_a_hole_List* liste, int unique_id_to_delete)
{
	struct Core_in_a_hole* temp = liste->head;
	struct Core_in_a_hole* prev = liste->head;
	
    if (temp != NULL && temp->unique_id == unique_id_to_delete)
    {
        liste->head = temp->next;
        free(temp);
        return;
    }
	
	while (temp->unique_id != unique_id_to_delete && temp != NULL)
	{
		prev = temp;
		temp = temp->next;
	}
	if (temp == NULL)
	{
		printf("Error, deletion of core %d failed.\n", unique_id_to_delete); fflush(stdout);
		exit(EXIT_FAILURE);
	}
	prev->next = temp->next;	
	if (unique_id_to_delete != temp->unique_id)
	{
		printf("ERROR: unique_id_to_delete != temp->unique_id!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}
	
	free(temp);
}

void create_and_insert_tail_interval_list(struct Interval_List* liste, int time_to_insert)
{
	struct Interval* i = (struct Interval*) malloc(sizeof(struct Interval));
	i->time = time_to_insert;
	i->next = NULL;
	
	if (liste->head == NULL)
	{
		liste->head = i;
		liste->tail = i;
	}
	else
	{
		liste->tail->next = i;
		liste->tail = i;
	}
}

void insert_tail_to_print_list(struct To_Print_List* liste, struct To_Print* tp)
{
	if (liste->head == NULL)
	{
		liste->head = tp;
		liste->tail = tp;
	}
	else
	{
		liste->tail->next = tp;
		liste->tail = tp;
	}
}

void insert_tail_data_list(struct Data_List* liste, struct Data* d)
{
	if (liste->head == NULL)
	{
		liste->head = d;
		liste->tail = d;
	}
	else
	{
		liste->tail->next = d;
		liste->tail = d;
	}
}

void insert_next_time_in_sorted_list(struct Next_Time_List* liste, int time_to_insert)
{
    struct Next_Time* current;
    /* Special case for the head end */
    if (liste->head == NULL || liste->head->time >= time_to_insert) {
		
		/* I don't want the same time twice. So I add this condition. */
		if (liste->head != NULL)
		{
			if (liste->head->time == time_to_insert)
			{
				return;
			}
		}
		
		struct Next_Time* new = (struct Next_Time*) malloc(sizeof(struct Next_Time));
		new->next = liste->head;
		new->time = time_to_insert;
        liste->head = new;
    }
    else
    {
        current = liste->head;
        while (current->next != NULL && current->next->time <= time_to_insert)
        {
            current = current->next;
        }
        
 		if (current != NULL)
 		{
			if (current->time == time_to_insert)
			{
				return;
			}   
		}
        
        struct Next_Time* new = (struct Next_Time*) malloc(sizeof(struct Next_Time));
        new->time = time_to_insert;
        new->next = current->next;
        current->next = new;
    }
}

void insert_job_in_sorted_list(struct Job_List* liste, struct Job* j)
{
    struct Job* current;
    if (liste->head == NULL || liste->head->start_time_from_history >= j->start_time_from_history)
    {	
		j->next = liste->head;
        liste->head = j;
    }
    else
    {
        current = liste->head;
        while (current->next != NULL && current->next->start_time_from_history <= j->start_time_from_history)
        {
            current = current->next;
        }
        j->next = current->next;
        current->next = j;
    }
}

/* Delete all time corresponding to this time. The list is sorted so it's quick. */
void delete_next_time_linked_list(struct Next_Time_List* liste, int time_to_delete)
{
    if (liste == NULL)
    {
		printf("Error list empty.\n");
        exit(EXIT_FAILURE);
    }

	struct Next_Time* temp = liste->head;
	struct Next_Time* prev = liste->head;
	
    if (temp != NULL && temp->time == time_to_delete)
    {
        liste->head = temp->next;
        free(temp);
        return;
    }
	
	while (temp->time != time_to_delete && temp != NULL)
	{
		prev = temp;
		temp = temp->next;
	}
	if (temp == NULL)
	{
		printf("Error, deletion of time %d failed.\n", time_to_delete);
		exit(EXIT_FAILURE);
	}
	prev->next = temp->next;
	free(temp);
}

void delete_job_linked_list(struct Job_List* liste, int unique_id_to_delete)
{
    if (liste == NULL)
    {
		printf("Error list empty.\n"); fflush(stdout);
        exit(EXIT_FAILURE);
    }

	struct Job* temp = liste->head;
	struct Job* prev = liste->head;
	
    if (temp != NULL && temp->unique_id == unique_id_to_delete)
    {
        liste->head = temp->next;
		if (temp->cores_used != NULL)
		{
			free(temp->cores_used);
		}
		free(temp);
        return;
    }
	
	while (temp->unique_id != unique_id_to_delete && temp != NULL)
	{
		prev = temp;
		temp = temp->next;
	}
	if (temp == NULL)
	{
		printf("Error, deletion of job %d failed.\n", unique_id_to_delete); fflush(stdout);
		exit(EXIT_FAILURE);
	}
	prev->next = temp->next;
	
	if (unique_id_to_delete != temp->unique_id)
	{
		printf("ERROR!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}
	
	if (temp->cores_used != NULL)
	{
		free(temp->cores_used);
	}
	free(temp);
}

void delete_specific_data_from_node(struct Data_List* liste, int unique_id_to_delete)
{
    if (liste == NULL)
    {
		printf("Error list empty.\n"); fflush(stdout);
        exit(EXIT_FAILURE);
    }

	struct Data* temp = liste->head;
	struct Data* prev = liste->head;
	
    if (temp != NULL && temp->unique_id == unique_id_to_delete)
    {
        liste->head = temp->next;
        free(temp);
        return;
    }
	
	while (temp->unique_id != unique_id_to_delete && temp != NULL)
	{
		prev = temp;
		temp = temp->next;
	}
	if (temp == NULL)
	{
		printf("Error, deletion of data %d failed.\n", unique_id_to_delete); fflush(stdout);
		exit(EXIT_FAILURE);
	}
	prev->next = temp->next;
	
	if (unique_id_to_delete != temp->unique_id)
	{
		printf("ERROR!\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}
	free(temp);
}

void copy_delete_insert_job_list(struct Job_List* to_delete_from, struct Job_List* to_append_to, struct Job* j)
{
	if (to_delete_from == NULL)
    {
		printf("Error list empty.\n");
        exit(EXIT_FAILURE);
    }
        
	struct Job* new = (struct Job*) malloc(sizeof(struct Job));
	new->next = NULL;
	new->unique_id = j->unique_id;
	new->subtime = j->subtime;
	new->delay = j->delay;
	new->walltime = j->walltime;
	new->cores = j->cores;
	new->data = j->data;
	new->data_size = j->data_size;
	new->index_node_list = j->index_node_list;
	new->start_time = j->start_time;
	new->end_time = j->end_time;
	new->end_before_walltime = j->end_before_walltime;
	new->transfer_time = j->transfer_time;
	new->waiting_for_a_load_time = j->waiting_for_a_load_time;
	new->workload = j->workload;
	new->start_time_from_history = j->start_time_from_history;
	new->node_from_history = j->node_from_history;
	new->node_used = (struct Node*) malloc(sizeof(struct Node));
	new->node_used = j->node_used;
	
	new->cores_used = (int*) malloc(new->cores*sizeof(int));
	for (int i = 0; i < new->cores; i++)
	{
		new->cores_used[i] = j->cores_used[i];
	}

	delete_job_linked_list(to_delete_from, j->unique_id);
	
	insert_tail_job_list(to_append_to, new);
}

void copy_delete_insert_job_list_sorted_by_file_size(struct Job_List* to_delete_from, struct Job_List* to_append_to, struct Job* j)
{
	if (to_delete_from == NULL)
    {
		printf("Error list empty.\n");
        exit(EXIT_FAILURE);
    }
    
	struct Job* new = (struct Job*) malloc(sizeof(struct Job));
	new->next = NULL;
	new->unique_id = j->unique_id;
	new->subtime = j->subtime;
	new->delay = j->delay;
	new->walltime = j->walltime;
	new->cores = j->cores;
	new->data = j->data;
	new->data_size = j->data_size;
	new->index_node_list = j->index_node_list;
	new->start_time = j->start_time;
	new->end_time = j->end_time;
	new->end_before_walltime = j->end_before_walltime;
	new->transfer_time = j->transfer_time;
	new->waiting_for_a_load_time = j->waiting_for_a_load_time;
	new->workload = j->workload;
	new->start_time_from_history = j->start_time_from_history;
	new->node_from_history = j->node_from_history;
	new->node_used = (struct Node*) malloc(sizeof(struct Node));
	new->node_used = j->node_used;
	new->cores_used = malloc(new->cores*sizeof(int));
	new->cores_used = j->cores_used;

	delete_job_linked_list(to_delete_from, j->unique_id);
	
	insert_job_sorted_by_decreasing_file_size(&to_append_to->head, new);
}

void get_length_job_list(struct Job* head, int* nb_jobs_in_queue, int* nb_cores_in_queue, int* nb_cores_from_workload_1_in_queue)
{
	*nb_jobs_in_queue = 0;
	*nb_cores_in_queue = 0;
	*nb_cores_from_workload_1_in_queue = 0;
	struct Job* j = head;
	while (j != NULL)
	{
		*nb_jobs_in_queue += 1;
		*nb_cores_in_queue += j->cores;
		if (j->workload == 1)
		{
			*nb_cores_from_workload_1_in_queue += j->cores;
		}
		j = j->next;
	}
}

void free_next_time_linked_list(struct Next_Time** head_ref)
{
	struct Next_Time* current = *head_ref;
	struct Next_Time* next;
 
	while (current != NULL)
	{
		next = current->next;
		free(current);
		current = next;
	}
   
   *head_ref = NULL;
}

void free_cores_in_a_hole(struct Core_in_a_hole** head_ref)
{
	struct Core_in_a_hole* current = *head_ref;
	struct Core_in_a_hole* next;
 
   while (current != NULL)
   {
       next = current->next;
       free(current);
       current = next;
   }
   
   *head_ref = NULL;
}

void free_interval_linked_list(struct Interval** head_ref, struct Interval** tail_ref)
{
   struct Interval* current = *head_ref;
   struct Interval* next;
 
   while (current != NULL)
   {
       next = current->next;
       free(current);
       current = next;
   }
   
   *head_ref = NULL;
   *tail_ref = NULL;
}

void sort_cores_by_available_time_in_specific_node(struct Node* n)
{
	for (int step = 0; step < 20 - 1; step++)
	{
		for (int i = 0; i < 20 - step - 1; ++i)
		{
			if (n->cores[i]->available_time > n->cores[i + 1]->available_time)
			{
				struct Core* temp = n->cores[i];
				n->cores[i] = n->cores[i+1];
				n->cores[i + 1] = temp;
			}
			else if (n->cores[i]->available_time == n->cores[i + 1]->available_time && n->cores[i]->unique_id > n->cores[i + 1]->unique_id)
			{
				struct Core* temp = n->cores[i];
				n->cores[i] = n->cores[i+1];
				n->cores[i + 1] = temp;
			}
		}
	}
}

void sort_cores_by_unique_id_in_specific_node(struct Node* n)
{
	for (int step = 0; step < 20 - 1; step++)
	{
		for (int i = 0; i < 20 - step - 1; ++i)
		{
			if (n->cores[i]->unique_id > n->cores[i + 1]->unique_id)
			{
				struct Core* temp = n->cores[i];
				n->cores[i] = n->cores[i+1];
				n->cores[i + 1] = temp;
			}
		}
	}
}

void insert_job_sorted_by_decreasing_file_size(struct Job** head, struct Job* newNode)
{
    struct Job dummy;
    struct Job* current = &dummy;
    dummy.next = *head;
 
    while (current->next != NULL && current->next->data_size >= newNode->data_size) {
        current = current->next;
    }
    newNode->next = current->next;
    current->next = newNode;
    *head = dummy.next;
}
 
void sort_job_list_by_file_size(struct Job** head)
{
    struct Job* result = NULL;
    struct Job* current = *head;
    struct Job* next;
 
    while (current != NULL)
    {
        next = current->next;
 
        insert_job_sorted_by_decreasing_file_size(&result, current);
        current = next;
    }
	*head = result;
}
