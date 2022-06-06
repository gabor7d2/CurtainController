/*
 * menu.h
 *
 * Created: 2022. 05. 25. 23:32:02
 *  Author: gabor
 */ 

#ifndef MENU_H_
#define MENU_H_

#include "utils.h"
#include "button_controller.h"

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

/**
 * Initialize menu.
 */
void Menu_Init(void (*handler)(CurtainAction));

/**
 * Give a button change to the menu to process.
 */
void Menu_ButtonChangeHandler(ButtonChange chg);

/**
 * Refresh the menu. Only modifies screen content if motor is not running.
 */
void Menu_RefreshScreen();

#endif /* MENU_H_ */