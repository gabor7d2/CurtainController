//////////////////////////////////////////////////////////////////////////
/// 
/// Controller for 2x16 character LCD Display '1602A'
/// https://www.openhacks.com/uploadsproductos/eone-1602a1.pdf
///
/// Uses port bits 0-3 of the port specified below, and the specified 
/// port bits for Reg. Select and Enable signals. The R/W signal of
/// the LCD is tied LOW as there is no need to read from the LCD in this program.
///
/// Call LCD_Init() before using this module.
///
//////////////////////////////////////////////////////////////////////////

#ifndef DISPLAY_H
#define DISPLAY_H

#include "utils.h"

#define LCD_Dir		DDRB		// LCD data direction register
#define LCD_Port	PORTB		// LCD data output register
#define LCD_RS		PB5			// LCD register select pin
#define LCD_EN		PB4 		// LCD enable pin

/**
 * Initializes the LCD panel, sets it to 4-bit, 2-line mode
 * with cursor on and increment cursor enabled.
 */
void LCD_Init();

/**
 * Sends a command to the LCD panel.
 */
void LCD_SendCommand(unsigned char cmd);

/**
 * Sets the position of the cursor on the LCD.
 */
void LCD_SetCursorPos(char row, char column);

/**
 * Clears the LCD.
 */
void LCD_Clear();

/**
 * Prints a character to the LCD panel.
 * The character will be displayed at the current cursor position.
 */
void LCD_PrintChar(unsigned char data);

/**
 * Prints a character to the LCD panel at the specified row and column positions.
 */
void LCD_PrintCharAt(unsigned char data, char row, char column);

/**
 * Prints a string to the LCD panel, each character one by one.
 * The string will be displayed at the current cursor position.
 */
void LCD_PrintString(const char *str);

/**
 * Prints a string to the LCD panel at the specified row and column positions.
 */
void LCD_PrintStringAt(const char *str, char row, char column);

#endif