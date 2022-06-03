/*
 * button_controller.h
 *
 * Created: 2022. 05. 24. 19:35:55
 *  Author: gabor
 */

//////////////////////////////////////////////////////////////////////////
///
/// Button controller module
///
/// Polls the buttons every 25 ms with TaskScheduler, and calls the
/// registered handler function on every button change.
///
/// The 25 ms polling interval takes care of debouncing the switch too.
///
/// Call Buttons_Init() before using this module.
///
//////////////////////////////////////////////////////////////////////////

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include "utils.h"

#define BTN1_RDir	DDRD		// Button 1 data direction register
#define BTN1_RPin	PIND		// Button 1 input pin register
#define BTN1_RPort	PORTD		// Button 1 output port register
#define BTN1_Pin	PD7			// Button 1 pin

#define BTN2_RDir	DDRD		// Button 2 data direction register
#define BTN2_RPin	PIND		// Button 2 input pin register
#define BTN2_RPort	PORTD		// Button 2 output port register
#define BTN2_Pin	PD6			// Button 2 pin

#define BTN3_RDir	DDRD		// Button 3 data direction register
#define BTN3_RPin	PIND		// Button 3 input pin register
#define BTN3_RPort	PORTD		// Button 3 output port register
#define BTN3_Pin	PD5			// Button 3 pin

// How big should the button change queue be
#define BTN_CHANGE_QUEUE_SIZE       10

// How many pollings to wait before sending out repeat btn press events
#define BTN_REPEAT_THRESHOLD        20
// How many pollings between sending out repeat btn press events
#define BTN_REPEAT_INTERVAL          5
// How many pollings to wait before sending out long btn press event
#define BTN_LONG_PRESS_THRESHOLD    60

typedef enum Button {
    LEFT, MIDDLE, RIGHT
} Button;

typedef struct ButtonChange {
    Button btn;
    bool press;
    bool release;
    bool longPress;
    bool repeat;
} ButtonChange;

/**
 * Initializes the button control logic, and registers the specified handler
 * that will be called on every button change.
 */
void Buttons_Init(void (*handler)(ButtonChange change));

#endif /* BUTTONS_H_ */