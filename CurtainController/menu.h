/*
 * menu.h
 *
 * Created: 2022. 05. 25. 23:32:02
 *  Author: gabor
 */ 

#ifndef MENU_H_
#define MENU_H_

#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "lcd_controller.h"
#include "rtc_controller.h"

typedef enum Menu {
    MAIN = 0,
    OPTIONS = 1,
    SCHEDULES = 10,
    ADD_SCHEDULE = 11,
    SET_TIME = 12,
    ABOUT = 13,
    EDIT_SCHEDULE = 20,
    MESSAGE = 30
} Menu;

// tracking what menu the user is currently in
Menu menu = MAIN;
// tracking what element the user is currently at in the menu
uint8_t elemidx = 0;

// store previous menu when displaying message to be able to return to it
Menu prevMenu = MAIN;

// the time that is currently being edited
rtc_time tempTime;
// the schedule that is currently being edited
CurtainSchedule tempSchedule;
// the index of the schedule that is currently being edited, 255 if it is a new schedule to be added
uint8_t tempScheduleIdx;

// lcd sprintf buffer or first line of message
char line[20];
// second line of message
char line2[20];

void refresh_main_menu(uint8_t id);
void Menu_RefreshScreen();

void (*curtain_action_handler)(CurtainAction);

void Menu_Init(void (*handler)(CurtainAction)) {
    curtain_action_handler = handler;

    // init task scheduler
    TaskScheduler_Init();
    
    // start task to refresh main menu every 500 ms
    TaskScheduler_Schedule(250, 500, refresh_main_menu);
}

void change_menu(Menu newMenu, uint8_t newElemIdx) {
    prevMenu = menu;
    menu = newMenu;
    elemidx = newElemIdx;
    Buttons_IgnoreAllBtnChanges();
}

void clear_message(uint8_t id) {
    menu = prevMenu;
    TaskScheduler_Deschedule(id);
    Menu_RefreshScreen();
}

void show_message(const char* l1, const char *l2) {
    change_menu(MESSAGE, elemidx);

    if (l1 == NULL) strcpy(line, "                ");
    else strcpy(line, l1);

    if (l2 == NULL) strcpy(line2, "                ");
    else strcpy(line2, l2);

    TaskScheduler_Schedule(50, 2000, clear_message);
}

void edit_value_with_rollover(uint8_t *value, uint8_t max, bool increase) {
    if (increase) {
        if (*value < max) (*value)++;
        else *value = 0;
    } else {
        if (*value > 0) (*value)--;
        else *value = max;
    }
}

bool btn_handler_main_menu(ButtonChange chg) {
    if (!chg.press) return false;

    switch (chg.btn) {
        case LEFT:
            curtain_action_handler(CLOSE);
            break;
        case MIDDLE:
            change_menu(OPTIONS, 0);
            return true;
        case RIGHT:
            curtain_action_handler(OPEN);
            break;
    }

    return false;
}

bool btn_handler_options_menu(ButtonChange chg) {
    if (!chg.press) return false;

    switch (chg.btn) {
        case LEFT:
            if (elemidx == 0) {
                // return to main menu
                menu = MAIN;
            } else elemidx--;
            break;
        case MIDDLE:
            // if current option is 'add schedule'
            if (elemidx == 1) {
                //show error msg if schedule manager is full
                if (ScheduleManager_GetCount() == POSSIBLE_CURTAIN_SCHEDULES) {
                    show_message("Limit reached   ", NULL);
                    break;
                }
                // set temp variables
                tempSchedule.hour = RTC_GetTime().hour;
                tempSchedule.min = RTC_GetTime().min;
                tempSchedule.daysAndAction = 0;
                tempScheduleIdx = 255;
            }
            // if current option is 'set time'
            if (elemidx == 2) {
                // set temp variables
                tempTime = RTC_GetTime();
                tempTime.sec = 0;
            }

            // enter option menu
            change_menu(SCHEDULES + elemidx, 0);
            break;
        case RIGHT:
            if (elemidx != 3) elemidx++;
            break;
    }

    return true;
}

