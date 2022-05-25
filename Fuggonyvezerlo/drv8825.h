/*
* drv8825.h
*
* Created: 2022. 05. 03. 9:31:52
*  Author: gabor
*/

//////////////////////////////////////////////////////////////////////////
// 
/// Controller for stepper motor driver board DRV8825
/// https://www.ti.com/lit/ds/symlink/drv8825.pdf
/// https://caxtool.hu/custom/mysticnails/image/data/Webshop/le%C3%ADr%C3%A1s/0J4232.600.png
///
/// Uses the specified port bits of the specified port for the board's
/// Enable, Step and Direction signals.
///
/// Automatically steps the motor with the set speed, using TIMER4.
/// Using a timer and interrupts for stepping makes sure that the motor's
/// speed is always constant and accurate.
///
/// Call Motor_Init() before using this module.
///
//////////////////////////////////////////////////////////////////////////

#ifndef DRV8825_H
#define DRV8825_H

#include <stdbool.h>
#include <avr/io.h>
#include "utils.h"

#define Motor_Dir	DDRD		// DRV8825 data direction register
#define Motor_Port	PORTD		// DRV8825 data output register
//#define Motor_Pin	PIND		// DRV8825 data input register (used for toggling)
#define Motor_EN	PD4			// DRV8825 enable pin
#define Motor_STEP  PD3			// DRV8825 step pin
#define Motor_DIR	PD2			// DRV8825 dir pin

uint16_t timerSpeed = 37500;

void Motor_SetSpeed(double rpm);
void Motor_Step();

/**
* Initialize motor pins, set motor speed and disable motor.
*/
void Motor_Init(double rpm) {
	// Set direction as output
	set_bit(Motor_Dir, Motor_EN);
	set_bit(Motor_Dir, Motor_STEP);
	set_bit(Motor_Dir, Motor_DIR);
	
	// Disable motor
	set_bit(Motor_Port, Motor_EN);
	
	// Setup TIMER 4: 16-bit, /64 prescaler, timer will be dynamically adjustable.
	TCCR4A = 0;				// Disconnect OC4A/OC4B pins, set WGM4[1:0] to 0
	TCCR4B = 0b00001011;	// set WGM4[3:2] to 01 (count upto the value in OCR4A), set prescaler to 64
	TCNT4 = 0;				// clear timer
	TIMSK4 = 0b00000010;	// enable interrupt when TCNT4 == OCR4A
	// https://eleccelerator.com/avr-timer-calculator/
	
	// Set motor speed
	Motor_SetSpeed(rpm);
}

// Only do stepping every 2 timer ticks
bool step = true;

/**
* Interrupt vector to automatically step the motor.
*/
ISR(TIMER4_COMPA_vect) {
	if (step) {
		Motor_Step();
	}
	step = !step;
}

/**
* Enable motor.
*/
void Motor_Enable() {
	clear_bit(Motor_Port, Motor_EN);
}

/**
* Disable motor.
*/
void Motor_Disable() {
	set_bit(Motor_Port, Motor_EN);
}

/**
* Get if motor is enabled.
*/
bool Motor_IsEnabled() {
	return bit_is_clear(Motor_Port, Motor_EN);
}

/**
* Set motor spinning direction.
*/
void Motor_SetDir(bool dir) {
	if (dir)
		set_bit(Motor_Port, Motor_DIR);
	else
		clear_bit(Motor_Port, Motor_DIR);
}

/**
* Reverse motor spinning direction.
*/
void Motor_ReverseDir() {
	//Motor_Pin |= Motor_DIR;
	if (bit_is_set(Motor_Port, Motor_DIR)) Motor_SetDir(false);
	else Motor_SetDir(true);
}

/**
* Set motor speed using rpm value.
*/
void Motor_SetSpeed(double rpm) {
	if (rpm <= 0) rpm = 1;
	if (rpm > 1000) rpm = 1000;
	timerSpeed = (uint16_t) (37500 / rpm);
	OCR4A = timerSpeed;
}

/**
* Do one step of the motor, if it is enabled.
*/
void Motor_Step() {
	if (!Motor_IsEnabled()) return;
	
	set_bit(Motor_Port, Motor_STEP);
	_delay_us(100);
	clear_bit(Motor_Port, Motor_STEP);
	_delay_us(100);
}

#endif