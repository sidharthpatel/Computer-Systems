#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clock.h"
#include "structs.h"
#include "constants.h"
#include "scheduler.h"

/*
 * Preprocessors
 */
Process *remove_from_front(Queue *list);
void insert_front(Queue *list, Process *p);
void recurse_to_subtract(Queue *list);
Process *find_lowest_priority(Queue *list, int *head_is_select);
Process *delete_func(Queue *list, Process *previous, int *hs);
void free_everything(Queue *list);

/* Initialize the Schedule Struct
 * Follow the specification for this function.
 * Returns a pointer to the new Schedule or NULL on any error.
 */
Schedule *scheduler_init() {
	//Allocate memory for scheduler.
	Schedule *scheduler = malloc(sizeof(Schedule));
	//Check whether memory has been appropriately allocated.
	if(scheduler == NULL) {
		printf("Error allocating memory\n");
		exit(1);
	}
	//Allocate memory for Ready Queue.
	scheduler -> ready_queue = malloc(sizeof(Queue));
	//Check whether memory has been allocated properly.
	if(scheduler -> ready_queue == NULL) {
		printf("Error allocating memory\n");
		exit(1);
	}
	scheduler -> ready_queue -> head = NULL;
	scheduler -> ready_queue -> count = 0;
	//Allocate memory for Stopped Queue.
	scheduler -> stopped_queue = malloc(sizeof(Queue));
	//Check whether memory has been allocated properly.
	if(scheduler -> stopped_queue == NULL) {
		exit(1);
	}
	scheduler -> stopped_queue -> head = NULL;
	scheduler -> stopped_queue -> count = 0;
	//Allocate memory for Defunct Queue.
	scheduler -> defunct_queue = malloc(sizeof(Queue));
	//Check if memory has been allocated properly.
	if(scheduler -> defunct_queue == NULL) {
		exit(1);
	}
	scheduler -> defunct_queue -> head = NULL;
	scheduler -> defunct_queue -> count = 0;
	return scheduler;
}

void insert_front(Queue *list, Process *p) {
	p -> next = list -> head;
	list -> head = p;
	list -> count++;
}

int scheduler_add(Schedule *schedule, Process *process) {
	/* 
	* These bits are shifted prior to the function to accomodate
	* changing the flags in the add function.
	*/
	int Created_bit = CREATED << 26;
	int Ready_bit = READY << 26;
	int Defunct_bit = DEFUNCT << 26;
	/*
	* Checking the type of flag the process is under and following
	* specifications before the insertion of the process in the
	* Scheduler.
	*/
	if((process -> flags & Created_bit) == Created_bit) {
		process -> flags &= ~Created_bit;
		process -> flags |= Ready_bit;
		// A call to the helper function to insert process at front of Ready Queue.
		insert_front(schedule -> ready_queue, process);
	}
	else if((process -> flags & Ready_bit) == Ready_bit) {
		/*
		 * If-Condition statements for insertion of the process based
		 * on the state of process's time_remaining.
		 */
		if(process -> time_remaining < 0) {
			return -1;
		}
		if((process -> time_remaining) > 0) {
			//Insert Process at the front of Ready Queue.
			insert_front(schedule -> ready_queue, process);
		}
		else {
			//Change state from Ready to Defunct.
			process -> flags &= ~Ready_bit;
			process -> flags |= Defunct_bit;
			//Insert Process at the front of Defunct Queue.
			insert_front(schedule -> defunct_queue, process);
		}
	}
	else if((process -> flags & Defunct_bit) == Defunct_bit) {
		//Insert Process at the front of Defunct Queue.
		insert_front(schedule -> defunct_queue, process);
	}
	/*
	 * If all else fails during the addition of the Process,
	 * considering it a failure.
	 */
	else { return -1; }
	return 0;
}

int scheduler_stop(Schedule *schedule, int pid) {
	int Ready_bit = READY << 26;
	int Stopped_bit = STOPPED << 26;
	Process *temp = schedule -> ready_queue -> head;
	Process *removed_process = schedule -> ready_queue -> head;
	/*
	 * Terminate the function if the Ready Queue is empty.
	 */
	if(schedule -> ready_queue -> count == 0) {
		return -1;
	}
	/*
	 * Ready Queue contains only one Process or the head contains the matching pid.
	 */
	if(schedule -> ready_queue -> count == 1 || schedule -> ready_queue -> head -> pid == pid) {
		//Call to function for the removal of head.
		removed_process = remove_from_front(schedule -> ready_queue);
		//Change flag from Ready to Stopped.
		removed_process -> flags &= ~Ready_bit;
		removed_process -> flags |= Stopped_bit;
		//Insert the removed process from Ready Queue t0 Stopped Queue.
		insert_front(schedule -> stopped_queue, removed_process);
		return 0;
	}
	while(temp -> next != NULL) {
		if(temp -> next -> pid == pid) {
			removed_process = temp -> next;
			temp -> next = temp -> next -> next;
			removed_process -> next = NULL;
			//Change flag from Ready to Stopped.
			removed_process -> flags &= ~Ready_bit;
			removed_process -> flags |= Stopped_bit;
			schedule -> ready_queue -> count--;
			insert_front(schedule -> stopped_queue, removed_process);
			return 0;
		}
		temp = temp -> next;
	}
	return -1;
}

