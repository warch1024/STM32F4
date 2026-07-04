#ifndef __OLED_IIC_H
#define	__OLED_IIC_H

#include "stm32f4xx.h"

#define OLED_ADDRESS	0x78 //OLED的I2C地址，8位地址
#define OLED_7BIT_ADDR	0x3c //OLED的寄存器地址，7位地址
#define USE_SOFT_IIC	1 //是否使用模拟I2C,0:使用硬件I2C,1:使用模拟I2C

void I2C_Configuration(void);
void I2C_WriteByte(uint8_t addr,uint8_t data);
void WriteCmd(unsigned char I2C_Command);
void WriteDat(unsigned char I2C_Data);
void OLED_Init(void);
void OLED_SetPos(unsigned char x, unsigned char y);
void OLED_Fill(unsigned char fill_Data);
void OLED_CLS(void);
void OLED_ON(void);
void OLED_OFF(void);
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize);
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N);
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
void oled_demo(void);
#endif



