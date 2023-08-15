#ifndef __CLCD_H
#define __CLCD_H

#include "stm32f1xx_hal.h"
#include "config_map.h"

// #define LCD_EN 0x04  // Enable bit
// #define LCD_RW 0x02  // Read/Write bit
// #define LCD_RS 0x01  // Register select bit

#define CLCD_COMMAND 			0x00
#define CLCD_DATA 				0x01

// commands
#define LCD_CLEARDISPLAY 		0x01
#define LCD_RETURNHOME 			0x02

#define LCD_ENTRYMODESET 		0x04
#define LCD_DISPLAYCONTROL 		0x08
#define LCD_CURSORSHIFT 		0x10
#define LCD_FUNCTIONSET 		0x20
#define LCD_SETCGRAMADDR 		0x40
#define LCD_SETDDRAMADDR	 	0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 			0x00
#define LCD_ENTRYLEFT 			0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 			0x04
#define LCD_DISPLAYOFF 			0x00
#define LCD_CURSORON 			0x02
#define LCD_CURSOROFF 			0x00
#define LCD_BLINKON 			0x01
#define LCD_BLINKOFF 			0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 		0x08
#define LCD_CURSORMOVE 			0x00
#define LCD_MOVERIGHT 			0x04
#define LCD_MOVELEFT 			0x00

// flags for function set
#define LCD_8BITMODE 			0x10
#define LCD_4BITMODE 			0x00
#define LCD_2LINE 				0x08
#define LCD_1LINE 				0x00
#define LCD_5x10DOTS 			0x04
#define LCD_5x8DOTS 			0x00

#define LCD_BACKLIGHT 			0x08
#define LCD_NOBACKLIGHT 		0x00
typedef struct
{

	uint8_t COLUMS;
	uint8_t ROWS;
	uint8_t ENTRYMODE;
	uint8_t DISPLAYCTRL;
	uint8_t CURSORSHIFT;
	uint8_t FUNCTIONSET;
	uint8_t BACKLIGHT;
}CLCD_Name;
extern CLCD_Name LCD;
void Led_ON(void);
void Led_OFF(void);
void TranLCD(uint8_t Data);
void CLCD_Init(uint8_t Colums, uint8_t Rows);
void CLCD_SetCursor(uint8_t Xpos, uint8_t YPos);
void CLCD_WriteChar(char character);
void CLCD_WriteString(char *String);
void CLCD_Clear(void);
void CLCD_ReturnHome(void);
void CLCD_CursorOn(void);
void CLCD_CursorOff(void);
void CLCD_BlinkOn(void);
void CLCD_BlinkOff(void);
void LCD_setCursor(uint8_t col, uint8_t row);
void LCD_SendString(char *str);
#endif
