#ifndef DISPLAY_H
#define DISPLAY_H

#include <avr/io.h>
#include <util/delay.h>

#define LCD_Dir		DDRB		// LCD data direction register
#define LCD_Port	PORTB		// LCD data output register
#define LCD_RS		PB4			// LCD register select pin
#define LCD_EN		PB5 		// LCD enable pin

/**
Sends a command to the LCD panel.
*/
void LCD_Command(unsigned char cmd)
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
Sends a character to the LCD panel.
The character will be displayed at the current cursor position.
*/
void LCD_Char(unsigned char data)
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

/**
Initializes the LCD panel, sets it to 4-bit, 2-line mode
with cursor off and increment cursor enabled.
*/
void LCD_Init (void)
{
	LCD_Dir = 0xFF;			// Set port direction as output
	_delay_ms(20);			// Wait for LCD to power up

	LCD_Command(0x02);		// Set 4-bit mode
	LCD_Command(0x28);      // Set 2 line, 5*7 matrix mode
	LCD_Command(0b00001110 /*00001 Display Cursor Blink*/);      // Set display=on, cursor=off
	LCD_Command(0x06);      // Enable Increment cursor
	LCD_Command(0x01);      // Clear screen
	_delay_ms(2);
}

/**
Sends a string to the LCD panel, each character one by one.
The string will be displayed at the current cursor position.
*/
void LCD_String (const char *str)
{
	for(int i=0; str[i] != 0; i++)
	{
		LCD_Char(str[i]);
	}
}

/**
Sends a string to the LCD panel to the specified row and column positions.
*/
void LCD_String_xy (const char *str, char row, char column)
{
	// Set cursor position to the specified position
	if (row == 0 && column<16)
	LCD_Command((column & 0x0F)|0x80);
	else if (row == 1 && column<16)
	LCD_Command((column & 0x0F)|0xC0);
	
	// Send string
	LCD_String(str);
}

/**
Clears the LCD.
*/
void LCD_Clear()
{
	LCD_Command (0x01);		// Clear screen
	_delay_ms(2);
	LCD_Command (0x80);		// Set cursor position to the beginning
}

#endif