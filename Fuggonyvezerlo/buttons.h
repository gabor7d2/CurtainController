/*
 * buttons.h
 *
 * Created: 2022. 05. 24. 19:35:55
 *  Author: gabor
 */

//////////////////////////////////////////////////////////////////////////
///
/// Buttons module
///
/// Polls the buttons every 50 ms with TIMER3, and adds the detected
/// button change events to a queue which can then be processed in 
/// the main loop of the program.
///
/// The 50 ms polling interval takes care of debouncing the switch too.
///
/// Call Buttons_Init() before using this module.
///
//////////////////////////////////////////////////////////////////////////

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdbool.h>
#include <avr/io.h>
#include "task_scheduler.h"
#include "utils.h"

#define BTN1_Dir	DDRD		// Button 1 data direction register
#define BTN1_Pin	PIND		// Button 1 input pin register
#define BTN1_Port	PORTD		// Button 1 output port register
#define BTN1		PD7			// Button 1 pin

#define BTN2_Dir	DDRD		// Button 2 data direction register
#define BTN2_Pin	PIND		// Button 2 input pin register
#define BTN2_Port	PORTD		// Button 2 output port register
#define BTN2		PD6			// Button 2 pin

#define BTN3_Dir	DDRD		// Button 3 data direction register
#define BTN3_Pin	PIND		// Button 3 input pin register
#define BTN3_Port	PORTD		// Button 3 output port register
#define BTN3		PD5			// Button 3 pin

#define BUTTONS_CHANGE_QUEUE_SIZE 10

void (*btn_change_handler)(uint8_t id, bool pressed);

bool Buttons_UpdateButtonStates(uint8_t id);

/**
 * Initializes the button control logic, sets pins as input, activates pullup resistors.
 */
void Buttons_Init(void (*handler)(uint8_t id, bool pressed)) {
    // store function pointer
    btn_change_handler = handler;

    // set pins as input
    clear_bit(BTN1_Dir, BTN1);
    clear_bit(BTN2_Dir, BTN2);
    clear_bit(BTN3_Dir, BTN3);

    // activate internal pullup resistors (writing 1 to port bit activates it)
    set_bit(BTN1_Port, BTN1);
    set_bit(BTN2_Port, BTN2);
    set_bit(BTN3_Port, BTN3);

    Task t = {.id = 1, .period = 50, .func = Buttons_UpdateButtonStates};
    TaskScheduler_Schedule(t);
}

// latest state of buttons
volatile bool btn1 = false, btn2 = false, btn3 = false;
// track whether state of buttons have changed since processing the changes
volatile bool chgbtn1 = false, chgbtn2 = false, chgbtn3 = false;

/*
 * Function that gets called every 50 ms by the TaskScheduler to update button states.
 */
bool Buttons_UpdateButtonStates(uint8_t id) {
	if (bit_is_clear(BTN1_Pin, BTN1) != btn1) {
		btn1 = !btn1;
        chgbtn1 = true;
	}
    
	if (bit_is_clear(BTN2_Pin, BTN2) != btn2) {
		btn2 = !btn2;
        chgbtn2 = true;
	}
    
	if (bit_is_clear(BTN3_Pin, BTN3) != btn3) {
		btn3 = !btn3;
        chgbtn3 = true;
	}

    // keep task running
    return true;
}

/**
 * Processes the queued button changes, calling the
 * function registered with Buttons_Init for each button change.
 * Should be called continuously from the main loop.
 */
void Buttons_ProcessButtonChanges() {
    if (chgbtn1) {
        btn_change_handler(1, btn1);
        chgbtn1 = false;
    }
    if (chgbtn2) {
        btn_change_handler(2, btn2);
        chgbtn2 = false;
    }
    if (chgbtn3) {
        btn_change_handler(3, btn3);
        chgbtn3 = false;
    }
}

#endif /* BUTTONS_H_ */