int scheduler_continue(Schedule *schedule, int pid) {
	int Stopped_bit = STOPPED << 26;
	int Ready_bit = READY << 26;
	Process *temp = schedule -> stopped_queue -> head;
	Process *removed_process = schedule -> stopped_queue -> head;
	/*
	 * If the Stopped Queue is empty then terminate the function altogther.
	 */
	if(schedule -> stopped_queue -> count == 0) {
		return -1;  
	}
	/*
	 * Stopped Queue is comprised of only one Process or the head itself has the matching pid.
	 */
  if(schedule -> stopped_queue -> count == 1 || schedule -> stopped_queue -> head -> pid == pid) {
		//Function call to remove the head.
		removed_process = remove_from_front(schedule -> stopped_queue);
		//Change flag from Stopped to Ready.
		removed_process -> flags &= ~Stopped_bit;
		removed_process -> flags |= Ready_bit;
		//Insert the removed process to teh Ready Queue.
		insert_front(schedule -> ready_queue, removed_process);
		return 0;
	}
	while(temp -> next != NULL) {
		if(temp -> next -> pid == pid) {
			removed_process = temp -> next;
			temp -> next = temp -> next -> next;
			removed_process -> next = NULL;
			//Change flag from Stopped to Ready.
			removed_process -> flags &= ~Stopped_bit;
			removed_process -> flags |= Ready_bit;
			schedule -> stopped_queue -> count--;
			insert_front(schedule -> ready_queue, removed_process);
			return 0;
		}
		temp = temp -> next;
	}
	return -1;
}

Process *remove_from_front(Queue *list) {
	Process *removed_process = list -> head;
	list -> head = list -> head -> next;
	removed_process -> next = NULL;
	list -> count--;
	return removed_process;
}

int scheduler_reap(Schedule *schedule, int pid) {
	int Defunct_bit = DEFUNCT << 26;
	int Terminated_bit = TERMINATED << 26;
	Process *temp = schedule -> defunct_queue -> head;
	Process *removed_process = schedule -> defunct_queue -> head;
	/*
	 * If the Defunct Queue is empty, there is no point in reaping because of the lack of processes
	 * in it. Return false immediately.
	 */
	if(schedule -> defunct_queue -> count == 0) {
		return -1;
	}
	/*
	 * Queue contains only one Process or the matching pid is the head itself then remove the head.
	 */
	if(schedule -> defunct_queue -> head -> pid == pid) {
		//Removing the head from the front and assiging it to the variable.
		removed_process = remove_from_front(schedule -> defunct_queue);
		// Change the state from Defunct to Terminated.
		removed_process -> flags &= ~Defunct_bit;
		removed_process -> flags |= Terminated_bit;
		//Return the Process exit code.
		int exit_code = removed_process -> flags;
		exit_code = exit_code << 6;
		exit_code = exit_code >> 6;
		free(removed_process);
		return exit_code;
	}
	while(temp -> next != NULL) {
		if(temp -> next -> pid == pid) {
			removed_process = temp -> next;
			temp -> next = temp -> next -> next;
			removed_process -> next = NULL;
			// Change state from Defunct to Terminated.
			removed_process -> flags &= ~Defunct_bit;
			removed_process -> flags |= Terminated_bit;
			// Return its exit code.
			int exit_code = removed_process -> flags;
			exit_code = exit_code << 6;
			exit_code = exit_code >> 6;
			//Decrementing the count.
			schedule -> defunct_queue -> count--;
			free(removed_process);
			return exit_code;
		}
		temp = temp -> next;
	}
	return -1;
}

