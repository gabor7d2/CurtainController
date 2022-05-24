/*
* main.c
*
* Created: 3/7/2022 10:24:24 PM
*  Author: gabor
*/

#define F_CPU 16000000
#define __DELAY_BACKWARD_COMPATIBLE__

#include <stdbool.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "display.h"
#include "drv8825.h"
#include "serial.h"

bool interrupt = false;
char prevPIND = 0xff;

int sec = 0;

ISR(PCINT2_vect) {
	interrupt = true;
}

// Runs every 1s
ISR(TIMER1_COMPA_vect) {
	sec++;
	/*char s[10];
	sprintf(s,"%d    ", sec);
	LCD_String_xy(s, 0, 0);
	Serial_PrintString(s);*/
}

// Runs every 5ms
ISR(TIMER3_COMPA_vect) {
	
}

void btnChange() {
	if (!(PIND & 0x20) && (prevPIND & 0x20)) {
		// D5 button pressed
		//LCD_String_xy("SAVE    ", 1, 0);
		//eeprom_write_word(0, sec);
		// enable drv8825
		//Motor_Enable();
		printf("SAVE");
	}
	
	if (!(PIND & 0x40) && (prevPIND & 0x40)) {
		// D6 button pressed
		//LCD_String_xy("RESTORE", 1, 0);
		//sec = eeprom_read_word(0);
		// disable drv8825
		//Motor_Disable();
		printf("RESTORE");
	}
	
	if (!(PIND & 0x80) && (prevPIND & 0x80)) {
		// D7 button pressed
		//LCD_String_xy("RESET   ", 1, 0);
		//sec = 0;
		//Motor_ReverseDir();
		printf("RESET");
	}
	
	prevPIND = PIND;
}

void start_timers() {
	// https://eleccelerator.com/avr-timer-calculator/
	
	// ######## TIMER 1: 16-bit, count upto 15625 with /1024 prescaler = reset every 1s
	
	TCCR1A = 0;				// Disconnect OC1A/OC1B pins, set WGM1[1:0] to 0
	TCCR1B = 0b00001101;	// set WGM1[3:2] to 01 (count upto the value in OCR1A), set prescaler to 1024
	TCNT1 = 0;				// clear timer
	OCR1A = 15625;			// set top value to count upto
	TIMSK1 = 0b00000010;	// enable interrupt when TCNT1 == OCR1A
	
	// ######## TIMER 3: 16-bit, count upto 1250 with /64 prescaler = reset every 5ms
	
	TCCR3A = 0;				// Disconnect OC3A/OC3B pins, set WGM3[1:0] to 0
	TCCR3B = 0b00001011;	// set WGM3[3:2] to 01 (count upto the value in OCR3A), set prescaler to 64
	TCNT3 = 0;				// clear timer
	OCR3A = 1250;			// set top value to count upto
	TIMSK3 = 0b00000010;	// enable interrupt when TCNT3 == OCR3A
}

int main()
{
	Serial_Init(9600);
	LCD_Init();			/* Initialization of LCD*/
	Motor_Init();
	Motor_SetSpeed(1.5);
	
	start_timers();
	
	// Set bits D5-D7 as input
	DDRD = DDRD & 0b00000111;
	// need to write 1 to activate internal pull-ups
	PORTD = PORTD | 0xf0;
	
	// wait for pins to stabilize
	_delay_ms(100);
	// clear any pin change interrupt flag that might have occurred
	PCIFR = 0x00;
	
	// enable interrupt for pins D5-D7
	PCICR = 0x04;
	PCMSK2 = 0xe0;
	sei();
	
	while (1) {
		if (interrupt) {
			btnChange();
			_delay_ms(5);
			interrupt = false;
		}
		int x = 0, y = 0;
		char c[2];
		c[1] = 0;
		while (Serial_Has_Unread()) {
			if (x > 15) {
				x = 0;
				y++;
			}
			c[0] = Serial_Read();
			if (c[0] == '\n' || c[0] == '\r') continue;
			LCD_String_xy(c, y, x);
			x++;
		}
		Motor_Step();
	}
}

