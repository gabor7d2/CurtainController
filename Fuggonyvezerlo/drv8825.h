/*
* drv8825.h
*
* Created: 2022. 05. 03. 9:31:52
*  Author: gabor
*/
#ifndef DRV8825_H
#define DRV8825_H

#include <stdbool.h>
#include <avr/io.h>

#define Motor_Dir	DDRD		// DRV8825 data direction register
#define Motor_Port	PORTD		// DRV8825 data output register
//#define Motor_Pin	PIND		// DRV8825 data input register (used for toggling)
#define Motor_EN	0x10		// DRV8825 enable pin
#define Motor_STEP  0x08		// DRV8825 step pin
#define Motor_DIR	0x04		// DRV8825 dir pin

int delayUs = 10000;

void Motor_Init() {
	Motor_Dir |= (Motor_EN | Motor_STEP | Motor_DIR);
	Motor_Port |= Motor_EN;
}

void Motor_Enable() {
	Motor_Port &= ~Motor_EN;
}

void Motor_Disable() {
	Motor_Port |= Motor_EN;
}

bool Motor_IsEnabled() {
	return !(Motor_Port & Motor_EN);
}

void Motor_SetDir(bool dir) {
	if (dir)
		Motor_Port |= Motor_DIR;
	else
		Motor_Port &=  ~Motor_DIR;
}

void Motor_ReverseDir() {
	//Motor_Pin |= Motor_DIR;
	if (Motor_Port & Motor_DIR) Motor_SetDir(false);
	else Motor_SetDir(true);
}

void Motor_SetSpeed(double speed) {
	delayUs = 10000 / speed;
}

void Motor_Step() {
	if (!Motor_IsEnabled()) return;
	
	Motor_Port |= Motor_STEP;
	_delay_us(delayUs);
	Motor_Port &= ~Motor_STEP;
	_delay_us(delayUs);
}

#endif