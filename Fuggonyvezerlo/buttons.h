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
/// Polls the buttons every 10 ms with TIMER3, and adds the detected
/// button change events to a queue which can then be processed in 
/// the main loop of the program.
///
/// The 10 ms polling interval takes care of debouncing the switch too.
///
/// Call Buttons_Init() before using this module.
///
//////////////////////////////////////////////////////////////////////////

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdbool.h>
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

/**
* Initializes the button control logic.
*/
void Buttons_Init() {
	// set pins as input
	clear_bit(BTN1_Dir, BTN1);
	clear_bit(BTN2_Dir, BTN2);
	clear_bit(BTN3_Dir, BTN3);
	
	// activate internal pullup resistors (writing 1 to port bit activates it)
	set_bit(BTN1_Port, BTN1);
	set_bit(BTN2_Port, BTN2);
	set_bit(BTN3_Port, BTN3);
	
	// Setup TIMER 3: 16-bit, count upto 1250 with /64 prescaler = reset every 10ms
	TCCR3A = 0;				// Disconnect OC3A/OC3B pins, set WGM3[1:0] to 0
	TCCR3B = 0b00001011;	// set WGM3[3:2] to 01 (count upto the value in OCR3A), set prescaler to 64
	TCNT3 = 0;				// clear timer
	OCR3A = 2500;			// set top value to count upto
	TIMSK3 = 0b00000010;	// enable interrupt when TCNT3 == OCR3A
	// https://eleccelerator.com/avr-timer-calculator/
}

bool btn1 = false, btn2 = false, btn3 = false;
typedef struct { unsigned char id; bool pressed; } BtnChange;
BtnChange btnChangeQueue[10];
unsigned char idx0 = 0, idx1 = 0;

/**
* Runs every 10 ms, adds button pin changes to the queue
*/
ISR(TIMER3_COMPA_vect) {
	if (bit_is_clear(BTN1_Pin, BTN1) != btn1) {
		btn1 = !btn1;
		if (idx0 > 9) idx0 = 0;
		btnChangeQueue[idx0].id = 1;
		btnChangeQueue[idx0].pressed = btn1;
		idx0++;
	}
	
	if (bit_is_clear(BTN2_Pin, BTN2) != btn2) {
		btn2 = !btn2;
		if (idx0 > 9) idx0 = 0;
		btnChangeQueue[idx0].id = 2;
		btnChangeQueue[idx0].pressed = btn2;
		idx0++;
	}
	
	if (bit_is_clear(BTN3_Pin, BTN3) != btn3) {
		btn3 = !btn3;
		if (idx0 > 9) idx0 = 0;
		btnChangeQueue[idx0].id = 3;
		btnChangeQueue[idx0].pressed = btn3;
		idx0++;
	}
}

/**
* Gets if there are any unprocessed button changes.
*/
bool Buttons_HasUnprocessedChange() {
	return idx0 != idx1;
}

/**
* Processes the first button change in the queue, calling the
* specified function which corresponds to the pressed/released button.
*/
void Buttons_ProcessChange(void (*b1)(bool), void (*b2)(bool), void (*b3)(bool)) {
	if (idx1 > 9) idx1 = 0;
	// get from the end of the queue
	BtnChange change = btnChangeQueue[idx1++];
	
	// call the appropriate function
	if (change.id == 1) b1(change.pressed);
	if (change.id == 2) b2(change.pressed);
	if (change.id == 3) b3(change.pressed);
}

#endif /* BUTTONS_H_ */