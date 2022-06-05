/*
 * curtain_controller.h
 *
 * Created: 2022. 05. 30. 22:34:24
 *  Author: gabor
 */

#ifndef CURTAIN_CONTROLLER_H_
#define CURTAIN_CONTROLLER_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "utils.h"
#include "task_scheduler.h"
#include "button_controller.h"
#include "serial_controller.h"
#include "rtc_controller.h"
#include "motor_controller.h"
#include "lcd_controller.h"
#include "schedule_manager.h"
#include "menu.h"

#define BAUD_RATE 9600
#define MOTOR_SPEED_RPM 35

// Curtain fully closed detector sensor (reed switch)
#define Sensor_Closed_RDir    DDRC      // data direction register
#define Sensor_Closed_RPin    PINC      // input pin register
#define Sensor_Closed_RPort   PORTC     // output port register
#define Sensor_Closed_Pin     PC0       // pin number

// Curtain fully open detector sensor (reed switch)
#define Sensor_Open_RDir      DDRC      // data direction register
#define Sensor_Open_RPin      PINC      // input pin register
#define Sensor_Open_RPort     PORTC     // output port register
#define Sensor_Open_Pin       PC1       // pin number

// spinning direction of motor
bool direction = false;

// sensor states
bool sensorClosed = false, sensorOpen = false;

void CurtainController_RefreshScreen();
void CurtainController_ButtonChangeHandler(ButtonChange chg);
void CurtainController_DoCurtainAction(CurtainAction a);
void process_serial_input(char *line);
void check_reed_switches(uint8_t id);

void CurtainController_Init() {
    Buttons_Init(CurtainController_ButtonChangeHandler);
    Serial_Init(BAUD_RATE, process_serial_input);
    Motor_Init(MOTOR_SPEED_RPM);
    RTC_Init();
    LCD_Init();
    ScheduleManager_Init(CurtainController_DoCurtainAction);
    Menu_Init(CurtainController_DoCurtainAction);

    // set reed switch pins as input
    clear_bit(Sensor_Closed_RDir, Sensor_Closed_Pin);
    clear_bit(Sensor_Open_RDir, Sensor_Open_Pin);
    
    // enable reed switch pullup resistors
    set_bit(Sensor_Closed_RPort, Sensor_Closed_Pin);
    set_bit(Sensor_Open_RPort, Sensor_Open_Pin);

    // init task scheduler
    TaskScheduler_Init();
    
    // start task to check reed switches
    TaskScheduler_Schedule(250, 0, check_reed_switches);

    CurtainController_RefreshScreen();
}

void CurtainController_DoCurtainAction(CurtainAction a) {
    check_reed_switches(0);
    switch (a) {
        case CLOSE:
            // don't allow motor to start if curtain is already fully closed
            if (sensorClosed) break;
            
            Buttons_IgnoreAllBtnChanges();
            direction = false;
            Motor_SetDir(false);
            Motor_Enable();
            break;
        case OPEN:
            // don't allow motor to start if curtain is already fully open
            if (sensorOpen) break;

            Buttons_IgnoreAllBtnChanges();
            direction = true;
            Motor_SetDir(true);
            Motor_Enable();
            break;
        case STOP:
            Buttons_IgnoreAllBtnChanges();
            Motor_Disable();
            Menu_RefreshScreen();
            break;
    }
    // display the new state
    CurtainController_RefreshScreen();
}

void CurtainController_ButtonChangeHandler(ButtonChange chg) {
    printf("btn %d %s\n", chg.btn, chg.press ? "press" : chg.release ? "release" : chg.repeat ? "repeat" : chg.longPress ? "long press" : "error");
    /*if (chg.press) {
        //StartMeasure();
        StartMeasure2();
    }
    if (chg.release) {
        //printf("%d\n", GetMeasurement());
        //printf("%d\n", GetMeasurement2());
    }*/
    
    if (Motor_IsEnabled()) {
        // motor is running, override button change handling
        if (chg.press) {
            switch (chg.btn) {
                case LEFT:
                    CurtainController_DoCurtainAction(CLOSE);
                    break;
                case MIDDLE:
                    CurtainController_DoCurtainAction(STOP);
                    break;
                case RIGHT:
                    CurtainController_DoCurtainAction(OPEN);
                    break;
            }
        }
    } else {
        // call menu button handler
        Menu_ButtonChangeHandler(chg);
    }
}

