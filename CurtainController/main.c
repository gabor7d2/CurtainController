/*
 * main.c
 *
 * Created: 3/7/2022 10:24:24 PM
 *  Author: gabor
 */

//#include "utils.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <errno.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "task_scheduler.h"
#include "curtain_controller.h"
//#include <util/delay.h>
//#include <avr/eeprom.h>
//#include <string.h>
//#include "lcd_controller.h"
//#include "motor_controller.h"
//#include "serial_controller.h"
//#include "button_controller.h"
//#include "rtc_controller.h"
//#include "curtain_controller.h"

//CurtainSchedule tempSchedule;
//
//char line[20];
//
//uint8_t tempHour, tempMin, tempDay, tempDaysAndType;
//
//bool Menu_MainRefresh(unsigned char id);
//
//void Menu_Init() {
//    tempSchedule.hour = 255;
//    tempSchedule.min = 0;
//    tempSchedule.daysAndAction = 0;
//
//    // load schedules
//    if (eeprom_read_byte(0x10) != 0x4b) {
//        // we never stored data before
//        eeprom_write_byte(0x10, 0x4b);
//        //eeprom_write_byte(0x11, 0x00);
//    } else {
//        tempSchedule.daysAndAction = eeprom_read_byte(0x12);
//        tempSchedule.hour = eeprom_read_byte(0x13);
//        tempSchedule.min = eeprom_read_byte(0x14);
//    }
//
//    Task t = {.id = 3, .period = 500, .func = Menu_MainRefresh};
//    TaskScheduler_Schedule(t);
//}
//
//uint8_t nextDay = 255;
//
//uint8_t calcNextDay() {
//    if (nextDay != 255 && currTime.sec != 0) return nextDay;
//
//    uint8_t currDay2 = currDay;
//    /*if ((tempSchedule.daysAndAction & (1 << currDay)) && tempSchedule.hour == currTime.hour && tempSchedule.min == currTime.min) {
//        if (currTime.sec == 0) {
//            return currDay;
//        } else {
//            for (uint8_t i = 0; i < 7; i++) {
//                currDay++;
//                if (currDay > 6) currDay = 0;
//                if (tempSchedule.daysAndAction & (1 << currDay)) return currDay;
//            }
//        }
//    }*/
//
//    if ((tempSchedule.daysAndAction & (1 << currDay2)) && tempSchedule.hour >= currTime.hour && tempSchedule.min >= currTime.min) {
//        return currDay2;
//    }
//
//    for (uint8_t i = 0; i < 7; i++) {
//        currDay2++;
//        if (currDay2 > 6) currDay2 = 0;
//        if (tempSchedule.daysAndAction & (1 << currDay2)) return currDay2;
//    }
//    return 8;
//}
//
//bool Menu_MainRefresh(uint8_t id) {
//    if (running) {
//        return true;
//    }
//
//    if (state == 0) {
//        if (tempSchedule.hour != 255) {
//            sprintf(line, "%s @%2d:%02d %s    ", (tempSchedule.daysAndAction & 0x80) ? "\x7f" "\x7e" : "\x7e" "\x7f", tempSchedule.hour, tempSchedule.min, DayOfWeekToStr(calcNextDay()));
//            LCD_PrintStringAt(line, 0, 0);
//        } else {
//            LCD_PrintStringAt("                ", 0, 0);
//        }
//        sprintf(line, "\x7f" "\x7e" "  %2d:%02d %s  " "\x7e" "\x7f", currTime.hour, currTime.min, DayOfWeekToStr(currDay));
//        LCD_PrintStringAt(line, 1, 0);
//    }
//    
//    // keep running
//    return true;
//}
//
//void Menu_Refresh() {
//    if (running) {
//        return;
//    }
//
//    switch (state) {
//        case 0:
//            if (tempSchedule.hour != 255) {
//                sprintf(line, "%s @%2d:%02d %s    ", (tempSchedule.daysAndAction & 0x80) ? "\x7f" "\x7e" : "\x7e" "\x7f", tempSchedule.hour, tempSchedule.min, DayOfWeekToStr(calcNextDay()));
//                LCD_PrintStringAt(line, 0, 0);
//            } else {
//                LCD_PrintStringAt("                ", 0, 0);
//            }
//            sprintf(line, "\x7f" "\x7e" "  %2d:%02d %s  " "\x7e" "\x7f", currTime.hour, currTime.min, DayOfWeekToStr(currDay));
//            LCD_PrintStringAt(line, 1, 0);
//            break;
//
//        case 1:
//            LCD_PrintStringAt("<   Set Time   >", 0, 0);
//            LCD_PrintStringAt("                ", 1, 0);
//            break;
//
//        case 2:
//            LCD_PrintStringAt("<   Schedule   >", 0, 0);
//            LCD_PrintStringAt("                ", 1, 0);
//            break;
//
//        case 3:
//            LCD_PrintStringAt("< Del Schedule  ", 0, 0);
//            LCD_PrintStringAt("                ", 1, 0);
//            break;
//
//        case 10:
//            sprintf(line, "-   %2d:%02d %s   +", tempHour, tempMin, DayOfWeekToStr(tempDay));
//            LCD_PrintStringAt("    Set Time    ", 0, 0);
//            LCD_PrintStringAt(line, 1, 0);
//            LCD_SetCursorPos(1, 5);
//            break;
//
//        case 11:
//            sprintf(line, "-   %2d:%02d %s   +", tempHour, tempMin, DayOfWeekToStr(tempDay));
//            LCD_PrintStringAt("    Set Time    ", 0, 0);
//            LCD_PrintStringAt(line, 1, 0);
//            LCD_SetCursorPos(1, 8);
//            break;
//
//        case 12:
//            sprintf(line, "-   %2d:%02d %s   +", tempHour, tempMin, DayOfWeekToStr(tempDay));
//            LCD_PrintStringAt("    Set Time    ", 0, 0);
//            LCD_PrintStringAt(line, 1, 0);
//            LCD_SetCursorPos(1, 11);
//            break;
//
//        case 20:
//            LCD_PrintStringAt("                ", 0, 0);
//            if (tempDaysAndType & 0x01)
//                LCD_PrintStringAt("M", 0, 0);
//            if (tempDaysAndType & 0x02)
//                LCD_PrintStringAt("Tu", 0, 2);
//            if (tempDaysAndType & 0x04)
//                LCD_PrintStringAt("W", 0, 5);
//            if (tempDaysAndType & 0x08)
//                LCD_PrintStringAt("Th", 0, 7);
//            if (tempDaysAndType & 0x10)
//                LCD_PrintStringAt("F", 0, 10);
//            if (tempDaysAndType & 0x20)
//                LCD_PrintStringAt("Sa", 0, 12);
//            if (tempDaysAndType & 0x40)
//                LCD_PrintStringAt("S", 0, 15);
//
//            sprintf(line, "    %2d:%02d %s    ", tempHour, tempMin, (tempDaysAndType & 0x80) ? "\x7f" "\x7e" : "\x7e" "\x7f");
//            LCD_PrintStringAt(line, 1, 0);
//
//            if (tempDay == 0) {
//                LCD_SetCursorPos(0, 0);
//            }
//            if (tempDay == 1) {
//                LCD_SetCursorPos(0, 2);
//            }
//            if (tempDay == 2) {
//                LCD_SetCursorPos(0, 5);
//            }
//            if (tempDay == 3) {
//                LCD_SetCursorPos(0, 7);
//            }
//            if (tempDay == 4) {
//                LCD_SetCursorPos(0, 10);
//            }
//            if (tempDay == 5) {
//                LCD_SetCursorPos(0, 12);
//            }
//            if (tempDay == 6) {
//                LCD_SetCursorPos(0, 15);
//            }
//            break;
//
//        case 21:
//            sprintf(line, "-   %2d:%02d %s   +", tempHour, tempMin, (tempDaysAndType & 0x80) ? "\x7f" "\x7e" : "\x7e" "\x7f");
//            LCD_PrintStringAt(line, 1, 0);
//            LCD_SetCursorPos(1, 5);
//            break;
//
//        case 22:
//            sprintf(line, "-   %2d:%02d %s   +", tempHour, tempMin, (tempDaysAndType & 0x80) ? "\x7f" "\x7e" : "\x7e" "\x7f");
//            LCD_PrintStringAt(line, 1, 0);
//            LCD_SetCursorPos(1, 8);
//            break;
//
//        case 23:
//            sprintf(line, "-   %2d:%02d %s   +", tempHour, tempMin, (tempDaysAndType & 0x80) ? "\x7f" "\x7e" : "\x7e" "\x7f");
//            LCD_PrintStringAt(line, 1, 0);
//            break;
//    }
//}
//
//void button_change_handler(uint8_t id, bool pressed) {
//    // Delegate button change to Menu if motor is not running
//    if (!running) {
//        switch (state) {
//            case 0:
//                if (id == 1 && pressed) open_curtains();
//                if (id == 2 && pressed) state = 1;
//                if (id == 3 && pressed) close_curtains();
//                break;
//
//            case 1:
//                if (id == 1 && pressed) state = 0;
//                if (id == 2 && pressed) {
//                    tempHour = currTime.hour;
//                    tempMin = currTime.min;
//                    tempDay = currDay;
//                    state = 10;
//                }
//                if (id == 3 && pressed) state = 2;
//                break;
//
//            case 2:
//                if (id == 1 && pressed) state = 1;
//                if (id == 2 && pressed) {
//                    tempDay = 0;
//                    if (tempSchedule.hour == 255) {
//                        tempHour = currTime.hour;
//                        tempMin = currTime.min;
//                    } else {
//                        tempHour = tempSchedule.hour;
//                        tempMin = tempSchedule.min;
//                    }
//                    tempDaysAndType = tempSchedule.daysAndAction;
//                    state = 20;
//                }
//                if (id == 3 && pressed) state = 3;
//                break;
//
//            case 3:
//                if (id == 1 && pressed) state = 2;
//                if (id == 2 && pressed) {
//                    tempSchedule.hour = 255;
//                    tempSchedule.min = 0;
//                    tempSchedule.daysAndAction = 0;
//                    eeprom_write_byte(0x12, 0);
//                    eeprom_write_byte(0x13, 255);
//                    eeprom_write_byte(0x14, 0);
//                    state = 0;
//                }
//                break;
//
//            case 10:
//                if (id == 1 && pressed) {
//                    if (tempHour > 0) tempHour--;
//                    else tempHour = 23;
//                }
//
//                if (id == 2 && pressed) state = 11;
//
//                if (id == 3 && pressed) {
//                    if (tempHour < 23) tempHour++;
//                    else tempHour = 0;
//                }
//                break;
//
//            case 11:
//                if (id == 1 && pressed) {
//                    if (tempMin > 0) tempMin--;
//                    else tempMin = 59;
//                }
//
//                if (id == 2 && pressed) state = 12;
//
//                if (id == 3 && pressed) {
//                    if (tempMin < 59) tempMin++;
//                    else tempMin = 0;
//                }
//                break;
//
//            case 12:
//                if (id == 1 && pressed) {
//                    if (tempDay > 0) tempDay--;
//                    else tempDay = 6;
//                }
//
//                if (id == 2 && pressed) {
//                    rtc_time t = {0, tempMin, tempHour};
//                    rtc3231_SetTime(&t);
//                    rtc_date d = {tempDay, 0, 0, 0};
//                    rtc3231_SetDate(&d);
//                    state = 0;
//                }
//
//                if (id == 3 && pressed) {
//                    if (tempDay < 6) tempDay++;
//                    else tempDay = 0;
//                }
//                break;
//
//            case 20:
//                if (id == 1 && pressed) {
//                    if (tempDay > 0) tempDay--;
//                }
//
//                if (id == 2 && pressed) {
//                    if (tempDaysAndType & (1 << tempDay)) tempDaysAndType &= ~(1 << tempDay);
//                    else tempDaysAndType |= (1 << tempDay);
//                }
//
//                if (id == 3 && pressed) {
//                    if (tempDay < 6) tempDay++;
//                    else state = 21;
//                }
//                break;
//
//            case 21:
//                if (id == 1 && pressed) {
//                    if (tempHour > 0) tempHour--;
//                    else tempHour = 23;
//                }
//
//                if (id == 2 && pressed) state = 22;
//
//                if (id == 3 && pressed) {
//                    if (tempHour < 23) tempHour++;
//                    else tempHour = 0;
//                }
//                break;
//
//            case 22:
//                if (id == 1 && pressed) {
//                    if (tempMin > 0) tempMin--;
//                    else tempMin = 59;
//                }
//
//                if (id == 2 && pressed) state = 23;
//
//                if (id == 3 && pressed) {
//                    if (tempMin < 59) tempMin++;
//                    else tempMin = 0;
//                }
//                break;
//
//            case 23:
//                if (id == 1 && pressed) {
//                    tempDaysAndType |= (1 << 7);
//                }
//
//                if (id == 2 && pressed) {
//                    tempSchedule.hour = tempHour;
//                    tempSchedule.min = tempMin;
//                    tempSchedule.daysAndAction = tempDaysAndType;
//                    eeprom_write_byte(0x12, tempDaysAndType);
//                    eeprom_write_byte(0x13, tempHour);
//                    eeprom_write_byte(0x14, tempMin);
//                    state = 0;
//                }
//
//                if (id == 3 && pressed) {
//                    tempDaysAndType &= ~(1 << 7);
//                }
//                break;
//        }
//        Menu_Refresh();
//        return;
//    }
//
//    switch (id) {
//        case 1:
//            if (pressed) {
//                open_curtains();
//            }
//            break;
//        case 2:
//            if (pressed) {
//                stop_curtains();
//            }
//            break;
//        case 3:
//            if (pressed) {
//                close_curtains();
//            }
//            break;
//    }
//}
//
//bool request_time_from_rtc() {
//    rtc_date date;
//    rtc3231_read_datetime(&(currTime), &date);
//    currDay = date.wday;
//
//    // keep running
//    return true;
//}
//
//void close_curtains() {
//    running = true;
//    direction = false;
//    Motor_SetDir(false);
//    Motor_Enable();
//
//    LCD_PrintStringAt("CLOSING...      ", 0, 0);
//    LCD_PrintStringAt("\x7f" "\x7e" "    stop    " "\x7e" "\x7f", 1, 0);
//}
//
//void open_curtains() {
//    running = true;
//    direction = true;
//    Motor_SetDir(true);
//    Motor_Enable();
//
//    LCD_PrintStringAt("OPENING...      ", 0, 0);
//    LCD_PrintStringAt("\x7f" "\x7e" "    stop    " "\x7e" "\x7f", 1, 0);
//}
//
//void stop_curtains() {
//    running = false;
//    Motor_Disable();
//    Menu_Refresh();
//}

