/*
 * task_scheduler.h
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
/// period is 1ms, and the largest is 65.535 s (> 1 min). If the period is
/// set to 0, the task is called as soon as possible, which may be less than 1ms.
///
/// The called function receives the task id as it's first parameter and it should return a
/// boolean that determines if the task should be kept running (0 == deactivate, 1 == keep running)
///
/// Uses TIMER0 for queuing tasks.
///
/// Call TaskScheduler_Init() before using, however, you can call this init
/// as many times as you want, it will only ever run once.
///
//////////////////////////////////////////////////////////////////////////

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include "utils.h"

#define SCHEDULER_POSSIBLE_TASKS 20
#define SCHEDULER_TASK_QUEUE_SIZE 30

void StartMeasure();

uint8_t GetMeasurement();

/**
 * Struct to hold data of a task.
 */
typedef struct Task {
    // Id of the task
	uint8_t id;
    // Period of the task, the task will be run every period * 1ms, if 0, the task will be run as soon as possible
	uint16_t period;
    // counter for counting ticks this task has been running for.
	uint16_t counter;
    // What function to call, the function receives the task id as a parameter
	void (*func)(uint8_t);
} Task;

/**
 * Initializes the Task Scheduler, starting the timer and deactivating all tasks.
 */
void TaskScheduler_Init();

/**
 * Schedule the specified task. Returns true if the task was scheduled,
 * and false if there was no room for the task.
 */
bool TaskScheduler_Schedule(uint8_t id, uint16_t period, void (*func)(uint8_t));

/**
 * Deactivate the task(s) with the specified id.
 */
void TaskScheduler_Deschedule(uint8_t id);

/**
 * Process tasks on the queue. Should be called continuously from the main loop.
 */
void TaskScheduler_ProcessTasks();

#endif /* SCHEDULER_H_ */