void CurtainController_RefreshScreen() {
    if (Motor_IsEnabled()) {
        // motor is running, override screen refresh
        if (direction) {
            LCD_PrintStringAt("OPENING...      ", 0, 0);
        } else {
            LCD_PrintStringAt("CLOSING...      ", 0, 0);
        }
        LCD_PrintStringAt("\x7e\x7f    stop    \x7f\x7e", 1, 0);
    }
}

void check_reed_switches(uint8_t id) {
    sensorClosed = bit_is_clear(Sensor_Closed_RPin, Sensor_Closed_Pin);
    sensorOpen = bit_is_clear(Sensor_Open_RPin, Sensor_Open_Pin);

    if (sensorClosed && Motor_IsEnabled() && !direction) {
        CurtainController_DoCurtainAction(STOP);
    }

    if (sensorOpen && Motor_IsEnabled() && direction) {
        CurtainController_DoCurtainAction(STOP);
    }
}

/**
 * Parses a character array into a uint8_t, and verifies
 * that the parse was successful and it is between the specified bounds.
 * 
 * @return true if parse was successful
 */
bool parse_uint8(char *input, uint8_t *output, uint8_t min, uint8_t max) {
    char *endptr = NULL;
    errno = 0;
    long out = strtol(input, &endptr, 10);

    if (strlen(line) != 0 && (*endptr != '\0' || errno != 0 || out < min || out > max)) {
        printf("\nEntered number is invalid!\nEnter number between %d and %d: ", min, max);
        return false;
    }
    *output = (uint8_t) out;
    return true;
}

/**
 * Parses the input to a character, verifies
 * that the character array contains exactly 1 character,
 * and that it is one of the specified choices,
 * or that the array contains no characters and returns the specified default.
 * 
 * @return the character, or \0 if parse was not successful.
 */
char parse_choice(char *input, char c1, char c2, char def) {
    if (strlen(input) == 0) return def;
    if (strlen(input) > 1) return '\0';

    if (input[0] == c1) return c1;
    else if (input[0] == c2) return c2;
    else return '\0';
}

// what 'menu' the serial com is currently in
uint8_t serialMode = 0;

// the time that is currently being edited
rtc_time tempTime2;

// the schedule that is currently being edited
CurtainSchedule tempSchedule2;

// the index of the schedule that is currently being edited, 255 if it is a new schedule to be added
uint8_t tempScheduleIdx2;

// false if time edit is for setting time, true if it is for editing schedule time
bool timeEditMode = false;

void print_schedules();

void serial_edit_data();

