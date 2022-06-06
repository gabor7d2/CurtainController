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

#ifndef BUTTON_CONTROLLER_H_
#define BUTTON_CONTROLLER_H_

#include "utils.h"

#define BTN_RDir	DDRD		// Buttons data direction register
#define BTN_RPin	PIND		// Buttons input pin register
#define BTN_RPort	PORTD		// Buttons output port register

#define BTN1_Pin	PD7			// Button 1 pin
#define BTN2_Pin	PD6			// Button 2 pin
#define BTN3_Pin	PD5			// Button 3 pin

#define BTN_Pin(x) (((x) == 0) ? BTN1_Pin : ((x) == 1) ? BTN2_Pin : BTN3_Pin)

// How big should the button change queue be
#define BTN_CHANGE_QUEUE_SIZE       10

// How many pollings to wait before sending out repeat btn press events
#define BTN_REPEAT_THRESHOLD        20
// How many pollings between sending out repeat btn press events
#define BTN_REPEAT_INTERVAL          4
// How many pollings to wait before sending out long btn press event
#define BTN_LONG_PRESS_THRESHOLD    50

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

/**
 * Ignore any button changes of the specified button until a press occurs.
 */
void Buttons_IgnoreBtnChanges(Button btn);

/**
 * Ignore any button changes until a press occurs on the specific button.
 */
void Buttons_IgnoreAllBtnChanges();

#endif /* BUTTON_CONTROLLER_H_ */