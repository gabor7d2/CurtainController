/*
 * menu.h
 *
 * Created: 2022. 05. 25. 23:32:02
 *  Author: gabor
 */ 

#ifndef MENU_H_
#define MENU_H_

#include "scheduler.h"
#include "buttons.h"
#include "display.h"
#include "rtc3231.h"
#include "utils.h"

void close_curtains();

void open_curtains();

void stop_curtains();

SharedData *data;

char line[20];

uint8_t tempHour, tempMin, tempDay, tempDaysAndType;

//CurtainSchedule schedules[10];
//CurtainSchedule *closest = NULL;

bool Menu_MainRefresh(unsigned char id);

void Menu_Init(SharedData *sharedData) {
	data = sharedData;
	
	data->tempSchedule.hour = 255;
	data->tempSchedule.min = 0;
	data->tempSchedule.daysAndType = 0;
	
	// load schedules
	if (eeprom_read_byte(0x10) != 0x4b) {
		// we never stored data before
		eeprom_write_byte(0x10, 0x4b);
		//eeprom_write_byte(0x11, 0x00);
	} else {
		sharedData->tempSchedule.daysAndType = eeprom_read_byte(0x12);
		sharedData->tempSchedule.hour = eeprom_read_byte(0x13);
		sharedData->tempSchedule.min = eeprom_read_byte(0x14);
	}
	
	/*for(char i = eeprom_read_byte(0x11) - 1; i >= 0; i--) {
		schedules[i].daysAndType = eeprom_read_byte(0x12 + 3 * i);
		schedules[i].hour = eeprom_read_byte(0x12 + 3 * i + 1);
		schedules[i].min = eeprom_read_byte(0x12 + 3 * i + 2);
		
		if (closest == nullptr) closest = schedules[i];
		else {
			
		}
	}*/
	
	Task t = {.id = 1, .freq = 50, .func = Menu_MainRefresh};
	Scheduler_Schedule(t);
}

uint8_t nextDay = 255;

uint8_t calcNextDay() {
	if (nextDay != 255 && data->currTime.sec != 0) return nextDay;
	
	uint8_t currDay = data->currDay;
	/*if ((data->tempSchedule.daysAndType & (1 << currDay)) && data->tempSchedule.hour == data->currTime.hour && data->tempSchedule.min == data->currTime.min) {
		if (data->currTime.sec == 0) {
			return currDay;
		} else {
			for (uint8_t i = 0; i < 7; i++) {
				currDay++;
				if (currDay > 6) currDay = 0;
				if (data->tempSchedule.daysAndType & (1 << currDay)) return currDay;
			}
		}
	}*/
	
	if ((data->tempSchedule.daysAndType & (1 << currDay)) && data->tempSchedule.hour >= data->currTime.hour && data->tempSchedule.min >= data->currTime.min) {
		return currDay;
	}
	
	for (uint8_t i = 0; i < 7; i++) {
		currDay++;
		if (currDay > 6) currDay = 0;
		if (data->tempSchedule.daysAndType & (1 << currDay)) return currDay;
	}
	return 8;
}

bool Menu_MainRefresh(unsigned char id) {
	if (data->running) {
		return true;
	}
	
	if (data->state == 0) {
		if (data->tempSchedule.hour != 255) {
			sprintf(line, "%s @%2d:%02d %s    ", (data->tempSchedule.daysAndType & 0x80) ? "\x7f" "\x7e" : "\x7e" "\x7f", data->tempSchedule.hour, data->tempSchedule.min, DayOfWeekToStr(calcNextDay()));
			LCD_PrintStringAt(line, 0, 0);
			} else {
			LCD_PrintStringAt("                ", 0, 0);
		}
		sprintf(line, "\x7f" "\x7e" "  %2d:%02d %s  " "\x7e" "\x7f", data->currTime.hour, data->currTime.min, DayOfWeekToStr(data->currDay));
		LCD_PrintStringAt(line, 1, 0);
	}
	
	return true;
}

