/*
 * curtain_controller.h
 *
 * Created: 2022. 05. 30. 22:34:24
 *  Author: gabor
 */

#ifndef CURTAIN_CONTROLLER_H_
#define CURTAIN_CONTROLLER_H_

#include "utils.h"
#include "task_scheduler.h"
#include "button_controller.h"
#include "serial_controller.h"
#include "rtc_controller.h"
#include "motor_controller.h"
#include "lcd_controller.h"
#include "schedule_manager.h"

bool direction = false;

void CurtainController_RefreshScreen();
void CurtainController_ButtonChangeHandler(uint8_t id, bool pressed);
void CurtainController_DoCurtainAction(CurtainAction a);
void process_serial_input(char *line);

void CurtainController_Init() {
    Buttons_Init(CurtainController_ButtonChangeHandler);
    Serial_Init(9600, process_serial_input);
    Motor_Init(35);
    RTC_Init();
    LCD_Init();
    ScheduleManager_Init(CurtainController_DoCurtainAction);
}

void CurtainController_DoCurtainAction(CurtainAction a) {
    switch (a) {
        case CLOSE:
            direction = false;
            Motor_SetDir(false);
            Motor_Enable();
            break;
        case OPEN:
            direction = true;
            Motor_SetDir(true);
            Motor_Enable();
            break;
        case STOP:
            Motor_Disable();
            break;
    }
    CurtainController_RefreshScreen();
}

void CurtainController_RefreshScreen() {
    if (Motor_IsEnabled()) {
        if (direction) {
            LCD_PrintStringAt("OPENING...      ", 0, 0);
        } else {
            LCD_PrintStringAt("CLOSING...      ", 0, 0);
        }
        LCD_PrintStringAt("\x7e" "\x7f" "    stop    " "\x7f" "\x7e", 1, 0);
    } else {
        LCD_Clear();
        // call menu refresh screen
    }
}

void CurtainController_ButtonChangeHandler(uint8_t id, bool pressed) {
    if (Motor_IsEnabled()) {
        if (direction) {
            LCD_PrintStringAt("OPENING...      ", 0, 0);
        } else {
            LCD_PrintStringAt("CLOSING...      ", 0, 0);
        }
        LCD_PrintStringAt("\x7e" "\x7f" "    stop    " "\x7f" "\x7e", 1, 0);
    } else {
        CurtainController_DoCurtainAction(OPEN);
        // call menu button handler
    }
}

void process_serial_input(char *line) {
    Serial_PrintString("\nUnknown command.\n\n");
}

#endif /* CURTAIN_CONTROLLER_H_ */