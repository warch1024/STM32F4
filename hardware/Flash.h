#ifndef __FLASH_H__
#define __FLASH_H__
#include "stm32f4xx.h"
#define SECTOR_5_BASE (0x8020000)

typedef struct Sector_RW_Config_t{
    uint32_t sector_base_address;//扇区基地址
    uint32_t current_address;//当前写入地址
    uint16_t sector_number;//扇区号
    uint16_t sector_K_size;//扇区大小（KB）
}sector_rw_config_t;
extern sector_rw_config_t CURRENT_sector_info;


void Flash_Init(void);
void Flash_Unlock(void);
uint8_t sector_read_byte(uint32_t address);
uint32_t sector_read_word(uint32_t address);
int8_t sector_program_bytes(uint32_t base_address,uint32_t sector_base, uint16_t sector_K_size, 
    uint8_t *bytes, uint16_t byte_count);
int8_t sector_program_by_bytes(sector_rw_config_t *sector_info, uint8_t *bytes, uint16_t byte_count);
void flash_test(void);



#endif


