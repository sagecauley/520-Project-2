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

	/*FILE* file = fopen(argv[1], "r");
	if (!file) {
		return EXIT_FAILURE;
	}*/

	ready_queue = load_process_control_blocks(argv[1]);

	if (!ready_queue || dyn_array_size(ready_queue) == 0) {
		return EXIT_FAILURE;
	}

	char* algorithm = argv[2];
	ScheduleResult_t result;

	if (memcmp(algorithm, FCFS, 4) == 0) {
		if (first_come_first_serve(ready_queue, &result))
		{
			printf("FCFS:\n");
			printf("Average Waiting Time: %.2f\n", result.average_waiting_time);
			printf("Average Turnaround Time: %.2f\n", result.average_turnaround_time);
			printf("Total Run Time: %lu\n", result.total_run_time);
		}
		else
		{
			printf("Error in FCFS scheduling\n");
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}