bool btn_handler_schedules_menu(ButtonChange chg) {
    if (chg.longPress && chg.btn == MIDDLE && ScheduleManager_GetCount() != 0) {
        // delete schedule
        ScheduleManager_Delete(elemidx);
        // if the schedule was last on the list, move idx back
        if (elemidx > 0 && elemidx == ScheduleManager_GetCount()) elemidx--;
        // discard middle button changes so that schedule edit doesn't happen
        Buttons_IgnoreAllBtnChanges();
        return true;
    }

    if (chg.press && chg.btn != MIDDLE) {
        if (chg.btn == LEFT) {
            if (elemidx == 0) {
                // return to main menu
                change_menu(OPTIONS, 0);
            } else elemidx--;
            return true;
        }
        else if (chg.btn == RIGHT) {
            if (elemidx < ScheduleManager_GetCount() - 1) {
                elemidx++;
                return true;
            }
        }
    }

    if (chg.release && chg.btn == MIDDLE && ScheduleManager_GetCount() != 0) {
        // edit schedule
        tempSchedule = ScheduleManager_Get(elemidx);
        tempScheduleIdx = elemidx;
        change_menu(EDIT_SCHEDULE, 0);
        return true;
    }

    return false;
}

void schedule_editor_exit() {
    // if any day is set, save schedule
    if (tempSchedule.daysAndAction & 0x7f) {
        if (tempScheduleIdx == 255) {
            ScheduleManager_Add(tempSchedule);
        } else {
            ScheduleManager_Edit(tempScheduleIdx, tempSchedule);
        }
    }

    // return to main/schedules menu
    if (tempScheduleIdx == 255) {
        change_menu(MAIN, 0);
    } else {
        change_menu(SCHEDULES, tempScheduleIdx);
    }
}

bool btn_handler_schedule_editor(ButtonChange chg) {
    // if middle btn long pressed, exit
    if (chg.longPress && chg.btn == MIDDLE) {
        schedule_editor_exit();
        return true;
    }

    if (elemidx < 3) {
        // edit time and action

        // if middle btn released go to the next value to edit
        if (chg.release && chg.btn == MIDDLE) {
            elemidx++;
            return true;
        }

        if ((chg.press || chg.repeat) && chg.btn != MIDDLE && elemidx < 2) {
            // if left or right btn released or repeated
            if (elemidx == 0) edit_value_with_rollover(&tempSchedule.hour, 23, chg.btn == RIGHT);
            if (elemidx == 1) edit_value_with_rollover(&tempSchedule.min, 59, chg.btn == RIGHT);
            return true;
        } else if (chg.press && chg.btn != MIDDLE) {
            // set schedule action to close/open
            if (chg.btn == LEFT) tempSchedule.daysAndAction &= ~0x80;
            else tempSchedule.daysAndAction |= 0x80;
            return true;
        }
    } else {
        // edit active days
        
        // if middle btn released, toggle the day that is currently selected
        if (chg.release && chg.btn == MIDDLE) {
            if (tempSchedule.daysAndAction & (1 << (elemidx - 3))) tempSchedule.daysAndAction &= ~(1 << (elemidx - 3));
            else tempSchedule.daysAndAction |= (1 << (elemidx - 3));
            return true;
        }

        // if left btn released go to the previous day, or to edit the time again
        if (chg.press && chg.btn == LEFT) {
            if (elemidx > 3) elemidx--;
            else elemidx = 0;
            return true;
        }

        // if right btn released, go to the next day, or exit
        if (chg.press && chg.btn == RIGHT) {
            if (elemidx < 9) elemidx++;
            else schedule_editor_exit();
            return true;
        }
    }

    return false;
}