//uint8_t hour, min, day;
//char serialMode = 0;

//void process_serial_input(char * line) {
//    char *endptr = NULL;
//    
//    if (serialMode == 0) {
//        if (strcmp(line, "open") == 0) {
//            open_curtains();
//            const char *msg = "\nOpening curtains..\n";
//            printf(msg);
//        } else if (strcmp(line, "close") == 0) {
//            close_curtains();
//            const char *msg = "\nClosing curtains..\n";
//            printf(msg);
//        } else if (strcmp(line, "stop") == 0) {
//            stop_curtains();
//            const char *msg = "\nCurtains stopped!\n";
//            printf(msg);
//        } else if (strcmp(line, "gettime") == 0) {
//            printf("\nCurrent time: %d:%02d %s\n", currTime.hour, currTime.min, DayOfWeekToStr(currDay));
//        } else if (strcmp(line, "getschedule") == 0) {
//            if (tempSchedule.hour != 255) {
//                printf("\nScheduled: %s @ %2d:%02d %s\n", (tempSchedule.daysAndAction & 0x80) ? "open" : "close", tempSchedule.hour, tempSchedule.min, DayOfWeekToStr(calcNextDay()));
//            } else {
//            const char *msg = "\nNo schedule is set.\n";
//            printf(msg);
//            }
//        } else if (strcmp(line, "delschedule") == 0) {
//            tempSchedule.hour = 255;
//            tempSchedule.min = 0;
//            tempSchedule.daysAndAction = 0;
//            eeprom_write_byte(0x12, 0);
//            eeprom_write_byte(0x13, 255);
//            eeprom_write_byte(0x14, 0);
//            const char *msg = "\nSchedule deleted!\n";
//            printf(msg);
//        } else if (strcmp(line, "settime") == 0) {
//            printf("\nEnter hour (0-23): ");
//            serialMode = 1;
//        } else {
//            const char *msg = "\nUnknown command.\n";
//            printf(msg);
//        }
//    } else if (serialMode == 1) {
//        errno = 0;
//        hour = strtol(line, &endptr, 10);
//        
//        if (strlen(line) != 0 && (*endptr != NULL || errno != 0 || hour > 23)) {
//            printf("\nEntered number is invalid!\nEnter hour (0-23): \n");
//        } else {
//            printf("\nEnter minutes (0-59): \n");
//            serialMode = 2;
//        }
//    } else if (serialMode == 2) {
//        errno = 0;
//        min = strtol(line, &endptr, 10);
//        
//        if (strlen(line) != 0 && (*endptr != NULL || errno != 0 || min > 59)) {
//            printf("\nEntered number is invalid!\nEnter minutes (0-59): ");
//        } else {
//            printf("\nEnter day of week (0-6): ");
//            serialMode = 3;
//        }
//    } else if (serialMode == 3) {
//        errno = 0;
//        day = strtol(line, &endptr, 10);
//        
//        if (strlen(line) != 0 && (*endptr != NULL || errno != 0 || day > 6)) {
//            printf("\nEntered number is invalid!\nEnter day of week (0-6): ");
//        } else {
//            rtc_time t = {
//                0,
//                min,
//                hour
//            };
//            rtc3231_SetTime(& t);
//            rtc_date d = {
//                day,
//                0,
//                0,
//                0
//            };
//            rtc3231_SetDate(& d);
//            printf("\nTime set to: %2d:%02d %s\n", hour, min, DayOfWeekToStr(day));
//
//            serialMode = 0;
//        }
//    }
//}

void repeat(uint8_t id) {
    LCD_PrintChar('a');
}

int main() {
    // main init
    TaskScheduler_Init();
    CurtainController_Init();

    //TaskScheduler_Schedule(100, 1000, repeat);

    // enable global interrupts
    sei();

    while (1) {
        TaskScheduler_ProcessTasks();
    }
}

