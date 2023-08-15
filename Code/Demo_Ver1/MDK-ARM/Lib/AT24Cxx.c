#include "AT24Cxx.h"
#include "stm32f1xx_hal.h"
void AT24Cxx_Init(struct AT24Cxx *rom,I2C_HandleTypeDef *i2c,uint8_t i2c_address, uint32_t eeprom_size)
{
    rom->_i2c = i2c;
	rom->i2c_address = i2c_address;
	rom->eeprom_size = eeprom_size;
}
uint8_t AT24_read(struct AT24Cxx *rom,uint16_t address)
{
    uint8_t adr[2];
    adr[0] = (address>>8)&0xff;

    adr[1] = address&0xff;
    HAL_I2C_Master_Transmit(rom->_i2c,rom->i2c_address,&adr[0],2,100);
	uint8_t data;
	HAL_I2C_Master_Receive(rom->_i2c,rom->i2c_address,&data,1,20);
	return data;
}
void AT24_write(struct AT24Cxx *rom,uint16_t address, uint8_t value)
{
	if(AT24_read(rom,address) != value)
	{
	uint8_t adr[3];
    adr[0] = (address>>8)&0xff;

    adr[1] = address&0xff;
	 adr[2] = value;
	HAL_I2C_Master_Transmit(rom->_i2c,rom->i2c_address,&adr[0],3,100);
	HAL_Delay(5);
	}
	
}
void AT24_clear(struct AT24Cxx *rom)
{
	uint16_t size = rom->eeprom_size*128;
	uint16_t couter;

	for(couter = 0 ;couter < size;couter++)
	{
		AT24_write(rom,couter,255);
	}
}
#if(_EEPROM_USE_WP_PIN == 1)
void AT24Cxx_Init_WP_Pin(struct AT24Cxx *rom,GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    
    rom->GPIOx = GPIOx;
    rom->GPIO_Pin = GPIO_Pin;
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(GPIOx == GPIOA)  __HAL_RCC_GPIOA_CLK_ENABLE();
    else if(GPIOx == GPIOB)  __HAL_RCC_GPIOB_CLK_ENABLE();
    else if(GPIOx == GPIOC)  __HAL_RCC_GPIOC_CLK_ENABLE();
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
}
void enabel_ROM(struct AT24Cxx *rom)
{
    HAL_GPIO_WritePin(rom->GPIOx, rom->GPIO_Pin, GPIO_PIN_RESET);
}
void look_ROM(struct AT24Cxx *rom)
{
    HAL_GPIO_WritePin(rom->GPIOx, rom->GPIO_Pin, GPIO_PIN_SET);
}
#endif
