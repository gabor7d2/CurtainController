/*
 * curtain_controller.h
 *
 * Created: 2022. 05. 30. 22:34:24
 *  Author: gabor
 */

#ifndef CURTAIN_CONTROLLER_H_
#define CURTAIN_CONTROLLER_H_

#include <stdio.h>
#include "utils.h"
#include "task_scheduler.h"
#include "button_controller.h"
#include "serial_controller.h"
#include "rtc_controller.h"
#include "motor_controller.h"
#include "lcd_controller.h"
#include "schedule_manager.h"
#include "menu.h"

#define Sensor_Closed_RDir    DDRC
#define Sensor_Closed_RPin    PINC
#define Sensor_Closed_RPort   PORTC
#define Sensor_Closed_Pin     PC0

#define Sensor_Open_RDir      DDRC
#define Sensor_Open_RPin      PINC
#define Sensor_Open_RPort     PORTC
#define Sensor_Open_Pin       PC1

bool direction = false;
bool sensorClosed = false, sensorOpen = false;

void CurtainController_RefreshScreen();
void CurtainController_ButtonChangeHandler(uint8_t btnId, bool pressed);
void CurtainController_DoCurtainAction(CurtainAction a);
void process_serial_input(char *line);
void check_reed_switches(uint8_t id);

void CurtainController_Init() {
    Buttons_Init(CurtainController_ButtonChangeHandler);
    Serial_Init(9600, process_serial_input);
    Motor_Init(35);
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
            if (sensorClosed) break;
            direction = false;
            Motor_SetDir(false);
            Motor_Enable();
            break;
        case OPEN:
            if (sensorOpen) break;
            direction = true;
            Motor_SetDir(true);
            Motor_Enable();
            break;
        case STOP:
            Motor_Disable();
            break;
    }
    // display the new state
    CurtainController_RefreshScreen();
}

void CurtainController_ButtonChangeHandler(uint8_t btnId, bool pressed) {
    if (pressed) {
        StartMeasure();
    } else {
        printf("%d\n", GetMeasurement());
    }
    
    if (Motor_IsEnabled()) {
        // motor is running, override button change handling
        if (pressed) {
            switch (btnId) {
                case 1:
                    CurtainController_DoCurtainAction(CLOSE);
                    break;
                case 2:
                    CurtainController_DoCurtainAction(STOP);
                    break;
                case 3:
                    CurtainController_DoCurtainAction(OPEN);
                    break;
            }
        }
    } else {
        // call menu button handler
        Menu_ButtonChangeHandler(btnId, pressed);
        CurtainController_RefreshScreen();
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
    } else {
        // call menu refresh screen
        Menu_RefreshScreen();
    }
}

void process_serial_input(char *line) {
    printf("\nUnknown command.\n\n");
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

#endif /* CURTAIN_CONTROLLER_H_ */