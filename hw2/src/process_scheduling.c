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
	//sorts the array based on arrival time
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


bool shortest_job_first(dyn_array_t* ready_queue, ScheduleResult_t* result)
{
	//Need to use dyn_array_sort just not sure how atm
	if (ready_queue == NULL || result == NULL) {
		return false;
	}
	dyn_array_sort(ready_queue, compare_arrival_time);
	size_t maxIndexSched = 0;
	ProcessControlBlock_t* currBlock = dyn_array_at(ready_queue, 0);
	uint32_t currTime = currBlock->arrival + currBlock->remaining_burst_time;
	int totalWatingTime = 0;
	int totalTurnaroundTime = currBlock->remaining_burst_time;
	int totalRunTime = currBlock->remaining_burst_time;
	currBlock->started = 1;
	size_t currInd = 0;
	size_t schedInd = 0;
	uint32_t shortestTime = __UINT32_MAX__;
	size_t numPCB = dyn_array_size(ready_queue) - 1;
	for(size_t i = 0; i < numPCB; i++)
	{
		currInd = 0;
		schedInd = 0;
		shortestTime = __UINT32_MAX__;
		currBlock = dyn_array_at(ready_queue, currInd);
		while(currInd < numPCB && currBlock->arrival <= currTime)
		{
			if(currBlock->remaining_burst_time < shortestTime  && currBlock->started == 0)
			{
				schedInd = currInd;
				shortestTime = currBlock->remaining_burst_time;
			}
				
			currInd++;
			currBlock = dyn_array_at(ready_queue, currInd);
		}	
		
		currBlock = dyn_array_at(ready_queue, schedInd);
		if(currBlock->started == 1)
			currBlock = dyn_array_at(ready_queue, maxIndexSched+1);
		if(currBlock ->arrival <= currTime)
		{
			totalWatingTime += currTime - (currBlock->arrival);
			totalTurnaroundTime += (currTime - currBlock->arrival) + currBlock->remaining_burst_time;
			currTime += currBlock->remaining_burst_time;	
		}
		else
		{
			totalTurnaroundTime+= currBlock->remaining_burst_time;
			
			// totalWatingTime += 0; This process did not wait at all
			currTime = currBlock->arrival + currBlock->remaining_burst_time;
		}
		//We ignore wait time in the same way as if this arrived at time 2 with nothing before it
		totalRunTime+= currBlock->remaining_burst_time;
		currBlock ->started = 1;
		maxIndexSched = maxIndexSched > schedInd ? maxIndexSched : schedInd;
	}
	result->average_waiting_time = (float) totalWatingTime / (numPCB+1);
	result->average_turnaround_time = (float)totalTurnaroundTime / (numPCB+1);
	result->total_run_time = currTime;

	/*
	int shortestJob = INT32_MAX;
	ProcessControlBlock_t* currBlock = NULL;
	int startTime = 0;
	float totalWait = 0;
	size_t index = 0;
	size_t numQueue = dyn_array_size(ready_queue);
	for(size_t i = 0; i < numQueue; i++)
	{
		for(size_t j = 0; j < numQueue; j++)
		{
			currBlock = (ProcessControlBlock_t*)dyn_array_at(ready_queue, j);
			if(i == 0 && j == 0)
				startTime = currBlock->arrival;
			if(currBlock->remaining_burst_time < shortestJob && currBlock ->started == 0)
			{
				shortestJob = currBlock->remaining_burst_time;
				index = j;
			}
		}
		if(currBlock == NULL)
			return false;
		totalWait += currBlock -> remaining_burst_time;
	}*/
	return true;
}

