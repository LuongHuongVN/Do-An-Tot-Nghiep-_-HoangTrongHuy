#ifndef __CONFIG_PIN_MAP
#define __CONFIG_PIN_MAP
/*
File config các chân gpio
Các thiết bị

Led báo:
    Xanh lá : B11
    Đỏ : B10
Còi : B1
Cảm biến HALL : B0
Bàn phím : 
    Hàng 1 A11
    Hàng 2 A10
    Hàng 3 A9
    Hàng 4 A8
    Cột 1 B5
    Cột 2 B4
    Cột 3 B3
RFID :
rst:A12
    CE : B12
    SCK : B13
    MOSI : B14
    MISO : B15
LCD:
    RS : A0
    RW : A1
    E : A2
    D4 : A3
    D5 : A4
    D6 : A5
    D7 : A6
    LED : A7
RELAY: B8
*/
// define led
#define LED_PORT GPIOB
#define LED_BLUE_PIN GPIO_PIN_11
#define LED_RED_PIN GPIO_PIN_10

#define LED_BLUE_ON HAL_GPIO_WritePin(LED_PORT,LED_BLUE_PIN,GPIO_PIN_RESET)
#define LED_BLUE_OFF HAL_GPIO_WritePin(LED_PORT,LED_BLUE_PIN,GPIO_PIN_RESET)

#define LED_RED_ON HAL_GPIO_WritePin(LED_PORT,LED_RED_PIN,GPIO_PIN_SET)
#define LED_RED_OFF HAL_GPIO_WritePin(LED_PORT,LED_RED_PIN,GPIO_PIN_RESET)


// define buzz
#define BUZZ_PORT GPIOB
#define BUZZ_PIN GPIO_PIN_1
#define BUZZ_ON HAL_GPIO_WritePin(BUZZ_PORT,BUZZ_PIN,GPIO_PIN_SET)
#define BUZZ_OFF HAL_GPIO_WritePin(BUZZ_PORT,BUZZ_PIN,GPIO_PIN_RESET)
// define key
#define KEY_ROW_PORT GPIOA
#define KEY_ROW1_PIN GPIO_PIN_11
#define KEY_ROW2_PIN GPIO_PIN_10
#define KEY_ROW3_PIN GPIO_PIN_9
#define KEY_ROW4_PIN GPIO_PIN_8
#define KEY_COL_PORT GPIOB
#define KEY_COL1_PIN GPIO_PIN_5
#define KEY_COL2_PIN GPIO_PIN_4
#define KEY_COL3_PIN GPIO_PIN_3
// define rfid
#define RFID_PORT GPIOB
#define RFID_CE_PIN GPIO_PIN_12
#define RFID_SCK_PIN GPIO_PIN_13
#define RFID_MOSI_PIN GPIO_PIN_14
#define RFID_MISO_PIN GPIO_PIN_15

#define RFID_PORT_RST GPIOA
#define RFID_RST_PIN GPIO_PIN_12
// define lcd
#define LCD_PORT GPIOA
#define LCD_RS GPIO_PIN_0
#define LCD_RW GPIO_PIN_1
#define LCD_EN GPIO_PIN_2
#define LCD_D4 GPIO_PIN_3
#define LCD_D5 GPIO_PIN_4
#define LCD_D6 GPIO_PIN_5
#define LCD_D7 GPIO_PIN_6
#define LCD_LED GPIO_PIN_7
#define LCD_LED_ON HAL_GPIO_WritePin(LCD_PORT,LCD_LED,GPIO_PIN_SET)
#define LCD_LED_OFF HAL_GPIO_WritePin(LCD_PORT,LCD_LED,GPIO_PIN_RESET)
#endif