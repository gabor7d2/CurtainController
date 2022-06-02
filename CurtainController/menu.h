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

uint8_t menuid = 0;
char line[20];

void refresh_main_menu(uint8_t id);

void (*curtain_action_handler)(CurtainAction);

void Menu_Init(void (*handler)(CurtainAction)) {
    curtain_action_handler = handler;

    // init task scheduler
    TaskScheduler_Init();
    
    // start task to refresh main menu every 500 ms
    TaskScheduler_Schedule(250, 500, refresh_main_menu);
}

void Menu_ButtonChangeHandler(uint8_t btnId, bool pressed) {
    if (pressed) {
        switch (btnId) {
            case 1:
                curtain_action_handler(CLOSE);
                break;
            case 2:
                break;
            case 3:
                curtain_action_handler(OPEN);
                break;
        }
    }
}

void Menu_RefreshScreen() {
    switch (menuid) {
        case 0:
            LCD_PrintStringAt("                ", 0, 0);
            rtc_time time = RTC_GetTime();
            sprintf(line, "\x7e\x7f  %2d:%02d %s  \x7f\x7e", time.hour, time.min, DayOfWeekToStr(time.wday));
            LCD_PrintStringAt(line, 1, 0);
            break;
    }
}

void refresh_main_menu(uint8_t id) {
    if (!Motor_IsEnabled() && menuid == 0) {
        Menu_RefreshScreen();
    }
}

#endif /* MENU_H_ */