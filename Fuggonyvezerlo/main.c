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
#include <string.h>
#include "display.h"
#include "drv8825.h"
#include "serial.h"
#include "buttons.h"
#include "rtc3231.h"
#include "scheduler.h"

void close_curtains();

void open_curtains();

void stop_curtains();

#include "menu.h"

SharedData sharedData = { .state = 0, .running = false, .direction = false };

void button_change_handler(BtnChange change) {
	// Delegate button change to Menu if motor is not running
	if (!sharedData.running) {
		Menu_ProcessButtonChange(change);
		return;
	}
	
	switch (change.id) {
		case 1:
		if (change.pressed) {
			open_curtains();
		}
		break;
		case 2:
		if (change.pressed) {
			stop_curtains();
		}
		break;
		case 3:
		if (change.pressed) {
			close_curtains();
		}
		break;
	}
}

bool request_time_from_rtc() {
	rtc_date date;
	rtc3231_read_datetime(&(sharedData.currTime), &date);
	sharedData.currDay = date.wday;
	
	// keep running
	return true;
}

void close_curtains() {
	data->running = true;
	data->direction = false;
	Motor_SetDir(false);
	Motor_Enable();
	
	LCD_PrintStringAt("CLOSING...      ", 0, 0);
	LCD_PrintStringAt("\x7f" "\x7e" "    stop    " "\x7e" "\x7f", 1, 0);
}

void open_curtains() {
	data->running = true;
	data->direction = true;
	Motor_SetDir(true);
	Motor_Enable();
	
	LCD_PrintStringAt("OPENING...      ", 0, 0);
	LCD_PrintStringAt("\x7f" "\x7e" "    stop    " "\x7e" "\x7f", 1, 0);
}

void stop_curtains() {
	data->running = false;
	Motor_Disable();
}

uint8_t hour, min, day;
char serialMode = 0;

void process_serial_input(char * line) {
    if (serialMode == 0) {
        if (strcmp(line, "open") == 0) {
            open_curtains();
            printf("\r\nOpening curtains..\r\n");
        } else if (strcmp(line, "close") == 0) {
            close_curtains();
            printf("\r\nClosing curtains..\r\n");
        } else if (strcmp(line, "stop") == 0) {
            stop_curtains();
            printf("\r\nCurtains stopped!\r\n");
        } else if (strcmp(line, "gettime") == 0) {
            printf("\r\nCurrent time: %2d:%02d %s\r\n", data -> currTime.hour, data -> currTime.min, DayOfWeekToStr(data -> currDay));
        } else if (strcmp(line, "settime") == 0) {
            printf("\r\nEnter hour (0-23): ");
            serialMode = 1;
        } else if (strcmp(line, "getschedule") == 0) {
            if (data -> tempSchedule.hour != 255) {
                printf("\r\nScheduled: %s @ %2d:%02d %s\r\n", (data -> tempSchedule.daysAndType & 0x80) ? "open" : "close", data -> tempSchedule.hour, data -> tempSchedule.min, DayOfWeekToStr(calcNextDay()));
            } else {
                printf("\r\nNo schedule is set.\r\n");
            }
        } else if (strcmp(line, "delschedule") == 0) {
            data -> tempSchedule.hour = 255;
            data -> tempSchedule.min = 0;
            data -> tempSchedule.daysAndType = 0;
            eeprom_write_byte(0x12, 0);
            eeprom_write_byte(0x13, 255);
            eeprom_write_byte(0x14, 0);
            printf("\r\nSchedule deleted!\r\n");
        } else {
            printf("\r\nUnknown command.\r\n");
        }
    } else if (serialMode == 1) {
        sscanf(line, "%d", & hour);
        if (hour > 23) {
            printf("\r\nEntered number is too big!\r\nEnter hour (0-23): ");
        } else {
            printf("\r\nEnter minutes (0-59): ");
            serialMode = 2;
        }
    } else if (serialMode == 2) {
        sscanf(line, "%d", & min);
        if (min > 59) {
            printf("\r\nEntered number is too big!\r\nEnter minutes (0-59): ");
        } else {
            printf("\r\nEnter day of week (0-6): ");
            serialMode = 3;
        }
    } else if (serialMode == 3) {
        sscanf(line, "%d", & day);
        if (day > 6) {
            printf("\r\nEntered number is too big!\r\nEnter day of week (0-6): ");
        } else {
            rtc_time t = {
                0,
                min,
                hour
            };
            rtc3231_write_time( & t);
            rtc_date d = {
                day,
                0,
                0,
                0
            };
            rtc3231_write_date( & d);
            printf("\r\nTime set to: %2d:%02d %s\r\n", hour, min, DayOfWeekToStr(day));

            serialMode = 0;
        }
    }
}

int main()
{
	// init modules
	Scheduler_Init();
	LCD_Init();
	Buttons_Init(button_change_handler);
	Motor_Init(35);
	Serial_Init(9600);
	rtc3231_init();
	
	Task t = { .id = 0, .freq = 100, .func = request_time_from_rtc};
	Scheduler_Schedule(t);
	
	// set reed switch pins as input
	clear_bit(DDRC, PC0);
	clear_bit(DDRC, PC1);
	// enable reed switch pullup resistors
	set_bit(PORTC, PC0);
	set_bit(PORTC, PC1);
	
	// enable global interrupts
	sei();
	
	Menu_Init(&sharedData);
	
	char char_line[64];
	char lineidx = 0;
	bool hasNewLine = false;
	
	while (1) {
		Buttons_ProcessButtonChanges();
		Scheduler_ProcessTasks();
		
		if (bit_is_clear(PINC, PC0) && sharedData.running && !sharedData.direction) {
			stop_curtains();
		}
		
		if (bit_is_clear(PINC, PC1) && sharedData.running && sharedData.direction) {
			stop_curtains();
		}
		
		if (sharedData.currTime.sec == 0 && sharedData.currTime.hour == sharedData.tempSchedule.hour && sharedData.currTime.min == sharedData.tempSchedule.min && (sharedData.tempSchedule.daysAndType & (1 << sharedData.currDay))) {
			if (sharedData.tempSchedule.daysAndType & (1 << 7)) open_curtains();
			else close_curtains();
		}
		
		while (Serial_HasUnread()) {
			char c = Serial_Read();
			if (c == '\r') continue;
			if (c == '\n') {
				hasNewLine = true;
				char_line[lineidx++] = '\0';
				break;
			}
			char_line[lineidx++] = c;
		}
		
		if (hasNewLine) {
			process_serial_input(char_line);
			hasNewLine = false;
			lineidx = 0;
		}
	}
}