bool priority(dyn_array_t *ready_queue, ScheduleResult_t *result) 
{
    if (ready_queue == NULL || result == NULL) {
        return false;
    }

    size_t n = dyn_array_size(ready_queue);
    if (n == 0) {
        return false;
    }

    // Sort by arrival time first! 
    dyn_array_sort(ready_queue, compare_arrival_time);

    uint32_t current_time = 0;
    uint32_t total_waiting = 0;
    uint32_t total_turnaround = 0;
    size_t completed = 0;
    bool processed[n];
    memset(processed, 0, sizeof(processed));

    while (completed < n) {
        ProcessControlBlock_t *highest_priority_pcb = NULL;
        size_t highest_priority_index = -1;

        // Find highest-priority process that has arrived
        for (size_t i = 0; i < n; i++) {
            ProcessControlBlock_t *pcb = (ProcessControlBlock_t *)dyn_array_at(ready_queue, i);
            if (!processed[i] && pcb->arrival <= current_time) {
                if (highest_priority_pcb == NULL || pcb->priority < highest_priority_pcb->priority) {
                    highest_priority_pcb = pcb;
                    highest_priority_index = i;
                }
            }
        }

        if (highest_priority_pcb == NULL) {
            // Advance to next available process
            for (size_t i = 0; i < n; i++) {
                if (!processed[i]) {
                    current_time = ((ProcessControlBlock_t *)dyn_array_at(ready_queue, i))->arrival;
                    break;
                }
            }
            continue;
        }

        // Mark process as completed and update timeline
        processed[highest_priority_index] = true;
        completed++;

        if (current_time < highest_priority_pcb->arrival) {
            current_time = highest_priority_pcb->arrival;
        }

        uint32_t start_time = current_time;
        uint32_t waiting_time = start_time - highest_priority_pcb->arrival;
        total_waiting += waiting_time;

        current_time += highest_priority_pcb->remaining_burst_time;
        uint32_t turnaround_time = current_time - highest_priority_pcb->arrival;
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

	//sorting the array based on arrival time
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
			free(pcb2);
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

		free(pcb);

	}

	result->average_waiting_time = (float)total_waiting / n;
	result->average_turnaround_time = (float)total_turnaround / n;
	result->total_run_time = total_run;
	//dyn_array_destroy(ready_queue);
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
    
    uint32_t total_waiting = 0;
    uint32_t total_turnaround = 0;
    uint32_t current_time = 0;
	uint32_t run_time = 0;
	//keeps track of the time when no process is there to subtract from the total turnaround time since nothing is waiting or running
	uint32_t nothing_happens = 0;

    // Sort the array based on arrival time
    dyn_array_sort(ready_queue, compare_arrival_time);

    bool completed[n];  // Tracks if a process is completed
    size_t number_completed = 0;

    uint32_t remaining_time[n]; 
	uint32_t burst_time[n];

    // Initialize the tracking arrays
    for (size_t i = 0; i < n; i++) {
        ProcessControlBlock_t* pcb = (ProcessControlBlock_t*)dyn_array_at(ready_queue, i);
        remaining_time[i] = pcb->remaining_burst_time;
		burst_time[i] = pcb->remaining_burst_time;
        completed[i] = false; 
    }

    // Loops until all processes are completed
    while (number_completed < n) {
        int shortest_index = -1;
        uint32_t shortest_time = UINT32_MAX;

        // Finds the process with the shortest remaining time that has arrived
        for (size_t i = 0; i < n; i++) {
            ProcessControlBlock_t* pcb = (ProcessControlBlock_t*)dyn_array_at(ready_queue, i);
            if (pcb->arrival <= current_time && !completed[i] && remaining_time[i] < shortest_time) {
                shortest_index = i;
                shortest_time = remaining_time[i];
            }
        }

        // No process avaliable move time forward
        if (shortest_index == -1) {
            current_time++;
			nothing_happens++;
            continue;
        }

        // Execute the shortest process for 1 tick
        remaining_time[shortest_index]--;
        current_time++;

        // If the process is completed
        if (remaining_time[shortest_index] == 0) {
            completed[shortest_index] = true;
            number_completed++;

            ProcessControlBlock_t* completed_pcb = (ProcessControlBlock_t*)dyn_array_at(ready_queue, shortest_index);
            uint32_t turnaround_time = current_time - completed_pcb->arrival;
            uint32_t waiting_time = turnaround_time - burst_time[shortest_index];

            total_turnaround += turnaround_time;
            total_waiting += waiting_time;
			run_time += burst_time[shortest_index];
        }
    }

    // Store results
	total_turnaround -= nothing_happens;
    result->average_waiting_time = (float)total_waiting / n;
    result->average_turnaround_time = (float)total_turnaround / n;
    result->total_run_time = run_time; 

    return true;
}
