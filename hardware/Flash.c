#include "Flash.h"
#include "stm32f4xx.h"
#include "stdio.h"

sector_rw_config_t CURRENT_sector_info = {0x8020000, 0x8020000, 7, 128};    //当前正在操作的扇区信息
void Flash_Init(void){
    CURRENT_sector_info.current_address = 0x8060000;   //当前写入地址的偏移地址
    CURRENT_sector_info.sector_number = 7;  //当前的扇区号为7
    CURRENT_sector_info.sector_K_size = 128;  //当前的扇区大小为128KB
    CURRENT_sector_info.sector_base_address = 0x8060000;//当前的扇区为扇区7的基地址
}
inline void Flash_Unlock(void)
{
    // 解锁Flash
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR);
}

uint32_t sector_read_word(uint32_t address)
{
    // 读取Flash扇区的某个字
    return *(uint32_t *)address;
}
uint8_t sector_read_byte(uint32_t address){
    // 读取Flash扇区的某个字节 
    return *(uint8_t *)address;
}

int8_t sector_program_bytes(uint32_t base_address,uint32_t sector_base, uint16_t sector_K_size, 
    uint8_t *bytes, uint16_t byte_count){
    // 编程Flash扇区的某个字节
    if (base_address < sector_base || base_address >= sector_base + sector_K_size * 1024)
    {
        return -1;  //地址超出扇区范围
    }
    if(sector_K_size * 1024 < byte_count){  // 检查扇区大小是否足够
        return -2;  //写入内容超出扇区大小
    }
    int8_t ret = 0;
    for (uint16_t i = 0; i < byte_count && bytes != NULL; i++){
        if((ret = FLASH_ProgramByte(base_address + i, bytes[i])) != FLASH_COMPLETE){
            return ret;  //写入失败
        }
    }
    return FLASH_COMPLETE;  //写入成功
}
// 编程Flash扇区的某个字节
// sector_info: 扇区信息指针
// bytes: 要写入的字节数组指针
// byte_count: 要写入的字节数
// 返回值: FLASH_COMPLETE 成功，其他值 失败
int8_t sector_program_by_bytes(sector_rw_config_t *sector_info, uint8_t *bytes, uint16_t byte_count){
    // 编程Flash扇区的某个字节
    if (sector_info->current_address < sector_info->sector_base_address || 
        sector_info->current_address >= sector_info->sector_base_address + sector_info->sector_K_size * 1024){
        return -1;  //地址超出扇区范围
    }
    if(sector_info->sector_K_size * 1024 < byte_count){  // 检查扇区大小是否足够
        return -2;  //写入内容超出扇区大小
    }
    int8_t ret = 0;
    for (uint16_t i = 0; i < byte_count && bytes != NULL; i++){
        if((ret = FLASH_ProgramByte(sector_info->current_address ++, bytes[i])) != FLASH_COMPLETE){
            return ret;  //写入失败
        }
    }
    return FLASH_COMPLETE;  //写入成功
}
void flash_test(void)
{
    // 测试Flash
    Flash_Unlock();
    FLASH_EraseSector(FLASH_Sector_7, VoltageRange_3);
    char test[] = "Hello, World!,123+++==##dfdfs@@@@你好a\r\n";
    uint8_t ret = sector_program_by_bytes(&CURRENT_sector_info, (uint8_t *)test, sizeof(test));
    if(ret != FLASH_COMPLETE){
        printf("sector_program_by_bytes failed, ret = %d\r\n", ret);
    }
    else{
        printf("sector_program_by_bytes success\r\n");
        for (uint16_t i = 0; i < sizeof(test); i++){
            printf("%c", sector_read_byte(CURRENT_sector_info.sector_base_address + i));
        }
        printf("\r\n");
    }
    FLASH_Lock();
}