Process *scheduler_generate(char *command, int pid, int base_priority, int time_remaining, int is_sudo) {
	//Allocating memory for a new Process.
	Process *new_process = malloc(sizeof(Process));
	// Allocation Successful check.
	if(new_process == NULL) {
		printf("Error allocating memory\n");
		exit(1);
	}
	// Allocating memory for a string.
	new_process -> command = malloc(MAX_COMMAND);
	// Checking for successful allocation.
	if(new_process -> command == NULL) {
		printf("Error allocating memory\n");
		exit(1);
	}
	//Copying string to the Process.
	strcpy(new_process -> command, command);
	new_process -> pid = pid;
	new_process -> base_priority = base_priority;
	new_process -> time_remaining = time_remaining;
	new_process -> cur_priority = base_priority;
	//If the Process contains a sudo bit, turn it on.
	if(is_sudo == 1) {
		new_process -> flags = SUDO << 26;
	}
	//Turn the created bit on and all other bits off (except sudo).
	new_process -> flags |= CREATED << 26;
	return new_process;
}

/*
 * Recurses through the given Queue decrementing the cur_priority
 * of each Processes within the Queue by one to prevent starvation.
 */
void recurse_to_subtract(Queue *list) {
	Process *recurse_var = list -> head;
	//Keep decrementing cur_priorty until the end of list.
	while (recurse_var != NULL) {
		if(recurse_var -> cur_priority != 0) {
			recurse_var -> cur_priority--;
		}
		recurse_var = recurse_var -> next;
	}
}

Process *find_lowest_priority(Queue *list, int *head_is_select) {
	int head_temp = 0;
	int lowest_priority = list -> head -> cur_priority;
	Process *temp = list -> head;
	Process *previous = NULL; 
	if(list -> head == NULL) {
		return NULL;
	}
	while(temp->next != NULL){
		if(temp -> next -> cur_priority < lowest_priority) {
			lowest_priority = temp -> next -> cur_priority;
			previous = temp;
		}
		temp = temp->next;
	}
	if(previous == NULL) {
		head_temp = 1;
	}
	*head_is_select = head_temp;
	return previous;
}

Process *delete_func(Queue *list, Process *previous, int *hs){
	Process *temp = list-> head;
	Process *select = list->head;
	// Flag to assist in the decision-making of Process removal.
	int nhs = *hs;
  /*
	 * If the Queue contains only one Process or the lowest cur_priority
	 * Process is the head itself, remove the head and relink the Queue.
	 */
	if(list -> count == 1 || nhs == 1) {
		// Calling remove function to remove Process from the Queue.
		select = remove_from_front(list);
	}
	else if(select != NULL && temp != NULL){	
		while(temp != NULL){
			// remove the next pointer.
			// remove at the front
			if(previous == list -> head){
				select = previous -> next;
				previous -> next = select -> next;
				select -> next = NULL;
				list -> count--;
				break;
			}
			else if(previous-> next ->next == NULL){
				//remove at the end.
				select = previous -> next;
				previous -> next = NULL;
				list -> count--;
				break;
			}
			else if ( temp == previous){
				//remove at the middle.
				//select = previous -> next;
				temp -> next = previous -> next -> next;
				select -> next = NULL;
				list -> count--;
				break;
			}
		temp = temp -> next;
		}
	}
	else {
		return NULL;
	}
	return select;
}

Process *scheduler_select(Schedule *schedule) {
  /* This a flag generated to accomodate in the removal of the Process from the front of
   * the Queue.
   */
	int head_is_select = 0;
	Process *removed_process = NULL;
  /*
	 * There is no point in even running the function if the Ready Queue is empty.
	 */
	if(scheduler_count(schedule -> ready_queue) == 0) {
		return NULL;
	}
	//Calls the helper function find_lowest_priority to search for the lowest possible priority.
	Process *previous = find_lowest_priority(schedule -> ready_queue, &head_is_select);
	// Calls the delete_func function to delete the Process found above.
	removed_process = delete_func(schedule->ready_queue, previous, &head_is_select);
	//Replacing the cur_priority of the removed process to based_priority.
	removed_process -> cur_priority = removed_process -> base_priority;
	//Recurse through the Queue and subtract one from each Process' cur_priority to prevent starvation.
	recurse_to_subtract(schedule -> ready_queue);
	return removed_process;
}

int scheduler_count(Queue *ll) {
	if(ll == NULL) {
		return -1;
	}
	return (ll -> count);
}

void free_everything(Queue *list){
	Process *temp = list -> head;
	while(list -> head != NULL) {
		list -> head = list -> head -> next;
		free(temp -> command);
		free(temp);
		temp = list -> head;
	}
}

void scheduler_free(Schedule *scheduler) {
	free_everything(scheduler -> ready_queue);
	free_everything(scheduler -> stopped_queue);
	free_everything(scheduler -> defunct_queue);
	free(scheduler -> ready_queue);
	free(scheduler -> stopped_queue);
	free(scheduler -> defunct_queue);
	free(scheduler);
	return;
}
