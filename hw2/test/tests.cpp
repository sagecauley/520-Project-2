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

dyn_array_t* makePCB()
{
	dyn_array_t* pcb = dyn_array_create(4, sizeof(ProcessControlBlock_t), NULL);
	
	((ProcessControlBlock_t)((pcb->array)[0]))-> remaining_burst_time = 15;
	((ProcessControlBlock_t)((pcb->array)[1]))-> remaining_burst_time = 10;
	((ProcessControlBlock_t)((pcb->array)[2]))-> remaining_burst_time = 5;
	((ProcessControlBlock_t)((pcb->array)[3]))-> remaining_burst_time = 20;
	((ProcessControlBlock_t)((pcb->array)[0]))-> priority = 0;
	((ProcessControlBlock_t)((pcb->array)[1]))-> priority = 0;
	((ProcessControlBlock_t)((pcb->array)[2]))-> priority = 0;
	((ProcessControlBlock_t)((pcb->array)[3]))-> priority = 0;
	((ProcessControlBlock_t)((pcb->array)[0]))-> arrival = 0;
	((ProcessControlBlock_t)((pcb->array)[1]))-> arrival = 1;
	((ProcessControlBlock_t)((pcb->array)[2]))-> arrival = 2;
	((ProcessControlBlock_t)((pcb->array)[3]))-> arrival = 3;
	((ProcessControlBlock_t)((pcb->array)[0]))-> started = 0;
	((ProcessControlBlock_t)((pcb->array)[1]))-> started = 0;
	((ProcessControlBlock_t)((pcb->array)[2]))-> started = 0;
	((ProcessControlBlock_t)((pcb->array)[3]))-> started = 0;
	/*
	ProcessControlBlock_t ctrb = {15, 0, 0, 0};
	memcpy(((pcb->array)+0), ctrb, sizeof(ProcessControlBlock_t)); 
	ProcessControlBlock_t ctrb2 = {10, 0, 1, 0};
	memcpy(((pcb->array)+1), ctrb2, sizeof(ProcessControlBlock_t)); 
	ProcessControlBlock_t ctrb3 = {5, 0, 2, 0};
	memcpy(((pcb->array)+2), ctrb, sizeof(ProcessControlBlock_t)); 
	ProcessControlBlock_t ctrb4 = {20, 0, 3, 0};
	memcpy(((pcb->array)+0), &ctrb2, sizeof(ProcessControlBlock_t)); */
	return pcb;
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	::testing::AddGlobalTestEnvironment(new GradeEnvironment);
	return RUN_ALL_TESTS();
}
TEST(PCB, BadData)
{
	//Test
	ASSERT_EQ(NULL, load_process_control_blocks(NULL));
	ASSERT_EQ(NULL, load_process_control_blocks("ThisFileDoesNotExistHelloWorld"));
}
TEST(PCB, GoodData)
{
	dyn_array_t* ptr = load_process_control_blocks("pcb.bin");
	ASSERT_NE((dyn_array_t*)NULL, ptr);

	free(ptr);
}
TEST(FCFC, BadData)
{
	dyn_array_t * arPtr = makePCB();
	ScheduleResult_t* schPtr = (ScheduleResult_t*)malloc(sizeof(ScheduleResult_t));
	ASSERT_EQ(false, first_come_first_serve(NULL, schPtr));
	ASSERT_EQ(false, first_come_first_serve(arPtr, NULL));
	//ASSERT_EQ(false, first_come_first_serve(arrPtr, NULL))
}
TEST(FCFS, GoodData)
{
	dyn_array_t * arPtr = makePCB();
	ScheduleResult_t* schPtr = (ScheduleResult_t*)malloc(sizeof(ScheduleResult_t));
	ASSERT_NE(nullptr, first_come_first_serve(arPtr, schPtr));
}