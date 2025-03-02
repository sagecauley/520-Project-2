#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include "dyn_array.h"
#include "processing_scheduling.h"


// You might find this handy.  I put it around unused parameters, but you should
// remove it before you submit. Just allows things to compile initially.
#define UNUSED(x) (void)(x)

// private function
void virtual_cpu(ProcessControlBlock_t *process_control_block) 
{
	// decrement the burst time of the pcb
	--process_control_block->remaining_burst_time;
}

int compare_remaining_time(const void* a, const void* b) {
	ProcessControlBlock_t* pcb1 = (ProcessControlBlock_t*)a;
	ProcessControlBlock_t* pcb2 = (ProcessControlBlock_t*)b;

	return pcb1->remaining_burst_time - pcb2->remaining_burst_time;
}

int compare_arrival_time(const void* a, const void* b) {
	ProcessControlBlock_t *pcb1 = (ProcessControlBlock_t*)a;
	ProcessControlBlock_t *pcb2 = (ProcessControlBlock_t*)b;

	return pcb1->arrival - pcb2->arrival;
}
bool first_come_first_serve(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	if (ready_queue == NULL || result == NULL) {
		return false;
	}

	size_t n = dyn_array_size(ready_queue);
	if (n == 0) {
		return false;
	}

	//variables to hold the time
	uint32_t total_waiting = 0;
	uint32_t total_turnaround = 0;
	//total current time
	uint32_t current = 0;
	uint32_t total_run = 0;

	dyn_array_sort(ready_queue, compare_arrival_time);

	
	for (size_t i = 0; i < n; i++) {
		//gets the specific element at i
		ProcessControlBlock_t *pcb = (ProcessControlBlock_t*)dyn_array_at(ready_queue, i);
		//if the current gets to be greater than the arrival then it resets since wait would be 0 and the ones after would only be waiting for this current process to end
		if (current < pcb->arrival) {
			current = pcb->arrival;
		}
		total_waiting += current - pcb->arrival;

		current += pcb->remaining_burst_time;
		total_turnaround += current - pcb->arrival;
		total_run += pcb->remaining_burst_time;
		
	}

	result->average_waiting_time = (float)total_waiting / n;
	result->average_turnaround_time = (float)total_turnaround / n;
	result->total_run_time =  total_run;

	return true;
}


bool shortest_job_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}

// Priority Helper Function: Sorting for priority-based scheduling
int compare_priority(const void *a, const void *b){
	const ProcessControlBlock_t *pcb_a = (const ProcessControlBlock_t *)a;
	const ProcessControlBlock_t *pcb_b = (const ProcessControlBlock_t *)b; 

	if(pcb_a->priority == pcb_b->priority){
		return(pcb_a->arrival - pcb_b->arrival);
	}
	
	return(pcb_a->priority - pcb_b->priority);
}

bool priority(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	if(ready_queue == NULL || result == NULL){
		return false;
	}

	size_t n = dyn_array_size(ready_queue);
	if(n == 0){
		return false;
	}

	// Sort ready queue based on prirority and arrival time
	dyn_array_sort(ready_queue, compare_priority);

	uint32_t current_time = 0;
	uint32_t total_waiting = 0;
	uint32_t total_turnaround = 0;

	for(size_t i = 0; i < n; i++){
		ProcessControlBlock_t *pcb = (ProcessControlBlock_t *)dyn_array_at(ready_queue, i);

		// Make sure CPU time starts when process arrives
		if(current_time < pcb->arrival){
			current_time = pcb->arrival;
		}

		uint32_t start_time = (current_time > pcb->arrival) ? current_time : pcb->arrival;
        uint32_t waiting_time = start_time - pcb->arrival;
        total_waiting += waiting_time;

		// Run for full burst time
		current_time += pcb->remaining_burst_time;
		uint32_t turnaround_time = current_time - pcb->arrival;
		total_turnaround += turnaround_time;
	}

	result->average_waiting_time = (float)total_waiting / n;
	result->average_turnaround_time = (float)total_turnaround / n;
	result->total_run_time = current_time;

	return true;
}

bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) 
{
	if (ready_queue == NULL || result == NULL) {
		return false;
	}

	size_t n = dyn_array_size(ready_queue);
	if (n == 0) {
		return false;
	}

	//variables to hold the time
	uint32_t total_waiting = 0;
	uint32_t total_turnaround = 0;
	uint32_t current = 0;
	uint32_t total_run = 0;

	dyn_array_sort(ready_queue, compare_arrival_time);


	for (size_t i = 0; i < n; i++) {
		//needs to get sorted everytime before grabbing the next element because a new process might have arrived
		dyn_array_sort(ready_queue, compare_arrival_time);
		//gets the specific element at i
		ProcessControlBlock_t* pcb = (ProcessControlBlock_t*)dyn_array_at(ready_queue, i);
		if (i + 1 < n) {
			ProcessControlBlock_t* pcb2 = (ProcessControlBlock_t*)dyn_array_at(ready_queue, i + 1);
			//seeing if the first process and second process arrivals are equal then it will choose to run the one that hasn't ran before if one hasn't
			if (pcb->started && pcb->arrival == pcb2->arrival && pcb2->started == false) {
				pcb = pcb2;
			}
		}
		
		if (pcb->remaining_burst_time > quantum) {
			if (current < pcb->arrival) {
				current = pcb->arrival;
			}

			//adds the total time to waiting before adding the current burst
			total_waiting += current - pcb ->arrival;
			//updates current after the process is ran
			current += quantum;
			total_turnaround += current - pcb->arrival;
			total_run += quantum;
			pcb->remaining_burst_time -= quantum;
			//gets pushed to the back at the current time
			pcb->arrival = current;
			pcb->started = true;
		}
		else {
			if (current < pcb->arrival) {
				current = pcb->arrival;
			}
			total_waiting += current - pcb->arrival;
			current += pcb->remaining_burst_time;
			total_turnaround += current - pcb ->arrival;
			total_run += pcb->remaining_burst_time;
		}

	}

	result->average_waiting_time = (float)total_waiting / n;
	result->average_turnaround_time = (float)total_turnaround / n;
	result->total_run_time = total_run;
	
	return true;
}

dyn_array_t *load_process_control_blocks(const char *input_file) 
{
	if(!input_file){
		return NULL;
	}

	// Open binary file for reading
	int fd = open(input_file, O_RDONLY);
	if (fd == -1) {
		return NULL;
	}

	// Read first 32-bit integer, finds number of processes
	uint32_t num_processes;
	if (read(fd, &num_processes, sizeof(uint32_t)) != sizeof(uint32_t)) {
		close(fd);
		return NULL;
	}

	dyn_array_t *ready_queue = dyn_array_create(num_processes, sizeof(ProcessControlBlock_t), NULL);
	if (!ready_queue) {
		close(fd);
		return NULL;
	}

	// Read data for each process
	for(size_t i = 0; i < num_processes; i++){
		ProcessControlBlock_t pcb;

		// Burst time, priority, and arrival time reading
		if ((read(fd, &pcb.remaining_burst_time, sizeof(uint32_t)) != sizeof(uint32_t)) ||
		    (read(fd, &pcb.priority, sizeof(uint32_t)) != sizeof(uint32_t)) ||
		    (read(fd, &pcb.arrival, sizeof(uint32_t)) != sizeof(uint32_t))) {

			// Reading failed
			dyn_array_destroy(ready_queue);
			close(fd);
			return NULL;
		}

		pcb.started = false;

		// Store PCB in array
		if(!dyn_array_push_back(ready_queue, &pcb)) {
			dyn_array_destroy(ready_queue);
			close(fd);
			return NULL;
		}
	}

	close(fd);

	// Sort processes by arrival time
	dyn_array_sort(ready_queue, compare_arrival_time);
	return ready_queue;
}

bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	if (ready_queue == NULL || result == NULL) {
		return false;
	}

	size_t n = dyn_array_size(ready_queue);
	if (n == 0) {
		return false;
	}

	//variables to hold the time
	uint32_t total_waiting = 0;
	uint32_t total_turnaround = 0;

	dyn_array_sort(ready_queue, compare_remaining_time);


	for (size_t i = 0; i < n; i++) {

		//gets the specific element at i
		ProcessControlBlock_t* pcb = (ProcessControlBlock_t*)dyn_array_at(ready_queue, i);

		total_waiting += total_turnaround;
		total_turnaround += pcb->remaining_burst_time;

	}

	result->average_waiting_time = (float)total_waiting / n;
	result->average_turnaround_time = (float)total_turnaround / n;
	result->total_run_time = total_turnaround;

	return true;
}