void process_serial_input(char *line) {
    static uint8_t parsedNum = 255;

    if (serialMode == 0) {
        if (strcmp(line, "open") == 0) {
            CurtainController_DoCurtainAction(OPEN);
            printf("\nOpening curtains..\n\n");

        } else if (strcmp(line, "close") == 0) {
            CurtainController_DoCurtainAction(CLOSE);
            printf("\nClosing curtains..\n\n");

        } else if (strcmp(line, "stop") == 0) {
            CurtainController_DoCurtainAction(STOP);
            printf("\nCurtains stopped!\n\n");

        } else if (strcmp(line, "gettime") == 0) {
            printf("\nCurrent time: %d:%02d %s\n\n", RTC_GetTime().hour, RTC_GetTime().min, day_name(RTC_GetTime().wday));

        } else if (strcmp(line, "schedules") == 0) {
            print_schedules();

        } else if (strcmp(line, "nextschedule") == 0) {
            if (ScheduleManager_GetCount() > 0) {
                CurtainSchedule schedule = ScheduleManager_GetNearest();

                // extract day from nearest schedule
                uint8_t day = 0;
                while (day < 7 && !(schedule.daysAndAction & (1 << day)))
                    day++;

                printf("\nNext schedule: %s @ %d:%02d %s\n\n", (schedule.daysAndAction & 0x80) ? "open" : "close", schedule.hour, schedule.min, day_name(day));
            } else {
                printf("\nNo schedules are set.\n\n");
            }

        } else if (strcmp(line, "delschedule") == 0) {
            if (ScheduleManager_GetCount() > 0) {
                printf("\nEnter schedule id to delete (%d-%d): ", 1, ScheduleManager_GetCount());
                serialMode = 10;
            } else {
                printf("\nNo schedules exist.\n\n");
            }

        } else if (strcmp(line, "editschedule") == 0) {
            if (ScheduleManager_GetCount() > 0) {
                printf("\nEnter schedule id to edit (%d-%d): ", 1, ScheduleManager_GetCount());
                serialMode = 20;
            } else {
                printf("\nNo schedules exist.\n\n");
            }

        } else if (strcmp(line, "addschedule") == 0) {
            if (ScheduleManager_GetCount() < POSSIBLE_CURTAIN_SCHEDULES) {
                // get into schedule editing mode
                tempScheduleIdx2 = 255;
                tempSchedule2.daysAndAction = 0;
                tempSchedule2.hour = RTC_GetTime().hour;
                tempSchedule2.min = RTC_GetTime().min;
                timeEditMode = true;
                serialMode = 30;
                printf("\nEnter hour (0-23) [%d]: ", tempSchedule2.hour);
            } else {
                printf("\nSchedule limit reached.\n\n");
            }

        } else if (strcmp(line, "settime") == 0) {
            // get into time editing mode
            tempTime2 = RTC_GetTime();
            tempTime2.sec = 0;
            timeEditMode = false;
            serialMode = 30;
            printf("\nEnter hour (0-23) [%d]: ", tempTime2.hour);

        } else {
            printf("\nUnknown command.\n\n");
        }
    }

    // DEL SCHEDULE
    else if (serialMode == 10) {
        if (parse_uint8(line, &parsedNum, 1, ScheduleManager_GetCount())) {
            // delete schedule and refresh menu
            ScheduleManager_Delete(parsedNum - 1);
			printf("%d\n", parsedNum);
            printf("\nSchedule deleted.\n\n");
            serialMode = 0;
            Menu_RefreshScreen();
        }
    }

    // EDIT SCHEDULE
    else if (serialMode == 20) {
        if (parse_uint8(line, &parsedNum, 1, ScheduleManager_GetCount())) {
            // get into schedule editing mode
            tempScheduleIdx2 = parsedNum - 1;
            tempSchedule2 = ScheduleManager_Get(tempScheduleIdx2);
            timeEditMode = true;
            serialMode = 30;
            printf("\nEnter hour (0-23) [%d]: ", tempSchedule2.hour);
        }
    }

    else {
        serial_edit_data(line);
    }
}

void print_schedules() {
    if (ScheduleManager_GetCount() > 0) {
        printf("\nSchedules:");
        for (uint8_t i = 0; i < ScheduleManager_GetCount(); i++) {
            // print schedule info
            CurtainSchedule schedule = ScheduleManager_Get(i);
            printf("\n#%d %s @ %d:%02d, active on: ", i + 1, (schedule.daysAndAction & 0x80) ? "open" : "close", schedule.hour, schedule.min);

            // print active days
            uint8_t cnt = 0;
            for (uint8_t j = 0; j < 7; j++) {
                if (schedule.daysAndAction & (1 << j)) {
                    // print comma if this is not the first printed day
                    if (cnt != 0) printf(", ");
                    cnt++;
                    printf(day_name_short(j));
                }
            }
        }
        printf("\n\n");
    } else {
        printf("\nNo schedules are set.\n\n");
    }
}

