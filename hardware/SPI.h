#ifndef __SPI_H__
#define __SPI_H__
#include "stm32f4xx.h"

void SPI1_Init(void);
int16_t spi1_send_recv_data(uint8_t data);


#endif



