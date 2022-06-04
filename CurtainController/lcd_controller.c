#include "lcd_controller.h"
#include <avr/io.h>
#include <util/delay.h>

void LCD_Init()
{
	LCD_Dir = 0xFF;					// Set port direction as output
	_delay_ms(20);					// Wait for LCD to power up

	LCD_SendCommand(0x02);			// Set 4-bit mode
	LCD_SendCommand(0x28);			// Set 2 line, 5*7 matrix mode
	LCD_SendCommand(0b00001110);	// Set display=on, cursor=on
	LCD_SendCommand(0x06);			// Enable Increment cursor
	LCD_SendCommand(0x01);			// Clear screen
	_delay_ms(10);
}

void LCD_SendCommand(uint8_t cmd)
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
	
	_delay_ms(1);
}

void LCD_SetCursorPos(uint8_t row, uint8_t column)
{
	// Set cursor position to the specified position
	if (row == 0 && column < 32)
		LCD_SendCommand((column & 0x1F) | 0x80);
	else if (row == 1 && column < 32)
		LCD_SendCommand((column & 0x1F) | 0xC0);
}

void LCD_Clear()
{
	LCD_SendCommand (0x01);		// Clear screen
	LCD_SendCommand (0x80);		// Set cursor position to the beginning
}

void LCD_ClearLine(uint8_t row) {
	LCD_PrintStringAt("                ", row, 0);
}

void LCD_PrintChar(char data)
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
	
	_delay_ms(1);
}

void LCD_PrintCharAt(char data, uint8_t row, uint8_t column)
{
	LCD_SetCursorPos(row, column);
	LCD_PrintChar(data);
}

void LCD_PrintString(const char *str)
{
	for(int i=0; str[i] != 0; i++)
	{
		LCD_PrintChar(str[i]);
	}
}

void LCD_PrintStringAt(const char *str, uint8_t row, uint8_t column)
{
	LCD_SetCursorPos(row, column);
	LCD_PrintString(str);
}