#ifndef AT24CXX_H
#define AT24CXX_H
#include "stm32f1xx_hal.h"
#define		_EEPROM_USE_WP_PIN		0
struct AT24Cxx
{
    I2C_HandleTypeDef  *_i2c;
	uint32_t eeprom_size;
	uint8_t i2c_address;
	#if(_EEPROM_USE_WP_PIN == 1)
	GPIO_TypeDef *GPIOx;
	uint16_t GPIO_Pin;
	#endif
};
void AT24Cxx_Init(struct AT24Cxx *rom,I2C_HandleTypeDef *i2c,uint8_t i2c_address, uint32_t eeprom_size);
uint8_t AT24_read(struct AT24Cxx *rom,uint16_t address);
void AT24_write(struct AT24Cxx *rom,uint16_t address, uint8_t value);
void AT24_clear(struct AT24Cxx *rom);
#if(_EEPROM_USE_WP_PIN == 1)
void AT24Cxx_Init_WP_Pin(struct AT24Cxx *rom,GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void enabel_ROM(struct AT24Cxx *rom);
void look_ROM(struct AT24Cxx *rom);
#endif
#endif
