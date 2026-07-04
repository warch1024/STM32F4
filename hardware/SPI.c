#include "SPI.h"
#include "stm32f4xx.h"
#include "spi_flash.h"
#include "stdio.h"

void SPI1_Init(void){
    // 初始化SPI1， 用于与W25Q18F通信
    GPIO_InitTypeDef  GPIO_InitStructure; 
	SPI_InitTypeDef SPI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);    //spi1--PB3 PB4 PB5 PB14

	//初始化GPIO为SPI复用功能(片选为输出)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//
	GPIO_InitStructure.GPIO_Pin = SPI_FLASH_SCK_PIN|SPI_FLASH_MOSI_PIN|SPI_FLASH_MISO_PIN;//
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//无上下拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//输出速度
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	//将PN3 PB4 PB5复用功能映射到SPI1
	GPIO_PinAFConfig(GPIOB,SPI_FLASH_SCK_PIN_SOURCE,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB,SPI_FLASH_MOSI_PIN_SOURCE,GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOB,SPI_FLASH_MISO_PIN_SOURCE,GPIO_AF_SPI1);
	//PB14-片选 输出，并且默认高电平
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;//
	GPIO_InitStructure.GPIO_Pin = SPI_FLASH_CS_PIN;//
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//无上下拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//输出速度
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	//关闭片选
	GPIO_SetBits(SPI_FLASH_PORT,SPI_FLASH_CS_PIN);
	
	//初始化SPI1 ----- mode 0
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//双线全双工SPI_InitStructure
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;//主设备
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;//一次传输8位
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;//高位在前 MSB
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;//默认低电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;//第一个边沿采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;//片选软件控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;//波特率分频系数 84M / 8 = 10.5M
	SPI_Init(SPI1,&SPI_InitStructure);
	
	//使能SPI1
	SPI_Cmd(SPI1,ENABLE);
}
//发送接收数据
//data ---- 要发送的数据
//返回值 ---- 收到的数据
int16_t spi1_send_recv_data(uint8_t data)
{
    int16_t recv_data = -1;
	//先发再收
	//等待上一个数据发送完成后发送数据
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)!=SET);
	SPI_I2S_SendData(SPI1,data);
	
	//发送接收同时进行，发送的时候就要接收
	//等待接收完成
	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)!=SET);
	recv_data = SPI_I2S_ReceiveData(SPI1);
	return recv_data;
}



