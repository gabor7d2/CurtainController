#include "task_scheduler.h"
#include <avr/io.h>
#include <avr/interrupt.h>

volatile Task tasks[SCHEDULER_POSSIBLE_TASKS];
volatile Task* volatile taskQueue[SCHEDULER_TASK_QUEUE_SIZE];

// idx0 is for keeping track of the next free space in the taskQueue, idx1 is for keeping track which task is the next for processing
volatile uint8_t taskidx0 = 0, taskidx1 = 0;

// whether the task scheduler is initialized
volatile bool initialized = false;

void TaskScheduler_Init() {
    // only allow initializing once
    if (initialized) return;
    initialized = true;
    
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
			if (tasks[i].counter % tasks[i].period == 0) {
				if (taskidx0 >= SCHEDULER_TASK_QUEUE_SIZE) taskidx0 = 0;
				// add a pointer of the task to the queue
				taskQueue[taskidx0++] = &tasks[i];
				tasks[i].counter = 1;
			} else tasks[i].counter++;
		}
	}
}

bool TaskScheduler_Schedule(uint8_t id, uint16_t period, void (*func)(uint8_t)) {
	// search for the first task that is not active (id == 255)
	for (uint8_t i = 0; i < SCHEDULER_POSSIBLE_TASKS; i++)
	{
		if (tasks[i].id == 255) {
			tasks[i].id = id;
			tasks[i].period = period;
			tasks[i].counter = 1;
			tasks[i].func = func;
			return true;
		}
	}
    return false;
}

void TaskScheduler_Deschedule(uint8_t id) {
	// search for all tasks with this id and deactivate them.
	for (uint8_t i = 0; i < SCHEDULER_POSSIBLE_TASKS; i++)
	{
		if (tasks[i].id == id) {
			tasks[i].id = 255;
		}
	}
}

void TaskScheduler_ProcessTasks() {
	// go through task queue
	while (taskidx0 != taskidx1) {
		if (taskidx1 >= SCHEDULER_TASK_QUEUE_SIZE) taskidx1 = 0;
        
        // call the tasks's function, if it is not deactivated (id != 255)
		// the task might be deactivated after there are other copies of it
		// on the queue (the time specified on the task has already elapsed
		// multiple times while it's function has been running for example)
		if (taskQueue[taskidx1]->id != 255) {
			taskQueue[taskidx1]->func(taskQueue[taskidx1]->id);
		}
		taskidx1++;
	}
    
    // Go through immediate tasks
    for (uint8_t i = 0; i < SCHEDULER_POSSIBLE_TASKS; i++)
	{
		if (tasks[i].id != 255 && tasks[i].period == 0) {
            tasks[i].func(tasks[i].id);
		}
	}
}

