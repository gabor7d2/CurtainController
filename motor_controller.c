
#define F_CPU 16000000
#define __DELAY_BACKWARD_COMPATIBLE__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "motor_controller.h"

uint16_t timerSpeed = 37500;

void Motor_Step();

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

void Motor_Enable() {
	clear_bit(Motor_Port, Motor_EN);
}

void Motor_Disable() {
	set_bit(Motor_Port, Motor_EN);
}

bool Motor_IsEnabled() {
	return bit_is_clear(Motor_Port, Motor_EN);
}

void Motor_SetDir(bool dir) {
	if (dir)
		set_bit(Motor_Port, Motor_DIR);
	else
		clear_bit(Motor_Port, Motor_DIR);
}

void Motor_ReverseDir() {
	//Motor_Pin |= Motor_DIR;
	if (bit_is_set(Motor_Port, Motor_DIR)) Motor_SetDir(false);
	else Motor_SetDir(true);
}

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
