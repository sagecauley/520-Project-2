#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>
#include "gtest/gtest.h"
#include "../include/processing_scheduling.h"

// Using a C library requires extern "C" to prevent function mangling
extern "C"
{
#include <dyn_array.h>
}

#define NUM_PCB 30
#define QUANTUM 5 // Used for Robin Round for process as the run time limit


unsigned int score;
unsigned int total;

class GradeEnvironment : public testing::Environment
{
	public:
		virtual void SetUp()
		{
			score = 0;
			total = 210;
		}

		virtual void TearDown()
		{
			::testing::Test::RecordProperty("points_given", score);
			::testing::Test::RecordProperty("points_total", total);
			std::cout << "SCORE: " << score << '/' << total << std::endl;
		}
};

dyn_array* makePCB()
{
	dyn_array_t* pcb = dyn_array_create(0, sizeof(ProcessControlBlock_t), NULL);
	ProcessControlBlock_t ctrb4 = {20, 0, 3, 0};
	dyn_array_push_front(pcb, &ctrb4);
	ProcessControlBlock_t ctrb3 = {5, 0, 2, 0};
	dyn_array_push_front(pcb, &ctrb3);
	ProcessControlBlock_t ctrb2 = {10, 0, 1, 0};
	dyn_array_push_front(pcb, &ctrb2);
	ProcessControlBlock_t ctrb = {15, 0, 0, 0};
	dyn_array_push_front(pcb, &ctrb);
	return pcb;
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	::testing::AddGlobalTestEnvironment(new GradeEnvironment);
	return RUN_ALL_TESTS();
}

// PCB Testing --
TEST(PCB, BadData)
{
	//Test
	ASSERT_EQ(NULL, load_process_control_blocks(NULL));
	ASSERT_EQ(NULL, load_process_control_blocks("ThisFileDoesNotExistHelloWorld"));
}
TEST(PCB, GoodData)
{
	dyn_array_t* ptr = load_process_control_blocks("../pcb.bin");
	dyn_array_t * arPtr = makePCB();
	ASSERT_NE((dyn_array_t*)NULL, ptr);
	ASSERT_EQ(dyn_array_size(ptr), dyn_array_size(arPtr));
	ASSERT_EQ(dyn_array_data_size(ptr), dyn_array_data_size(arPtr));
	for(size_t i = 0; i < dyn_array_size(ptr); i++)
	{
		ASSERT_EQ(((ProcessControlBlock_t*)dyn_array_at(ptr, i))->remaining_burst_time, ((ProcessControlBlock_t*)dyn_array_at(arPtr, i))->remaining_burst_time);
	}
	free(ptr);
}

// FCFS Testing --
TEST(FCFS, BadData)
{
	dyn_array_t * arPtr = dyn_array_create(4, sizeof(ProcessControlBlock_t), NULL);

	ScheduleResult_t* schPtr = (ScheduleResult_t*)malloc(sizeof(ScheduleResult_t));
	ASSERT_EQ(false, first_come_first_serve(NULL, schPtr));
	ASSERT_EQ(false, first_come_first_serve(arPtr, NULL));
}
TEST(FCFS, GoodData)
{
	dyn_array_t * arPtr = makePCB();
	ScheduleResult_t* schPtr = (ScheduleResult_t*)malloc(sizeof(ScheduleResult_t));
	ASSERT_NE(false, first_come_first_serve(arPtr, schPtr));
	ASSERT_EQ(16, schPtr->average_waiting_time);
	ASSERT_EQ(28.5, schPtr->average_turnaround_time);
	ASSERT_EQ((unsigned long)50, schPtr->total_run_time);
}

