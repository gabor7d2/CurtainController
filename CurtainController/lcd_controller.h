//////////////////////////////////////////////////////////////////////////
/// 
/// Controller for 2x16 character LCD Display '1602A'
/// https://www.sparkfun.com/datasheets/LCD/HD44780.pdf
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
#define LCD_RS		PB4			// LCD register select pin
#define LCD_EN		PB5 		// LCD enable pin

/**
 * Initializes the LCD panel, sets it to 4-bit, 2-line mode
 * with cursor on and increment cursor enabled.
 */
void LCD_Init();

/**
 * Sends a command to the LCD panel.
 */
void LCD_SendCommand(uint8_t cmd);

/**
 * Sets the position of the cursor on the LCD.
 */
void LCD_SetCursorPos(uint8_t row, uint8_t column);

/**
 * Clears the LCD.
 */
void LCD_Clear();

/**
 * Writes an empty string to the LCD on the specified row.
 */
void LCD_ClearLine(uint8_t row);

/**
 * Prints a character to the LCD panel.
 * The character will be displayed at the current cursor position.
 */
void LCD_PrintChar(char data);

/**
 * Prints a character to the LCD panel at the specified row and column positions.
 */
void LCD_PrintCharAt(char data, uint8_t row, uint8_t column);

/**
 * Prints a string to the LCD panel, each character one by one.
 * The string will be displayed at the current cursor position.
 */
void LCD_PrintString(const char *str);

/**
 * Prints a string to the LCD panel at the specified row and column positions.
 */
void LCD_PrintStringAt(const char *str, uint8_t row, uint8_t column);

#endif