void Menu_Refresh() {
	if (data->running) {
		return;
	}
	
	switch(data->state) {
		case 0:
		if (data->tempSchedule.hour != 255) {
			sprintf(line, "%s @%2d:%02d %s    ", (data->tempSchedule.daysAndType & 0x80) ? "\x7f" "\x7e" : "\x7e" "\x7f", data->tempSchedule.hour, data->tempSchedule.min, DayOfWeekToStr(calcNextDay()));
			LCD_PrintStringAt(line, 0, 0);
			} else {
			LCD_PrintStringAt("                ", 0, 0);
		}
		sprintf(line, "\x7f" "\x7e" "  %2d:%02d %s  " "\x7e" "\x7f", data->currTime.hour, data->currTime.min, DayOfWeekToStr(data->currDay));
		LCD_PrintStringAt(line, 1, 0);
		break;
		
		case 1:
		LCD_PrintStringAt("<   Set Time   >", 0, 0);
		LCD_PrintStringAt("                ", 1, 0);
		break;
		
		case 2:
		LCD_PrintStringAt("<   Schedule   >", 0, 0);
		LCD_PrintStringAt("                ", 1, 0);
		break;
		
		case 3:
		LCD_PrintStringAt("< Del Schedule >", 0, 0);
		LCD_PrintStringAt("                ", 1, 0);
		break;
		
		case 10:
		sprintf(line, "-   %2d:%02d %s   +", tempHour, tempMin, DayOfWeekToStr(tempDay));
		LCD_PrintStringAt("    Set Time    ", 0, 0);
		LCD_PrintStringAt(line, 1, 0);
		LCD_SetCursorPos(1, 5);
		break;
		
		case 11:
		sprintf(line, "-   %2d:%02d %s   +", tempHour, tempMin, DayOfWeekToStr(tempDay));
		LCD_PrintStringAt("    Set Time    ", 0, 0);
		LCD_PrintStringAt(line, 1, 0);
		LCD_SetCursorPos(1, 8);
		break;
		
		case 12:
		sprintf(line, "-   %2d:%02d %s   +", tempHour, tempMin, DayOfWeekToStr(tempDay));
		LCD_PrintStringAt("    Set Time    ", 0, 0);
		LCD_PrintStringAt(line, 1, 0);
		LCD_SetCursorPos(1, 11);
		break;
		
		case 20:
		LCD_PrintStringAt("                ", 0, 0);
		if (tempDaysAndType & 0x01)
		LCD_PrintStringAt("M", 0, 0);
		if (tempDaysAndType & 0x02)
		LCD_PrintStringAt("Tu", 0, 2);
		if (tempDaysAndType & 0x04)
		LCD_PrintStringAt("W", 0, 5);
		if (tempDaysAndType & 0x08)
		LCD_PrintStringAt("Th", 0, 7);
		if (tempDaysAndType & 0x10)
		LCD_PrintStringAt("F", 0, 10);
		if (tempDaysAndType & 0x20)
		LCD_PrintStringAt("Sa", 0, 12);
		if (tempDaysAndType & 0x40)
		LCD_PrintStringAt("S", 0, 15);
		
		sprintf(line, "    %2d:%02d %s    ", tempHour, tempMin, (tempDaysAndType & 0x80) ? "\x7f" "\x7e" : "\x7e" "\x7f");
		LCD_PrintStringAt(line, 1, 0);
		
		if (tempDay == 0) {
			LCD_SetCursorPos(0, 0);
		}
		if (tempDay == 1) {
			LCD_SetCursorPos(0, 2);
		}
		if (tempDay == 2) {
			LCD_SetCursorPos(0, 5);
		}
		if (tempDay == 3) {
			LCD_SetCursorPos(0, 7);
		}
		if (tempDay == 4) {
			LCD_SetCursorPos(0, 10);
		}
		if (tempDay == 5) {
			LCD_SetCursorPos(0, 12);
		}
		if (tempDay == 6) {
			LCD_SetCursorPos(0, 15);
		}
		break;
		
		case 21:
		sprintf(line, "-   %2d:%02d %s   +", tempHour, tempMin, (tempDaysAndType & 0x80) ? "\x7f" "\x7e" : "\x7e" "\x7f");
		LCD_PrintStringAt(line, 1, 0);
		LCD_SetCursorPos(1, 5);
		break;
		
		case 22:
		sprintf(line, "-   %2d:%02d %s   +", tempHour, tempMin, (tempDaysAndType & 0x80) ? "\x7f" "\x7e" : "\x7e" "\x7f");
		LCD_PrintStringAt(line, 1, 0);
		LCD_SetCursorPos(1, 8);
		break;
		
		case 23:
		sprintf(line, "-   %2d:%02d %s   +", tempHour, tempMin, (tempDaysAndType & 0x80) ? "\x7f" "\x7e" : "\x7e" "\x7f");
		LCD_PrintStringAt(line, 1, 0);
		break;
	}
}

