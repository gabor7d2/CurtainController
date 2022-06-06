/*
 * main.c
 *
 * Created: 3/7/2022 10:24:24 PM
 *  Author: gabor
 */

#include <avr/interrupt.h>
#include "task_scheduler.h"
#include "curtain_controller.h"

int main() {
    // main init
    TaskScheduler_Init();
    CurtainController_Init();

    // enable global interrupts
    sei();

    while (1) {
        TaskScheduler_ProcessTasks();
    }
}
