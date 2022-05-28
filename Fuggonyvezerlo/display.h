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

#include <avr/io.h>
#include <util/delay.h>

#define LCD_Dir		DDRB		// LCD data direction register
#define LCD_Port	PORTB		// LCD data output register
#define LCD_RS		PB4			// LCD register select pin
#define LCD_EN		PB5 		// LCD enable pin

/**
* Sends a command to the LCD panel.
*/
void LCD_SendCommand(unsigned char cmd)
{
	LCD_Port = (LCD_Port & 0xF0) | (cmd >> 4);		// Send upper 4 bits
	LCD_Port &= ~(1<<LCD_RS);						// RS=0 (command reg)
	
	LCD_Port |= (1<<LCD_EN);						// Enable pulse
	_delay_us(1);
	LCD_Port &= ~ (1<<LCD_EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0xF0) | (cmd & 0x0F);	// Send lower 4 bits
	
	LCD_Port |= (1<<LCD_EN);						// Enable pulse
	_delay_us(1);
	LCD_Port &= ~ (1<<LCD_EN);
	
	_delay_ms(2);
}

/**
* Prints a character to the LCD panel.
* The character will be displayed at the current cursor position.
*/
void LCD_PrintChar(unsigned char data)
{
	LCD_Port = (LCD_Port & 0xF0) | (data >> 4);		// Send upper 4 bits
	LCD_Port |= (1<<LCD_RS);						// RS=1 (data reg)
	
	LCD_Port |= (1<<LCD_EN);						// Enable pulse
	_delay_us(1);
	LCD_Port &= ~ (1<<LCD_EN);

	_delay_us(200);

	LCD_Port = (LCD_Port & 0xF0) | (data & 0x0F);	// Send lower 4 bits
	
	LCD_Port |= (1<<LCD_EN);						// Enable pulse
	_delay_us(1);
	LCD_Port &= ~ (1<<LCD_EN);
	
	_delay_ms(2);
}

void LCD_SetCursorPos(char row, char column) {
	// Set cursor position to the specified position
	if (row == 0 && column<16)
	LCD_SendCommand((column & 0x0F)|0x80);
	else if (row == 1 && column<16)
	LCD_SendCommand((column & 0x0F)|0xC0);
}

/**
* Prints a character to the LCD panel at the specified row and column positions.
*/
void LCD_PrintCharAt(unsigned char data, char row, char column)
{
	LCD_SetCursorPos(row, column);
	LCD_PrintChar(data);
}

/**
* Initializes the LCD panel, sets it to 4-bit, 2-line mode
* with cursor off and increment cursor enabled.
*/
void LCD_Init()
{
	LCD_Dir = 0xFF;					// Set port direction as output
	_delay_ms(20);					// Wait for LCD to power up

	LCD_SendCommand(0x02);			// Set 4-bit mode
	LCD_SendCommand(0x28);			// Set 2 line, 5*7 matrix mode
	LCD_SendCommand(0b00001110);	// Set display=on, cursor=off
	LCD_SendCommand(0x06);			// Enable Increment cursor
	LCD_SendCommand(0x01);			// Clear screen
	_delay_ms(10);
}

/**
* Prints a string to the LCD panel, each character one by one.
* The string will be displayed at the current cursor position.
*/
void LCD_PrintString(const char *str)
{
	for(int i=0; str[i] != 0; i++)
	{
		LCD_PrintChar(str[i]);
	}
}

/**
* Prints a string to the LCD panel at the specified row and column positions.
*/
void LCD_PrintStringAt(const char *str, char row, char column)
{
	LCD_SetCursorPos(row, column);
	LCD_PrintString(str);
}

/**
* Clears the LCD.
*/
void LCD_Clear()
{
	LCD_SendCommand (0x01);		// Clear screen
	_delay_ms(2);
	LCD_SendCommand (0x80);		// Set cursor position to the beginning
}

#endif