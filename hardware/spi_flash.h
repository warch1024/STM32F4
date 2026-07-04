#ifndef __SPI_FLASH_H__
#define __SPI_FLASH_H__
#include "stm32f4xx.h"

#define SPI_FLASH_PORT GPIOB
#define SPI_FLASH_CS_PIN GPIO_Pin_14

#define SPI_FLASH_SCK_PIN GPIO_Pin_3
#define SPI_FLASH_MOSI_PIN GPIO_Pin_5
#define SPI_FLASH_MISO_PIN GPIO_Pin_4

#define SPI_FLASH_SCK_PIN_SOURCE GPIO_PinSource3
#define SPI_FLASH_MOSI_PIN_SOURCE GPIO_PinSource5
#define SPI_FLASH_MISO_PIN_SOURCE GPIO_PinSource4
#define SPI_FLASH_SELECT    GPIO_ResetBits(SPI_FLASH_PORT, SPI_FLASH_CS_PIN)
#define SPI_FLASH_DESELECT   GPIO_SetBits(SPI_FLASH_PORT, SPI_FLASH_CS_PIN)

// W25Q128 核心指令集
#define W25Q_READ_ID        0x9F    // 读取芯片ID
#define W25Q_WRITE_ENABLE   0x06    // 写使能
#define W25Q_READ_SR1       0x05    // 读状态寄存器1
#define W25Q_SECTOR_ERASE   0x20    // 4KB扇区擦除
#define W25Q_PAGE_PROGRAM   0x02    // 页编程(256字节)
#define W25Q_READ_DATA      0x03    // 数据读取

/** W25Q128 容量参数， 
 * 一页256字节，一扇区4KB，16页，一块64KB，16扇区 
 *  总容量为128Mb，16MB, 4096扇区
*/
#define W25Q128_PAGE_SIZE   256     // 一页256字节
#define W25Q128_SECTOR_SIZE 4096    // 一扇区4KB, 16页
#define W25Q128_SECTOR_COUNT  4096   // 一共4096个扇区

#define W25Q128_M_D_ID  0xef17  //厂商+设备ID


void spi_flash_init(void);
uint16_t spi_flash_read_id(void);

void test_spi_flash(void);
void spi_flash_force_write(void);
void spi_flash_record_TH(void);
void spi_flash_record_TH_qury(void);

#endif
 