// Priority Testing ---
TEST(Priority, IdenticalPriorities)
{
    dyn_array_t *ready_queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);
    ASSERT_NE(ready_queue, nullptr);

	// Burst Time, Priority, Arrival Time
    ProcessControlBlock_t pcb1 = {4, 1, 0, false}; 
    ProcessControlBlock_t pcb2 = {3, 1, 1, false}; 
    ProcessControlBlock_t pcb3 = {2, 1, 2, false}; 

    dyn_array_push_back(ready_queue, &pcb1);
    dyn_array_push_back(ready_queue, &pcb2);
    dyn_array_push_back(ready_queue, &pcb3);

    ScheduleResult_t result;
    ASSERT_TRUE(priority(ready_queue, &result));

    EXPECT_NEAR(result.average_waiting_time, 2.67, 0.1);
    EXPECT_NEAR(result.average_turnaround_time, 5.67, 0.1);
    EXPECT_EQ(result.total_run_time, static_cast<unsigned long>(9)); 

    dyn_array_destroy(ready_queue);
}
TEST(Priority, SingleProcess)
{
    dyn_array_t *ready_queue = dyn_array_create(1, sizeof(ProcessControlBlock_t), NULL);
    ASSERT_NE(ready_queue, nullptr);

	// Burst Time, Priority, Arrival Time
    ProcessControlBlock_t pcb = {4, 1, 0, false}; 
    dyn_array_push_back(ready_queue, &pcb);

    ScheduleResult_t result;
    ASSERT_TRUE(priority(ready_queue, &result));

    EXPECT_EQ(result.average_waiting_time, 0.0);  
    EXPECT_EQ(result.average_turnaround_time, 4.0);  
    EXPECT_EQ(result.total_run_time, static_cast<unsigned long>(4));

    dyn_array_destroy(ready_queue);
}
TEST(Priority, MixedArrivalTimes) {
    dyn_array_t *ready_queue = dyn_array_create(4, sizeof(ProcessControlBlock_t), NULL);
    ASSERT_NE(ready_queue, nullptr);

    // Burst Time, Priority, Arrival Time
    ProcessControlBlock_t pcb1 = {5, 3, 0, false}; 
    ProcessControlBlock_t pcb2 = {3, 1, 2, false}; 
    ProcessControlBlock_t pcb3 = {4, 2, 4, false}; 
    ProcessControlBlock_t pcb4 = {2, 1, 6, false}; 

    dyn_array_push_back(ready_queue, &pcb1);
    dyn_array_push_back(ready_queue, &pcb2);
    dyn_array_push_back(ready_queue, &pcb3);
    dyn_array_push_back(ready_queue, &pcb4);

    ScheduleResult_t result;
    ASSERT_TRUE(priority(ready_queue, &result));

    EXPECT_NEAR(result.average_waiting_time, 2.75, 0.1); 
    EXPECT_NEAR(result.average_turnaround_time, 6.25, 0.1);
    EXPECT_EQ(result.total_run_time, static_cast<unsigned long>(14)); 

    dyn_array_destroy(ready_queue);
}

//-Shortest Job First Testing
/*TEST(shortest_job_first, BadData){
	dyn_array_t * arPtr = dyn_array_create(4, sizeof(ProcessControlBlock_t), NULL);

	ScheduleResult_t* result = (ScheduleResult_t*)malloc(sizeof(ScheduleResult_t));
	ASSERT_EQ(false, shortest_job_first(NULL, result));
	ASSERT_EQ(false, shortest_job_first(arPtr, NULL));
	dyn_array_destroy(arPtr);
	free(result);
}
TEST(shortest_job_first, GoodNumbers){
	dyn_array_t *ready_queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);
	ASSERT_NE(ready_queue, nullptr);

	ScheduleResult_t* result = (ScheduleResult_t*)malloc(sizeof(ScheduleResult_t));
	ASSERT_NE(result, nullptr);
	memset(result, 0, sizeof(ScheduleResult_t));

    ProcessControlBlock_t pcb1 = {5, 0, 0, false}; 
    ProcessControlBlock_t pcb2 = {2, 1, 3, false}; 
    ProcessControlBlock_t pcb3 = {3, 2, 5, false};
	ProcessControlBlock_t pcb4 = {1, 3, 7, false};
	ProcessControlBlock_t pcb5 = {9, 4, 8, false};

	dyn_array_push_back(ready_queue, &pcb1);
    dyn_array_push_back(ready_queue, &pcb2);
    dyn_array_push_back(ready_queue, &pcb3);
	dyn_array_push_back(ready_queue, &pcb4);
	dyn_array_push_back(ready_queue, &pcb5);

	ASSERT_EQ(true, shortest_job_first(ready_queue, result));

	EXPECT_NEAR(result->average_waiting_time, 5.6, 0.1); 
    EXPECT_NEAR(result->average_turnaround_time, 1.6, 0.1); 
	EXPECT_EQ(result->total_run_time, static_cast<unsigned long>(20)); 

	dyn_array_destroy(ready_queue);
    free(result);
}
TEST(shortest_job_first, AllSameBurst){
	dyn_array_t *ready_queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);
	ASSERT_NE(ready_queue, nullptr);

	ScheduleResult_t* result = (ScheduleResult_t*)malloc(sizeof(ScheduleResult_t));
	ASSERT_NE(result, nullptr);
	memset(result, 0, sizeof(ScheduleResult_t));

    ProcessControlBlock_t pcb1 = {3, 0, 0, false}; 
    ProcessControlBlock_t pcb2 = {3, 1, 3, false}; 
    ProcessControlBlock_t pcb3 = {3, 2, 5, false};
	ProcessControlBlock_t pcb4 = {3, 3, 7, false};
	ProcessControlBlock_t pcb5 = {3, 4, 8, false};

	dyn_array_push_back(ready_queue, &pcb1);
    dyn_array_push_back(ready_queue, &pcb2);
    dyn_array_push_back(ready_queue, &pcb3);
	dyn_array_push_back(ready_queue, &pcb4);
	dyn_array_push_back(ready_queue, &pcb5);

	ASSERT_EQ(true, shortest_job_first(ready_queue, result));

	EXPECT_NEAR(result->average_waiting_time, 4.4, 0.1); 
    EXPECT_NEAR(result->average_turnaround_time, 1.4, 0.1); 
	EXPECT_EQ(result->total_run_time, static_cast<unsigned long>(20)); 

	dyn_array_destroy(ready_queue);
    free(result);
}*/

