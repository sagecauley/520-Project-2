#include <stdio.h>
#include <stdlib.h>

#include "dyn_array.h"
#include "processing_scheduling.h"

#define FCFS "FCFS"
#define P "P"
#define RR "RR"
#define SJF "SJF"

// Add and comment your analysis code in this function.
// THIS IS NOT FINISHED.
int main(int argc, char **argv) 
{
	if (argc < 3)
	{
		printf("%s <pcb file> <schedule algorithm> [quantum]\n", argv[0]);
		return EXIT_FAILURE;
	}

	dyn_array_t* ready_queue = NULL;

	//loading the queue up
	ready_queue = load_process_control_blocks(argv[1]);

	if (!ready_queue || dyn_array_size(ready_queue) == 0) {
		printf("Loading Error, try adding ../ to the beginning of the file name\n");
		return EXIT_FAILURE;
	}

	char* algorithm = argv[2];
	ScheduleResult_t result;

	FILE *file = fopen("../README.md", "w");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

	//Checks to see which algoithm it is running
	if (memcmp(algorithm, FCFS, 4) == 0) {
		if (first_come_first_serve(ready_queue, &result))
		{
			printf("FCFS:\n");
			printf("Average Waiting Time: %.2f\n", result.average_waiting_time);
			printf("Average Turnaround Time: %.2f\n", result.average_turnaround_time);
			printf("Total Run Time: %lu\n", result.total_run_time);
			fprintf(file, "FCFS:\n");
    		fprintf(file, "Average Waiting Time: %.2f\n", result.average_waiting_time);
    		fprintf(file, "Average Turnaround Time: %.2f\n", result.average_turnaround_time);
    		fprintf(file, "Total Run Time: %lu\n", result.total_run_time);
		}
		else
		{
			printf("Error in FCFS scheduling\n");
			return EXIT_FAILURE;
		}
	}
	if (memcmp(algorithm, P, 1) == 0) {
		if (priority(ready_queue, &result))
		{
			printf("P:\n");
			printf("Average Waiting Time: %.2f\n", result.average_waiting_time);
			printf("Average Turnaround Time: %.2f\n", result.average_turnaround_time);
			printf("Total Run Time: %lu\n", result.total_run_time);
			fprintf(file, "P:\n");
    		fprintf(file, "Average Waiting Time: %.2f\n", result.average_waiting_time);
    		fprintf(file, "Average Turnaround Time: %.2f\n", result.average_turnaround_time);
    		fprintf(file, "Total Run Time: %lu\n", result.total_run_time);
		}
		else
		{
			printf("Error in P scheduling\n");
			return EXIT_FAILURE;
		}
	}
	if (memcmp(algorithm, RR, 2) == 0) {
		//gets the quantum from argv[3] and is converting it to numeric value
		size_t quantum = 0;
		for (int i = 0; argv[3][i] != '\0'; i++) {
			if (argv[3][i] >= '0' && argv[3][i] <= '9') {
				quantum = quantum * 10 + (argv[3][i] - '0');
			}
			else{
				printf("Invalid quantum value\n");
        		return EXIT_FAILURE;
			}
		}
		if (round_robin(ready_queue, &result, quantum))
		{
			printf("RR:\n");
			printf("Average Waiting Time: %.2f\n", result.average_waiting_time);
			printf("Average Turnaround Time: %.2f\n", result.average_turnaround_time);
			printf("Total Run Time: %lu\n", result.total_run_time);
			fprintf(file, "RR:\n");
    		fprintf(file, "Average Waiting Time: %.2f\n", result.average_waiting_time);
    		fprintf(file, "Average Turnaround Time: %.2f\n", result.average_turnaround_time);
    		fprintf(file, "Total Run Time: %lu\n", result.total_run_time);
		}
		else
		{
			printf("Error in RR scheduling\n");
			return EXIT_FAILURE;
		}
	}
	if (memcmp(algorithm, SJF, 3) == 0) {
		if (shortest_job_first(ready_queue, &result))
		{
			printf("SJF:\n");
			printf("Average Waiting Time: %.2f\n", result.average_waiting_time);
			printf("Average Turnaround Time: %.2f\n", result.average_turnaround_time);
			printf("Total Run Time: %lu\n", result.total_run_time);
			fprintf(file, "SJF:\n");
    		fprintf(file, "Average Waiting Time: %.2f\n", result.average_waiting_time);
    		fprintf(file, "Average Turnaround Time: %.2f\n", result.average_turnaround_time);
    		fprintf(file, "Total Run Time: %lu\n", result.total_run_time);
		}
		else
		{
			printf("Error in SJF scheduling\n");
			return EXIT_FAILURE;
		}
	}
	fclose(file);
	return EXIT_SUCCESS;
}