void Menu_ProcessButtonChange(BtnChange change) {
	switch(data->state) {
		case 0:
		if (change.id == 1 && change.pressed) open_curtains();
		if (change.id == 2 && change.pressed) data->state = 1;
		if (change.id == 3 && change.pressed) close_curtains();
		break;
		
		case 1:
		if (change.id == 1 && change.pressed) data->state = 0;
		if (change.id == 2 && change.pressed) {
			tempHour = data->currTime.hour;
			tempMin = data->currTime.min;
			tempDay = data->currDay;
			data->state = 10;
		}
		if (change.id == 3 && change.pressed) data->state = 2;
		break;
		
		case 2:
		if (change.id == 1 && change.pressed) data->state = 1;
		if (change.id == 2 && change.pressed) {
			tempDay = 0;
			if (data->tempSchedule.hour == 255) {
				tempHour = data->currTime.hour;
				tempMin = data->currTime.min;
			} else {
				tempHour = data->tempSchedule.hour;
				tempMin = data->tempSchedule.min;
			}
			tempDaysAndType = data->tempSchedule.daysAndType;
			data->state = 20;
		}
		if (change.id == 3 && change.pressed) data->state = 3;
		break;
		
		case 3:
		if (change.id == 1 && change.pressed) data->state = 2;
		if (change.id == 2 && change.pressed) {
			data->tempSchedule.hour = 255;
			data->tempSchedule.min = 0;
			data->tempSchedule.daysAndType = 0;
			eeprom_write_byte(0x12, 0);
			eeprom_write_byte(0x13, 255);
			eeprom_write_byte(0x14, 0);
			data->state = 0;
		}
		break;
		
		case 10:
		if (change.id == 1 && change.pressed) {
			if (tempHour > 0) tempHour--;
			else tempHour = 23;
		}
		
		if (change.id == 2 && change.pressed) data->state = 11;
		
		if (change.id == 3 && change.pressed) {
			if (tempHour < 23) tempHour++;
			else tempHour = 0;
		}
		break;
		
		case 11:
		if (change.id == 1 && change.pressed) {
			if (tempMin > 0) tempMin--;
			else tempMin = 59;
		}
		
		if (change.id == 2 && change.pressed) data->state = 12;
		
		if (change.id == 3 && change.pressed) {
			if (tempMin < 59) tempMin++;
			else tempMin = 0;
		}
		break;
		
		case 12:
		if (change.id == 1 && change.pressed) {
			if (tempDay > 0) tempDay--;
			else tempDay = 6;
		}
		
		if (change.id == 2 && change.pressed) {
			rtc_time t = {0, tempMin, tempHour};
			rtc3231_write_time(&t);
			rtc_date d = {tempDay, 0, 0, 0};
			rtc3231_write_date(&d);
			data->state = 0;
		}
		
		if (change.id == 3 && change.pressed) {
			if (tempDay < 6) tempDay++;
			else tempDay = 0;
		}
		break;
		
		case 20:
		if (change.id == 1 && change.pressed) {
			if (tempDay > 0) tempDay--;
		}
		
		if (change.id == 2 && change.pressed) {
			if (tempDaysAndType & (1 << tempDay)) tempDaysAndType &= ~(1 << tempDay);
			else tempDaysAndType |= (1 << tempDay);
		}
		
		if (change.id == 3 && change.pressed) {
			if (tempDay < 6) tempDay++;
			else data->state = 21;
		}
		break;
		
		case 21:
		if (change.id == 1 && change.pressed) {
			if (tempHour > 0) tempHour--;
			else tempHour = 23;
		}
		
		if (change.id == 2 && change.pressed) data->state = 22;
		
		if (change.id == 3 && change.pressed) {
			if (tempHour < 23) tempHour++;
			else tempHour = 0;
		}
		break;
		
		case 22:
		if (change.id == 1 && change.pressed) {
			if (tempMin > 0) tempMin--;
			else tempMin = 59;
		}
		
		if (change.id == 2 && change.pressed) data->state = 23;
		
		if (change.id == 3 && change.pressed) {
			if (tempMin < 59) tempMin++;
			else tempMin = 0;
		}
		break;
		
		case 23:
		if (change.id == 1 && change.pressed) {
			tempDaysAndType |= (1 << 7);
		}
		
		if (change.id == 2 && change.pressed) {
			data->tempSchedule.hour = tempHour;
			data->tempSchedule.min = tempMin;
			data->tempSchedule.daysAndType = tempDaysAndType;
			eeprom_write_byte(0x12, tempDaysAndType);
			eeprom_write_byte(0x13, tempHour);
			eeprom_write_byte(0x14, tempMin);
			data->state = 0;
		}
		
		if (change.id == 3 && change.pressed) {
			tempDaysAndType &= ~(1 << 7);
		}
		break;
	}
	Menu_Refresh();
}

#endif /* MENU_H_ */