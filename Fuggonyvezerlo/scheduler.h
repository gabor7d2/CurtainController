/*
 * scheduler.h
 *
 * Created: 2022. 05. 25. 23:35:53
 *  Author: gabor
 */ 

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdbool.h>
#include "display.h"

#define SCHEDULER_POSSIBLE_TASKS 10
#define SCHEDULER_TASK_QUEUE_SIZE 20

typedef struct Task {
	unsigned char id;
	unsigned int freq;
	bool (*func)(unsigned char);
} Task;

Task tasks[SCHEDULER_POSSIBLE_TASKS];
Task* taskQueue[SCHEDULER_TASK_QUEUE_SIZE];
unsigned char taskidx0 = 0, taskidx1 = 0;

unsigned int counter = 0;

void Scheduler_Init() {
	// Setup TIMER 1: 16-bit, count upto 625 with /256 prescaler = reset every 10ms
	TCCR1A = 0;				// Disconnect OC1A/OC1B pins, set WGM1[1:0] to 0
	TCCR1B = 0b00001100;	// set WGM1[3:2] to 01 (count upto the value in OCR1A), set prescaler to 256
	TCNT1 = 0;				// clear timer
	OCR1A = 625;			// set top value to count upto
	TIMSK1 = 0b00000010;	// enable interrupt when TCNT1 == OCR1A
	// https://eleccelerator.com/avr-timer-calculator/
	
	// Set all tasks to inactive
	for (unsigned char i = 0; i < SCHEDULER_POSSIBLE_TASKS; i++)
	{
		tasks[i].id = 255;
	}
}

/**
* Interrupt vector to queue due tasks
*/
ISR(TIMER1_COMPA_vect) {
	// go through all tasks that are active (id != 255)
	for (unsigned char i = 0; i < SCHEDULER_POSSIBLE_TASKS; i++)
	{
		if (tasks[i].id != 255) {
			// if the time specified in the task has elapsed
			if (counter % tasks[i].freq == 0) {
				if (taskidx0 >= SCHEDULER_TASK_QUEUE_SIZE) taskidx0 = 0;
				// add a task pointer to the queue
				taskQueue[taskidx0++] = &tasks[i];
			}
		}
	}
	counter++;
}

void Scheduler_Schedule(Task t) {
	// search for the first task that is not active (id == 255)
	for (unsigned char i = 0; i < SCHEDULER_POSSIBLE_TASKS; i++)
	{
		if (tasks[i].id == 255) {
			tasks[i] = t;
			break;
		}
	}
}

void Scheduler_ProcessTasks() {
	// go through task queue
	while (taskidx0 != taskidx1) {
		if (taskidx1 >= SCHEDULER_TASK_QUEUE_SIZE) taskidx1 = 0;
		bool ret = taskQueue[taskidx1]->func(taskQueue[taskidx1]->id);
		if (!ret) taskQueue[taskidx1]->id = 255;
		taskidx1++;
	}
}

#endif /* SCHEDULER_H_ */