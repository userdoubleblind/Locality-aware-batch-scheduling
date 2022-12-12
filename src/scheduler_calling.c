/*
 * Copyright (C) - Anonymous for double blind submission.
 */
 
/**
 * Contain the function used in the main to call the desired scheduler.
 * Used twice in the main. One time when needing to schedule all jobs 
 * and one time when needing to schedule only the newly submitted jobs.
 **/

#include <main.h>

/** Takes all the paramters a scheduler could hold and calls the right scheduler. **/
void call_scheduler(char* scheduler, struct Job_List* liste, int t, int multiplier_file_to_load, int multiplier_file_evicted, int start_immediately_if_EAT_is_t, int mixed_strategy)
{
	/* 
	 * Calling LEA, LEO or LEM. They uses the same function. 
	 * LEO has start_immediately_if_EAT_is_t set to 1.
	 * LEM has mixed_strategy set to 1.
	 */
	if (strcmp(scheduler, "LEA") == 0 || strcmp(scheduler, "LEO") == 0 || strcmp(scheduler, "LEM") == 0)
	{
		LEA(liste->head, node_list, t, multiplier_file_to_load, multiplier_file_evicted, start_immediately_if_EAT_is_t, mixed_strategy);
	}
	/* Calling LEA or LEO with conservative backfilling. */
	else if (strcmp(scheduler, "LEA_BF") == 0 || strcmp(scheduler, "LEO_BF") == 0 || strcmp(scheduler, "LEM_BF") == 0)
	{
		LEA_BF(liste->head, node_list, t, multiplier_file_to_load, multiplier_file_evicted, start_immediately_if_EAT_is_t, mixed_strategy);
	}
	/* EFT uses LEA's function as well. Indeed if you set 
	 * multiplier_file_to_load to 1 and multiplier_file_evicted to 0, you
	 * get EFT.
	 **/
	else if (strcmp(scheduler, "EFT") == 0)
	{
		LEA(liste->head, node_list, t, 1, 0, 0, 0);
	}
	else if (strcmp(scheduler, "EFT_BF") == 0)
	{
		LEA_BF(liste->head, node_list, t, 1, 0, 0, 0);
	}
	/* Calling the scheduler for Fcfs */
	else if (strcmp(scheduler, "FCFS") == 0)
	{
		FCFS(liste->head, node_list, t);
	}
	/* Calling the scheduler for Fcfs with conservative backfilling */
	else if (strcmp(scheduler, "FCFS_BF") == 0)
	{
		FCFS_BF(liste->head, node_list, t);
	}
	else
	{
		printf("Error: wrong scheduler in arguments.\n"); fflush(stdout);
		exit(EXIT_FAILURE);
	}
}
