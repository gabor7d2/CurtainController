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
#include "buttons.h"
#include "rtc3231.h"

typedef struct rtc_time rtc_time;
typedef struct rtc_date rtc_date;

int sec = 0;

// Runs every 1s
ISR(TIMER1_COMPA_vect) {
	sec++;
	//printf("%d\n", sec);
	/*char s[10];
	sprintf(s,"%d    ", sec);
	LCD_String_xy(s, 0, 0);
	Serial_PrintString(s);*/
}

/*void btnChange() {
if (!(PIND & 0x20) && (prevPIND & 0x20)) {
// D5 button pressed
//LCD_String_xy("SAVE    ", 1, 0);
//eeprom_write_word(0, sec);
// enable drv8825
//Motor_Enable();
printf("SAVE\n");
}

if (!(PIND & 0x40) && (prevPIND & 0x40)) {
// D6 button pressed
//LCD_String_xy("RESTORE", 1, 0);
//sec = eeprom_read_word(0);
// disable drv8825
//Motor_Disable();
printf("RESTORE\n");
}

if (!(PIND & 0x80) && (prevPIND & 0x80)) {
// D7 button pressed
//LCD_String_xy("RESET   ", 1, 0);
//sec = 0;
//Motor_ReverseDir();
printf("RESET\n");
}

prevPIND = PIND;
}*/

void start_timers() {
	// Setup TIMER 1: 16-bit, count upto 15625 with /1024 prescaler = reset every 1s
	TCCR1A = 0;				// Disconnect OC1A/OC1B pins, set WGM1[1:0] to 0
	TCCR1B = 0b00001101;	// set WGM1[3:2] to 01 (count upto the value in OCR1A), set prescaler to 1024
	TCNT1 = 0;				// clear timer
	OCR1A = 15625;			// set top value to count upto
	TIMSK1 = 0b00000010;	// enable interrupt when TCNT1 == OCR1A
	// https://eleccelerator.com/avr-timer-calculator/
}

uint16_t rpm = 1;

void button1_changed(bool pressed) {
	if (pressed) {
		rpm--;
		if (rpm < 1) rpm = 1;
		Motor_SetSpeed(rpm);
		
		char ch[5];
		sprintf(ch, "%d", rpm);
		LCD_Clear();
		LCD_PrintString(ch);
		
		printf("BTN1 pressed\n");
		} else {
		printf("BTN1 released\n");
	}
}

void button2_changed(bool pressed) {
	if (pressed) {
		rtc_time t;
		rtc_date d;
		rtc3231_read_datetime(&t, &d);
		char ch[17];
		
		sprintf(ch, "20%02d. %02d. %02d. %d", d.year, d.month, d.day, d.wday);
		LCD_PrintStringAt(ch, 0, 0);
		
		sprintf(ch, "%02d:%02d:%02d", t.hour, t.min, t.sec);
		LCD_PrintStringAt(ch, 1, 0);
		
		//Motor_ReverseDir();
		if (Motor_IsEnabled()) {
			Motor_Disable();
		} else Motor_Enable();
		
		/*rtc_date d;
		d.year = 22;
		d.month = 5;
		d.day = 25;
		rtc_time t;
		t.hour = 2;
		t.min = 33;
		t.sec = 0;
		
		rtc3231_write_date(&d);
		rtc3231_write_time(&t);*/
		
		printf("BTN2 pressed\n");
		} else {
		printf("BTN2 released\n");
	}
}

void button3_changed(bool pressed) {
	if (pressed) {
		rpm++;
		if (rpm > 1000) rpm = 1000;
		Motor_SetSpeed(rpm);
		
		char ch[5];
		sprintf(ch, "%d", rpm);
		LCD_Clear();
		LCD_PrintString(ch);
		
		printf("BTN3 pressed\n");
		} else {
		printf("BTN3 released\n");
	}
}

int main()
{
	Serial_Init(9600);
	LCD_Init();
	Buttons_Init();
	Motor_Init(35);
	
	clear_bit(DDRC, PC0);
	clear_bit(DDRC, PC1);
	set_bit(PORTC, PC0);
	set_bit(PORTC, PC1);
	
	//Motor_Enable();
	
	rtc3231_init();
	
	start_timers();
	
	// enable global interrupts
	sei();
	
	/*uint16_t t1 = TCNT1;
	LCD_PrintChar('a');
	uint16_t t2 = TCNT1;
	printf("LCD print: t1 = %d, t2 = %d\n", t1, t2);*/
	
	while (1) {
		int x = 0, y = 0;
		char c;
		while (Serial_HasUnread()) {
			if (x > 15) {
				x = 0;
				y++;
			}
			c = Serial_Read();
			if (c == '\n' || c == '\r') continue;
			LCD_PrintCharAt(c, y, x);
			x++;
		}
		
		if (bit_is_clear(PINC, PC0)) {
			LCD_PrintChar('A');
		}
		
		if (bit_is_clear(PINC, PC1)) {
			LCD_PrintChar('B');
		}
		
		while (Buttons_HasUnprocessedChange()) {
			//t1 = TCNT1;
			Buttons_ProcessChange(button1_changed, button2_changed, button3_changed);
			//t2 = TCNT1;
			//printf("Btn change processed: t1 = %d, t2 = %d\n", t1, t2);
			//LCD_PrintChar('a');
		}
	}
}