void serial_edit_data(char *line) {
    static uint8_t parsedNum = 255;

    // HOUR
    if (serialMode == 30) {
        if (parse_uint8(line, &parsedNum, 0, 23)) {
            // only change if not default value (empty line) was entered
            if (strlen(line) != 0) {
                if (timeEditMode) tempSchedule2.hour = parsedNum;
                else tempTime2.hour = parsedNum;
            }
            // request min
            printf("\nEnter minutes (0-59) [%d]: ", timeEditMode ? tempSchedule2.min : tempTime2.min);
            serialMode = 31;
        }
    }

    // MIN
    else if (serialMode == 31) {
        if (parse_uint8(line, &parsedNum, 0, 59)) {
            // only change if not default value (empty line) was entered
            if (strlen(line) != 0) {
                if (timeEditMode) tempSchedule2.min = parsedNum;
                else tempTime2.min = parsedNum;
            }
            // request day if this is time edit, otherwise request curtain action
            if (!timeEditMode) {
                printf("\nEnter day (0-6) [%d]: ", tempTime2.wday);
                serialMode = 32;
            } else {
                printf("\nEnter curtain action open/close (o/c) [%c]: ", (tempSchedule2.daysAndAction & 0x80) ? 'o' : 'c');
                serialMode = 33;
            }
        }
    }

    // DAY
    else if (serialMode == 32) {
        if (parse_uint8(line, &parsedNum, 0, 6)) {
            // only change if not default value (empty line) was entered
            if (strlen(line) != 0) {
                tempTime2.wday = parsedNum;
            }
            // save new time
            RTC_SetTime(tempTime2);
            printf("\nSet time to: %d:%02d %s\n\n", RTC_GetTime().hour, RTC_GetTime().min, day_name(RTC_GetTime().wday));
            serialMode = 0;
        }
    }

    // CURTAIN ACTION
    else if (serialMode == 33) {
        char ch = parse_choice(line, 'c', 'o', (tempSchedule2.daysAndAction & 0x80) ? 'o' : 'c');
        if (ch) {
            // if valid choice, set it and request first day
            if (ch == 'o') tempSchedule2.daysAndAction |= 0x80;
            else tempSchedule2.daysAndAction &= ~(0x80);

            printf("\nSet active days:\n");
            printf("\n%s (y/n) [%c]: ", day_name(0), (tempSchedule2.daysAndAction & 0x01) ? 'y' : 'n');
            serialMode = 40;
        } else {
            // otherwise request action again
            printf("\nInvalid choice!\nEnter curtain action open/close (o/c) [%c]: ", (tempSchedule2.daysAndAction & 0x80) ? 'o' : 'c');
        }
    }

    // SCHEDULE DAYS
    else if (serialMode >= 40 && serialMode <= 46) {
        char ch = parse_choice(line, 'y', 'n', (tempSchedule2.daysAndAction & (1 << (serialMode - 40))) ? 'y' : 'n');
        if (ch) {
            // if valid choice, set day
            if (ch == 'y') tempSchedule2.daysAndAction |= (1 << (serialMode - 40));
            else tempSchedule2.daysAndAction &= ~(1 << (serialMode - 40));

            if (serialMode == 46) {
                if (tempSchedule2.daysAndAction & 0x7f) {
                    // save schedule
                    if (tempScheduleIdx2 == 255) {
                        ScheduleManager_Add(tempSchedule2);
                        printf("\nSchedule added!\n\n");
                    } else {
                        ScheduleManager_Edit(tempScheduleIdx2, tempSchedule2);
                        printf("\nSchedule saved!\n\n");
                    }
                    // refresh menu
                    Menu_RefreshScreen();
                } else {
                    // no day set, discard changes
                    printf("\nNo day set, changes discarded!\n\n");
                }
                serialMode = 0;
            } else {
                // request next day
                printf("\n%s (y/n) [%c]: ", day_name(serialMode - 39), (tempSchedule2.daysAndAction & (1 << (serialMode - 39))) ? 'y' : 'n');
                serialMode++;
            }
        } else {
            // otherwise request day again
            printf("\nInvalid choice!\n%s (y/n) [%c]: ", day_name(serialMode - 40), (tempSchedule2.daysAndAction & (1 << (serialMode - 40))) ? 'y' : 'n');
        }
    }
}

#endif /* CURTAIN_CONTROLLER_H_ */