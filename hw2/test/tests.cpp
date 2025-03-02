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

// FCFC Testing --
TEST(FCFC, BadData)
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
TEST(Priority, BasicOrder)
{
    dyn_array_t *ready_queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);
    ASSERT_NE(ready_queue, nullptr);

    ProcessControlBlock_t pcb1 = {5, 2, 0, false}; 
    ProcessControlBlock_t pcb2 = {3, 1, 1, false}; 
    ProcessControlBlock_t pcb3 = {2, 3, 2, false}; 

    dyn_array_push_back(ready_queue, &pcb1);
    dyn_array_push_back(ready_queue, &pcb2);
    dyn_array_push_back(ready_queue, &pcb3);

    ScheduleResult_t result;
    ASSERT_TRUE(priority(ready_queue, &result));

    EXPECT_NEAR(result.average_waiting_time, 3.67, 0.1); 
    EXPECT_NEAR(result.average_turnaround_time, 7.0, 0.1); 
    EXPECT_EQ(result.total_run_time, static_cast<unsigned long>(11)); 

    dyn_array_destroy(ready_queue);
}
TEST(Priority, IdenticalPriorities)
{
    dyn_array_t *ready_queue = dyn_array_create(3, sizeof(ProcessControlBlock_t), NULL);
    ASSERT_NE(ready_queue, nullptr);

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