TEST(SRTF, BasicScheduling) {
    dyn_array_t *ready_queue = dyn_array_create(4, sizeof(ProcessControlBlock_t), NULL);
    ASSERT_NE(ready_queue, nullptr);

    // Burst Time, Priority, Arrival Time, Started
    ProcessControlBlock_t pcb1 = {8, 0, 0, false};  
    ProcessControlBlock_t pcb2 = {4, 0, 1, false};  
    ProcessControlBlock_t pcb3 = {2, 0, 2, false};  
    ProcessControlBlock_t pcb4 = {1, 0, 3, false};  

    dyn_array_push_back(ready_queue, &pcb1);
    dyn_array_push_back(ready_queue, &pcb2);
    dyn_array_push_back(ready_queue, &pcb3);
    dyn_array_push_back(ready_queue, &pcb4);

    ScheduleResult_t result;
    ASSERT_TRUE(shortest_remaining_time_first(ready_queue, &result));

    EXPECT_NEAR(result.average_waiting_time, 2.75, 0.1); 
    EXPECT_NEAR(result.average_turnaround_time, 6.5, 0.1);
    EXPECT_EQ(result.total_run_time, static_cast<unsigned long>(15)); 

    dyn_array_destroy(ready_queue);
}

TEST(SRTF, LateShortProcess) {
    dyn_array_t *ready_queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);
    ASSERT_NE(ready_queue, nullptr);

    ProcessControlBlock_t pcb1 = {10, 0, 0, false};  
    ProcessControlBlock_t pcb2 = {2, 0, 3, false};  
    ProcessControlBlock_t pcb3 = {1, 0, 5, false};  

    dyn_array_push_back(ready_queue, &pcb1);
    dyn_array_push_back(ready_queue, &pcb2);
    dyn_array_push_back(ready_queue, &pcb3);

    ScheduleResult_t result;
    ASSERT_TRUE(shortest_remaining_time_first(ready_queue, &result));

    EXPECT_NEAR(result.average_waiting_time, 1, 0.1); 
    EXPECT_NEAR(result.average_turnaround_time, 5.33, 0.1);
    EXPECT_EQ(result.total_run_time, static_cast<unsigned long>(13)); 

    dyn_array_destroy(ready_queue);
}

TEST(SRTF, FirstProcessFinishesBeforeOthersArrive) {
    dyn_array_t *ready_queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);
    ASSERT_NE(ready_queue, nullptr);

    ProcessControlBlock_t pcb1 = {4, 0, 0, false};  
    ProcessControlBlock_t pcb2 = {6, 0, 5, false};  
    ProcessControlBlock_t pcb3 = {3, 0, 7, false};  

    dyn_array_push_back(ready_queue, &pcb1);
    dyn_array_push_back(ready_queue, &pcb2);
    dyn_array_push_back(ready_queue, &pcb3);

    ScheduleResult_t result;
    ASSERT_TRUE(shortest_remaining_time_first(ready_queue, &result));

    EXPECT_NEAR(result.average_waiting_time, 1, 0.1); 
    EXPECT_NEAR(result.average_turnaround_time, 5.0, 0.1);
    EXPECT_EQ(result.total_run_time, static_cast<unsigned long>(13)); 

    dyn_array_destroy(ready_queue);
}