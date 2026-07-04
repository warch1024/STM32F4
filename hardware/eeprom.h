#ifndef __EEPROM_H__
#define __EEPROM_H__
#include "stm32f4xx.h"
#define EEPROM_SCL GPIOB, GPIO_Pin_8
#define EEPROM_SDA GPIOB, GPIO_Pin_9

#define EEPROM_ADDR 0x50 

void eeprom_init(void);
uint8_t eeprom_write_byte(uint8_t iic_addr, uint8_t byte_addr, uint8_t byte);
int16_t eeprom_read_byte(uint8_t iic_addr, uint8_t byte_addr);
void eeprom_test(void);


#endif





