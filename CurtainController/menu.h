/*
 * menu.h
 *
 * Created: 2022. 05. 25. 23:32:02
 *  Author: gabor
 */ 

#ifndef MENU_H_
#define MENU_H_

#include <stdio.h>
#include "utils.h"
#include "lcd_controller.h"
#include "rtc_controller.h"

typedef enum Menu {
    MAIN = 0,
    OPTIONS = 1,
    SCHEDULES = 10,
    ADD_SCHEDULE = 11,
    SET_TIME = 12
} Menu;

// tracking what menu the user is currently in
uint8_t menu = 0;
// tracking what element the user is currently at in the menu
uint8_t elemidx = 0;

rtc_time tempTime;

// lcd sprintf buffer
char line[20];

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

void time_edit_min(bool increase) {
    if (increase) {
        if (tempTime.min < 59) tempTime.min++;
        else tempTime.min = 0;
    } else {
        if (tempTime.min > 0) tempTime.min--;
        else tempTime.min = 59;
    }
}

void time_edit_hour(bool increase) {
    if (increase) {
        if (tempTime.hour < 23) tempTime.hour++;
        else tempTime.hour = 0;
    } else {
        if (tempTime.hour > 0) tempTime.hour--;
        else tempTime.hour = 23;
    }
}

void time_edit_day(bool increase) {
    if (increase) {
        if (tempTime.wday < 6) tempTime.wday++;
        else tempTime.wday = 0;
    } else {
        if (tempTime.wday > 0) tempTime.wday--;
        else tempTime.wday = 6;
    }
}

void display_set_time_menu();

void rapid_time_edit(uint8_t id) {
    // if the current menu isn't set time menu, deactivate this task
    if (menu != 10) {
        TaskScheduler_Deschedule(id);
        return;
    }

    if (id - 10 < 4) {
        TaskScheduler_Deschedule(id);
        TaskScheduler_Schedule(id + 4, 125, rapid_time_edit);
    } else {
        if (id - 14 < 2) time_edit_hour(id - 14 == 1);
        else time_edit_min(id - 14 == 3);
        display_set_time_menu();
    }
}

void btn_chg_set_time_menu(ButtonChange chg) {
    // if button released
    if (!chg.press) {
        // deactivate potentially running rapid edits
        for (uint8_t i = 0; i < 8; i++)
            TaskScheduler_Deschedule(10 + i);

        return;
    }

    // if middle btn pressed
    if (chg.btn == MIDDLE) {
        // go to the next value to edit
        elemidx++;
        if (elemidx == 3) {
            // set new time
            RTC_SetTime(tempTime);
            // return to main menu
            menu = 0;
        }
        return;
    }

    // if left or right btn pressed
    switch (elemidx) {
        case 0:
            time_edit_hour(chg.btn == RIGHT);
            TaskScheduler_Schedule(chg.btn == RIGHT ? 11 : 10, 500, rapid_time_edit);
            break;
        case 1:
            time_edit_min(chg.btn == RIGHT);
            TaskScheduler_Schedule(chg.btn == RIGHT ? 13 : 12, 500, rapid_time_edit);
            break;
        case 2:
            time_edit_day(chg.btn == RIGHT);
            break;
    }
}

void btn_chg_schedules_menu(ButtonChange chg) {
    
}

void btn_chg_add_schedule_menu(ButtonChange chg) {
    
}

void btn_chg_options_menu(ButtonChange chg) {
    if (menu == 1 && chg.press) {
        switch (chg.btn) {
            case LEFT:
                if (elemidx == 0) {
                    // return to main menu
                    menu = 0;
                } else elemidx--;
                break;
            case MIDDLE:
                // enter option menu
                menu = 10 + elemidx;
                elemidx = 0;
                tempTime = RTC_GetTime();
                tempTime.sec = 0;
                break;
            case RIGHT:
                if (elemidx != 2) elemidx++;
                break;
        }
    } else {
        switch (menu) {
            case 10:
                btn_chg_set_time_menu(chg);
                break;
            case 11:
                btn_chg_schedules_menu(chg);
                break;
            case 12:
                btn_chg_add_schedule_menu(chg);
                break;
        }
    }
}

void btn_chg_main_menu(ButtonChange chg) {
    if (!chg.press) return;

    switch (chg.btn) {
        case LEFT:
            curtain_action_handler(CLOSE);
            break;
        case MIDDLE:
            menu = 1;
            elemidx = 0;
            break;
        case RIGHT:
            curtain_action_handler(OPEN);
            break;
    }
}

void Menu_ButtonChangeHandler(ButtonChange chg) {
    //if  (!chg.press) return;

    if (menu == 0) {
        btn_chg_main_menu(chg);
    } else {
        btn_chg_options_menu(chg);
    }
    Menu_RefreshScreen();
}

void display_set_time_menu() {
    LCD_PrintStringAt("    Set Time    ", 0, 0);
    sprintf(line, "-   %2d:%02d %s   +", tempTime.hour, tempTime.min, DayOfWeekToStr(tempTime.wday));
    LCD_PrintStringAt(line, 1, 0);
    LCD_SetCursorPos(1, 5 + elemidx * 3);
}

void display_schedules_menu() {

}

void display_add_schedule_menu() {

}

void display_options_menu() {
    if (menu == 1) {
        switch (elemidx) {
            case 0:
                LCD_PrintStringAt("<   Set Time   >", 0, 0);
                break;
            case 1:
                LCD_PrintStringAt("<   Schedules  >", 0, 0);
                break;
            case 2:
                LCD_PrintStringAt("< Add Schedule  ", 0, 0);
                break;
        }
        LCD_ClearLine(1);
    } else {
        switch (menu) {
            case 10:
                display_set_time_menu();
                break;
            case 11:
                display_schedules_menu();
                break;
            case 12:
                display_add_schedule_menu();
                break;
        }
    }
}

void display_main_menu() {
        LCD_PrintStringAt("No schedules    ", 0, 0);
        rtc_time time = RTC_GetTime();
        sprintf(line, "\x7e\x7f  %2d:%02d %s  \x7f\x7e", time.hour, time.min, DayOfWeekToStr(time.wday));
        LCD_PrintStringAt(line, 1, 0);
}

void Menu_RefreshScreen() {
    if (menu == 0) {
        // show main menu
        display_main_menu();
    } else {
        // show options submenu
        display_options_menu();
    }
}

void refresh_main_menu(uint8_t id) {
    if (!Motor_IsEnabled() && menu == 0) {
        display_main_menu();
    }
}

#endif /* MENU_H_ */