bool btn_handler_set_time_menu(ButtonChange chg) {
    // if middle btn released
    if (chg.press && chg.btn == MIDDLE) {
        // go to the next value to edit
        elemidx++;
        if (elemidx == 3) {
            // set new time
            RTC_SetTime(tempTime);
            // return to main menu
            change_menu(MAIN, 0);
        }
        return true;
    }

    // if left or right btn released or repeated
    if ((chg.press || chg.repeat) && chg.btn != MIDDLE) {
        if (elemidx == 0) edit_value_with_rollover(&tempTime.hour, 23, chg.btn == RIGHT);
        if (elemidx == 1) edit_value_with_rollover(&tempTime.min, 59, chg.btn == RIGHT);
        if (elemidx == 2 && chg.press) edit_value_with_rollover(&tempTime.wday, 6, chg.btn == RIGHT);
        // only need refresh if press occurred or if repeat occurred that is not day edit
        return chg.press || elemidx != 2;
    }

    return false;
}

bool btn_handler_about_menu(ButtonChange chg) {
    // if any button pressed
    if (chg.press) {
        change_menu(OPTIONS, elemidx);
        return true;
    }

    return false;
}

void Menu_ButtonChangeHandler(ButtonChange chg) {
    bool refresh = false;
    switch (menu) {
        case MAIN:
            refresh = btn_handler_main_menu(chg);
            break;
        case OPTIONS:
            refresh = btn_handler_options_menu(chg);
            break;
        case SCHEDULES:
            refresh = btn_handler_schedules_menu(chg);
            break;
        case SET_TIME:
            refresh = btn_handler_set_time_menu(chg);
            break;
        case ABOUT:
            refresh = btn_handler_about_menu(chg);
            break;
        case ADD_SCHEDULE:
        case EDIT_SCHEDULE:
            refresh = btn_handler_schedule_editor(chg);
            break;
    }
    if (refresh) Menu_RefreshScreen();
}

/**
 * Returns a 2 character day name given the day of week number.
 * @param dayOfWeek Number between 0 and 6 (Monday to Sunday)
 * @return The 2 character name of the day, or "XX" if dayOfWeek isn't between 0 and 6.
 */
const char *day_name(uint8_t dayOfWeek) {
    switch (dayOfWeek) {
        case 0:
            return "Mo";
        case 1:
            return "Tu";
        case 2:
            return "We";
        case 3:
            return "Th";
        case 4:
            return "Fr";
        case 5:
            return "Sa";
        case 6:
            return "Su";
    }
    return "XX";
}

void display_main_menu() {
    // display nearest schedule
    if (ScheduleManager_GetCount() > 0) {
        CurtainSchedule schedule = ScheduleManager_GetNearest();

        // extract day from nearest schedule
        uint8_t day = 0;
        while (day < 7 && !(schedule.daysAndAction & (1 << day)))
            day++;

        sprintf(line, "  @ %2d:%02d %s %s ", schedule.hour, schedule.min, day_name(day), (schedule.daysAndAction & 0x80) ? "\x7f\x7e" : "\x7e\x7f");
        LCD_PrintStringAt(line, 0, 0);
    } else LCD_PrintStringAt("No schedules    ", 0, 0);

    // display time
    rtc_time time = RTC_GetTime();
    sprintf(line, "\x7e\x7f  %2d:%02d %s  \x7f\x7e", time.hour, time.min, day_name(time.wday));
    LCD_PrintStringAt(line, 1, 0);
}

void display_options_menu() {
    switch (elemidx) {
        case 0:
            LCD_PrintStringAt("<  Schedules   >", 0, 0);
            break;
        case 1:
            LCD_PrintStringAt("< Add Schedule >", 0, 0);
            break;
        case 2:
            LCD_PrintStringAt("<   Set Time   >", 0, 0);
            break;
        case 3:
            LCD_PrintStringAt("<     About     ", 0, 0);
            break;
    }
    LCD_ClearLine(1);
}

