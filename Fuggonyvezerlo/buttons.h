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

typedef struct BtnChange {
	uint8_t id;
	bool pressed;
} BtnChange;

void (*btn_change_handler)(BtnChange);

/**
* Initializes the button control logic.
*/
void Buttons_Init(void (*handler)(BtnChange)) {
	// store function pointers
	btn_change_handler = handler;
	
	// set pins as input
	clear_bit(BTN1_Dir, BTN1);
	clear_bit(BTN2_Dir, BTN2);
	clear_bit(BTN3_Dir, BTN3);
	
	// activate internal pullup resistors (writing 1 to port bit activates it)
	set_bit(BTN1_Port, BTN1);
	set_bit(BTN2_Port, BTN2);
	set_bit(BTN3_Port, BTN3);
	
	// Setup TIMER 3: 16-bit, count upto 12500 with /64 prescaler = reset every 50ms
	TCCR3A = 0;				// Disconnect OC3A/OC3B pins, set WGM3[1:0] to 0
	TCCR3B = 0b00001011;	// set WGM3[3:2] to 01 (count upto the value in OCR3A), set prescaler to 64
	TCNT3 = 0;				// clear timer
	OCR3A = 12500;			// set top value to count upto
	TIMSK3 = 0b00000010;	// enable interrupt when TCNT3 == OCR3A
	// https://eleccelerator.com/avr-timer-calculator/
}

bool btn1 = false, btn2 = false, btn3 = false;
BtnChange btnChangeQueue[BUTTONS_CHANGE_QUEUE_SIZE];
unsigned char btnidx0 = 0, btnidx1 = 0;

/**
* Interrupt vector that runs every 10 ms, adds button pin changes to the queue
*/
ISR(TIMER3_COMPA_vect) {
	if (bit_is_clear(BTN1_Pin, BTN1) != btn1) {
		btn1 = !btn1;
		if (btnidx0 >= BUTTONS_CHANGE_QUEUE_SIZE) btnidx0 = 0;
		btnChangeQueue[btnidx0].id = 1;
		btnChangeQueue[btnidx0].pressed = btn1;
		btnidx0++;
	}
	
	if (bit_is_clear(BTN2_Pin, BTN2) != btn2) {
		btn2 = !btn2;
		if (btnidx0 >= BUTTONS_CHANGE_QUEUE_SIZE) btnidx0 = 0;
		btnChangeQueue[btnidx0].id = 2;
		btnChangeQueue[btnidx0].pressed = btn2;
		btnidx0++;
	}
	
	if (bit_is_clear(BTN3_Pin, BTN3) != btn3) {
		btn3 = !btn3;
		if (btnidx0 >= BUTTONS_CHANGE_QUEUE_SIZE) btnidx0 = 0;
		btnChangeQueue[btnidx0].id = 3;
		btnChangeQueue[btnidx0].pressed = btn3;
		btnidx0++;
	}
}

/**
* Processes the queued button changes, calling the
* function registered with Buttons_Init for each button change.
* Should be called continuously from the main loop.
*/
void Buttons_ProcessButtonChanges() {
	while (btnidx0 != btnidx1) {
		if (btnidx1 >= BUTTONS_CHANGE_QUEUE_SIZE) btnidx1 = 0;
		// get from the end of the queue
		BtnChange change = btnChangeQueue[btnidx1++];
		
		// call the registered function
		btn_change_handler(change);
	}
}

#endif /* BUTTONS_H_ */