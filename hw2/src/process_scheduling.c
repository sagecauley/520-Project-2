#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

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

bool first_come_first_serve(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	if (ready_queue == NULL || result == NULL) {
		return false;
	}

	int n = dyn_array_size(ready_queue);
	if (n == 0) {
		return false;
	}

	int current = 0;
	int total_waiting = 0;
	int total_running = 0;

	for (size_t i = 0; i < n; i++) {

		void* dyn_array_at(const dyn_array_t* const dyn_array, const size_t index);
		ProcessControlBlock_t *pcb = (ProcessControlBlock_t*)dyn_array_at(ready_queue, i);

		if (current < pcb -> arrival) {
			current = pcb -> arrival;
		}

		int waiting_time = current - pcb->arrival;
		total_waiting += waiting_time;

		current += pcb->remaining_burst_time;

		int running_time = current - pcb->arrival;

		total_running += running_time;
	}

	result->average_waiting_time = (uint32_t)total_waiting / n;
	result->average_turnaround_time = (uint32_t)total_running / n;
	result->total_run_time = current;

	return true;
}

bool shortest_job_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	//Need to use dyn_array_sort just not sure how atm
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}

bool priority(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}

bool round_robin(dyn_array_t *ready_queue, ScheduleResult_t *result, size_t quantum) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	UNUSED(quantum);
	return false;
}

dyn_array_t *load_process_control_blocks(const char *input_file) 
{
	UNUSED(input_file);
	return NULL;
}

bool shortest_remaining_time_first(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
	UNUSED(ready_queue);
	UNUSED(result);
	return false;
}