void display_schedule_days(CurtainSchedule *schedule) {
    sprintf(line, "                ");
    if (schedule->daysAndAction & 0x01) line[0] = 'M';
    if (schedule->daysAndAction & 0x02) strncpy(line + 2, "Tu", 2);
    if (schedule->daysAndAction & 0x04) line[5] = 'W';
    if (schedule->daysAndAction & 0x08) strncpy(line + 7, "Th", 2);
    if (schedule->daysAndAction & 0x10) line[10] = 'F';
    if (schedule->daysAndAction & 0x20) strncpy(line + 12, "Sa", 2);
    if (schedule->daysAndAction & 0x40) line[15] = 'S';
    LCD_PrintStringAt(line, 1, 0);
}

void display_schedules_menu() {
    if (ScheduleManager_GetCount() == 0) {
        LCD_PrintStringAt("< No schedules  ", 0, 0);
        LCD_ClearLine(1);
    } else {
        CurtainSchedule schedule = ScheduleManager_Get(elemidx);
        bool lastSchedule = (elemidx == ScheduleManager_GetCount() - 1);
        sprintf(line, "< #%d  %2d:%02d %s %s", elemidx + 1, schedule.hour, schedule.min,(schedule.daysAndAction & 0x80) ? "\x7f\x7e" : "\x7e\x7f", lastSchedule ? " " : ">");
        LCD_PrintStringAt(line, 0, 0);
        display_schedule_days(&schedule);
    }
}

void display_schedule_editor() {
    sprintf(line, "    %2d:%02d %s    ", tempSchedule.hour, tempSchedule.min, (tempSchedule.daysAndAction & 0x80) ? "\x7f\x7e" : "\x7e\x7f");
    LCD_PrintStringAt(line, 0, 0);
    display_schedule_days(&tempSchedule);

    // set cursor pos to the currently edited value
    if (elemidx < 3) LCD_SetCursorPos(0, 5 + elemidx * 3);
    else {
        if (elemidx == 3) LCD_SetCursorPos(1, 0);
        else if (elemidx == 4) LCD_SetCursorPos(1, 3);
        else if (elemidx == 5) LCD_SetCursorPos(1, 5);
        else if (elemidx == 6) LCD_SetCursorPos(1, 8);
        else if (elemidx == 7) LCD_SetCursorPos(1, 10);
        else if (elemidx == 8) LCD_SetCursorPos(1, 13);
        else if (elemidx == 9) LCD_SetCursorPos(1, 15);
    }
}

void display_set_time_menu() {
    LCD_PrintStringAt("    Set Time    ", 0, 0);
    sprintf(line, "-   %2d:%02d %s   +", tempTime.hour, tempTime.min, day_name(tempTime.wday));
    LCD_PrintStringAt(line, 1, 0);
    // set cursor pos to the currently edited value
    LCD_SetCursorPos(1, 5 + elemidx * 3);
}

void display_about_menu() {
    sprintf(line, "Fuggonyvezerlo  ");
    LCD_PrintStringAt(line, 0, 0);
    sprintf(line, "v%d.%d by gabor   ", FIRMWARE_VERSION_MAJOR, FIRMWARE_VERSION_MINOR);
    LCD_PrintStringAt(line, 1, 0);
}

void display_message() {
    LCD_PrintStringAt(line, 0, 0);
    LCD_PrintStringAt(line2, 1, 0);
}

void Menu_RefreshScreen() {
	// don't display anything if motor running
	if (Motor_IsEnabled()) return;
	
    switch (menu) {
        case MAIN:
            display_main_menu();
            break;
        case OPTIONS:
            display_options_menu();
            break;
        case SCHEDULES:
            display_schedules_menu();
            break;
        case SET_TIME:
            display_set_time_menu();
            break;
        case ABOUT:
            display_about_menu();
            break;
        case ADD_SCHEDULE:
        case EDIT_SCHEDULE:
            display_schedule_editor();
            break;
        case MESSAGE:
            display_message();
            break;
    }
}

void refresh_main_menu(uint8_t id) {
    if (!Motor_IsEnabled() && menu == MAIN) {
        display_main_menu();
    }
}

#endif /* MENU_H_ */