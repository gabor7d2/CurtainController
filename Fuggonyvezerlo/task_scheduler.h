/*
 * scheduler.h
 *
 * Created: 2022. 05. 25. 23:35:53
 *  Author: gabor
 */

//////////////////////////////////////////////////////////////////////////
/// 
/// Task scheduler module.
///
/// Schedule and deschedule tasks which run at the period you specify.
/// A task consists of an id, a period and a function to call. A scheduled
/// task's function will be called every 'period * 1 ms'. Thus, the smallest
/// period is 1ms, and the largest is 65.535 s (> 1 min).
/// The function receives the task id as it's first parameter and it should return a
/// boolean that determines if the task should be kept running (0 == deactivate, 1 == keep running)
///
/// Uses TIMER0 for queuing tasks.
///
/// Call TaskScheduler_Init() before using this module.
///
//////////////////////////////////////////////////////////////////////////

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdbool.h>
#include "utils.h"

#define SCHEDULER_POSSIBLE_TASKS 10
#define SCHEDULER_TASK_QUEUE_SIZE 20

/**
 * Struct to hold data of a task.
 */
typedef struct Task {
    // Id of the task
	uint8_t id;
    // Period of the task, the task will be run every period * 1ms, if 0, the task will be run as soon as possible
	uint16_t period;
    // What function to call, the function receives the task id as a parameter
    // and should return a boolean that determines if the task should be kept running (0 == deactivate, 1 == keep running)
	bool (*func)(uint8_t);
} Task;

volatile Task tasks[SCHEDULER_POSSIBLE_TASKS];
volatile Task* volatile taskQueue[SCHEDULER_TASK_QUEUE_SIZE];

// idx0 is for keeping track of the next free space in the taskQueue, idx1 is for keeping track which task is the next for processing
volatile uint8_t taskidx0 = 0, taskidx1 = 0;

// counter for counting timer ticks.
volatile uint16_t counter = 0;

/**
 * Initializes the Task Scheduler, starting the timer and deactivating all tasks.
 */
void TaskScheduler_Init() {
	// Setup TIMER 0: 8-bit, count upto 250 with /64 prescaler = reset every 1 ms
	TCCR0A = 0x2;			// Disconnect OC0A/OC0B pins, set WGM0[1:0] to 10
	TCCR0B = 0b00000011;	// set WGM0[2] to 0 (count upto the value in OCR0A), set prescaler to 64
	TCNT0 = 0;				// clear timer
	OCR0A = 250;			// set top value to count upto
	TIMSK0 = 0x2;           // enable interrupt when TCNT0 == OCR0A
	// https://eleccelerator.com/avr-timer-calculator/
	
	// Set all tasks to inactive
	for (uint8_t i = 0; i < SCHEDULER_POSSIBLE_TASKS; i++)
	{
		tasks[i].id = 255;
	}
}

/**
* Interrupt vector to queue due tasks, runs every 1 ms.
*/
ISR(TIMER0_COMPA_vect) {
	// go through all tasks that are active (id != 255)
	for (uint8_t i = 0; i < SCHEDULER_POSSIBLE_TASKS; i++)
	{
		if (tasks[i].id != 255 && tasks[i].period != 0) {
			// if the time specified in the task has elapsed
			if (counter % tasks[i].period == 0) {
				if (taskidx0 >= SCHEDULER_TASK_QUEUE_SIZE) taskidx0 = 0;
				// add a pointer of the task to the queue
				taskQueue[taskidx0++] = &tasks[i];
			}
		}
	}
	counter++;
}

/**
 * Schedule the specified task. Returns true if the task was scheduled,
 * and false if there was no room for the task.
 */
bool TaskScheduler_Schedule(Task t) {
	// search for the first task that is not active (id == 255)
	for (uint8_t i = 0; i < SCHEDULER_POSSIBLE_TASKS; i++)
	{
		if (tasks[i].id == 255) {
			tasks[i] = t;
			return true;
		}
	}
    return false;
}

/**
 * Deactivate the task(s) with the specified id.
 */
void TaskScheduler_Deschedule(uint8_t id) {
	// search for all tasks with this id and deactivate them.
	for (uint8_t i = 0; i < SCHEDULER_POSSIBLE_TASKS; i++)
	{
		if (tasks[i].id == id) {
			tasks[i].id = 255;
		}
	}
}

/**
 * Process tasks on the queue. Should be called continuously from the main loop.
 */
void TaskScheduler_ProcessTasks() {
	// go through task queue
	while (taskidx0 != taskidx1) {
		if (taskidx1 >= SCHEDULER_TASK_QUEUE_SIZE) taskidx1 = 0;
        
        // if the function of the task returns true, keep it scheduled, else deactivate it
		bool ret = taskQueue[taskidx1]->func(taskQueue[taskidx1]->id);
		if (!ret) taskQueue[taskidx1]->id = 255;
		taskidx1++;
	}
    
    // Go through immediate tasks
    for (uint8_t i = 0; i < SCHEDULER_POSSIBLE_TASKS; i++)
	{
		if (tasks[i].id != 255 && tasks[i].period == 0) {
            bool ret = tasks[i].func(tasks[i].id);
            if (!ret) tasks[i].id = 255;
		}
	}
}

#endif /* SCHEDULER_H_ */