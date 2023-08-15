/******************************************************************************************************************
@File:  	CLCD I2C Chip PCF8574
@Author:  Khue Nguyen
@Website: khuenguyencreator.com
@Youtube: https://www.youtube.com/channel/UCt8cFnPOaHrQXWmVkk-lfvg
Huong dan su dung:
- Su dung thu vien HAL
- Khoi tao bien LCD: CLCD_Name LCD1;
- Khoi tao LCD do: CLCD_Init(&LCD1,&hi2c1,0x4e,20,4);
- Su dung cac ham phai truyen dia chi cua LCD do: 
CLCD_SetCursor(&LCD1, 0, 1);
CLCD_WriteString(&LCD1,"hello anh em ");
******************************************************************************************************************/
#include "LCD_I2C.h"

//************************** Low Level Function ****************************************************************//
CLCD_Name LCD;

static void CLCD_Delay(uint16_t Time)
{
	HAL_Delay(Time);
}
void Led_ON()
{
	HAL_GPIO_WritePin(LCD_PORT,LCD_RS,GPIO_PIN_SET);
}
void Led_OFF()
{
	HAL_GPIO_WritePin(LCD_PORT,LCD_RS,GPIO_PIN_RESET);
}
static void CLCD_Write( uint8_t Data, uint8_t Mode)
{
	uint8_t Data_H = Data >>4;
	uint8_t Data_L = Data;

	if(Mode == CLCD_DATA)
	{
		HAL_GPIO_WritePin(LCD_PORT,LCD_RS,GPIO_PIN_SET);
	}
	else if(Mode == CLCD_COMMAND)
	{
		HAL_GPIO_WritePin(LCD_PORT,LCD_RS,GPIO_PIN_RESET);
	}
	HAL_GPIO_WritePin(LCD_PORT, LCD_D4, Data_H&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_PORT, LCD_D5, Data_H>>1&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_PORT, LCD_D6, Data_H>>2&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_PORT, LCD_D7, Data_H>>3&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);


	HAL_GPIO_WritePin(LCD_PORT, LCD_EN, GPIO_PIN_RESET);
	CLCD_Delay(1);
	HAL_GPIO_WritePin(LCD_PORT, LCD_EN, GPIO_PIN_SET);
	CLCD_Delay(1);
	HAL_GPIO_WritePin(LCD_PORT, LCD_EN, GPIO_PIN_RESET);
	CLCD_Delay(1);

	HAL_GPIO_WritePin(LCD_PORT, LCD_D4, Data_L&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_PORT, LCD_D5, Data_L>>1&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_PORT, LCD_D6, Data_L>>2&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_PORT, LCD_D7, Data_L>>3&0x01?GPIO_PIN_SET:GPIO_PIN_RESET);


	HAL_GPIO_WritePin(LCD_PORT, LCD_EN, GPIO_PIN_RESET);
	CLCD_Delay(1);
	HAL_GPIO_WritePin(LCD_PORT, LCD_EN, GPIO_PIN_SET);
	CLCD_Delay(1);
	HAL_GPIO_WritePin(LCD_PORT, LCD_EN, GPIO_PIN_RESET);
	CLCD_Delay(1);
}


//************************** High Level Function ****************************************************************//
void CLCD_Init( uint8_t Colums, uint8_t Rows)
{
	LCD.COLUMS = Colums;
	LCD.ROWS = Rows;
	
	LCD.FUNCTIONSET = LCD_FUNCTIONSET|LCD_4BITMODE|LCD_2LINE|LCD_5x8DOTS;
	LCD.ENTRYMODE = LCD_ENTRYMODESET|LCD_ENTRYLEFT|LCD_ENTRYSHIFTDECREMENT;
	LCD.DISPLAYCTRL = LCD_DISPLAYCONTROL|LCD_DISPLAYON|LCD_CURSOROFF|LCD_BLINKOFF;
	LCD.CURSORSHIFT = LCD_CURSORSHIFT|LCD_CURSORMOVE|LCD_MOVERIGHT;
	//LCD.BACKLIGHT = LCD_BACKLIGHT;

	CLCD_Delay(50);
	CLCD_Write(0x33, CLCD_COMMAND);
//	CLCD_Delay(5);
	CLCD_Write(0x33, CLCD_COMMAND);
	CLCD_Delay(5);
	CLCD_Write(0x32, CLCD_COMMAND);
	CLCD_Delay(5);

	
	CLCD_Write(LCD.ENTRYMODE,CLCD_COMMAND);
	CLCD_Write(LCD.DISPLAYCTRL,CLCD_COMMAND);
	CLCD_Write(LCD.CURSORSHIFT,CLCD_COMMAND);
	CLCD_Write(LCD.FUNCTIONSET,CLCD_COMMAND);
	
	CLCD_Write(LCD_CLEARDISPLAY,CLCD_COMMAND);
	CLCD_Write(LCD_RETURNHOME,CLCD_COMMAND);
	
}
void CLCD_SetCursor( uint8_t Xpos, uint8_t Ypos)
{
	uint8_t DRAM_ADDRESS = 0x00;
	uint8_t DRAM_OFFSET[4] = {0x00, 0x40, 0x14, 0x54};
	if(Xpos >= LCD.COLUMS)
	{
		Xpos = LCD.COLUMS - 1;
	}
	if(Ypos >= LCD.ROWS)
	{
		Ypos = LCD.ROWS -1;
	}
	DRAM_ADDRESS = DRAM_OFFSET[Ypos] + Xpos;
	CLCD_Write(LCD_SETDDRAMADDR|DRAM_ADDRESS, CLCD_COMMAND);
}
void CLCD_WriteChar( char character)
{
	CLCD_Write(character, CLCD_DATA);
}
void CLCD_WriteString( char *String)
{
	while(*String)CLCD_WriteChar(*String++);
}
void CLCD_Clear()
{
	CLCD_Write(LCD_CLEARDISPLAY, CLCD_COMMAND);
	CLCD_Delay(5);
}
void CLCD_ReturnHome()
{
	CLCD_Write(LCD_RETURNHOME, CLCD_COMMAND);
	CLCD_Delay(5);
}
void CLCD_CursorOn()
{
	LCD.DISPLAYCTRL |= LCD_CURSORON;
	CLCD_Write(LCD.DISPLAYCTRL, CLCD_COMMAND);
}
void CLCD_CursorOff()
{
	LCD.DISPLAYCTRL &= ~LCD_CURSORON;
	CLCD_Write(LCD.DISPLAYCTRL, CLCD_COMMAND);
}
void CLCD_BlinkOn()
{
	LCD.DISPLAYCTRL |= LCD_BLINKON;
	CLCD_Write(LCD.DISPLAYCTRL, CLCD_COMMAND);
}
void CLCD_BlinkOff()
{
	LCD.DISPLAYCTRL &= ~LCD_BLINKON;
	CLCD_Write(LCD.DISPLAYCTRL, CLCD_COMMAND);
}
