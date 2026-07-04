#include "spi_flash.h"
#include "stm32f4xx.h"
#include "SPI.h"
#include <stdio.h>
#include "temperature_humidity_sensor.h"
#include "RTC.h"
#include <string.h>

static uint8_t page_buf[W25Q128_PAGE_SIZE];//一页256字节
static uint8_t page_cache[W25Q128_PAGE_SIZE];//一页256字节
static uint16_t current_page_byte_index = 0;//当前写入的字节索引
static uint32_t current_write_page_index = 0;//当前记录的页地址
static uint32_t current_write_sector_index = 0;//当前记录的扇区地址

void spi_flash_init(void){
    // 初始化SPI1
    SPI1_Init();
    memset(page_buf, 0xFF, sizeof(page_buf));//清空页缓冲区
    memset(page_cache, 0xFF, sizeof(page_cache));//清空页缓存区
}

uint16_t spi_flash_read_id(void){
    // 读取W25Q18F的ID
    uint16_t id = 0xF3F3;
    SPI_FLASH_SELECT;
    spi1_send_recv_data(0x90);//读取ID命令
    //发送24位地址
    spi1_send_recv_data(0x00);//
    spi1_send_recv_data(0x00);//
    spi1_send_recv_data(0x00);//
    //接收ID
    // 检查返回值是否有效
    int16_t rec_data = spi1_send_recv_data(0xFF);
    if (rec_data == -1) {
        printf("Error: Failed to receive manufacturer ID byte\n");
        return 0xFFFF; // 返回错误值
    }
    id = 0x00;
    id |= rec_data << 8;
    rec_data = spi1_send_recv_data(0xFF);
    if (rec_data == -1) {
        printf("Error: Failed to receive device ID byte\n");
        return 0xFFFF; // 返回错误值
    }
    id |= rec_data;
    SPI_FLASH_DESELECT;
    return id;
}

uint8_t spi_flash_read_status(void){
    // 读取W25Q18F的状态寄存器, 1 = 忙
    uint8_t status = 0x00;
    SPI_FLASH_SELECT;
    spi1_send_recv_data(W25Q_READ_SR1);//读取状态寄存器命令
    //接收状态寄存器
    // 检查返回值是否有效
    int16_t rec_data = spi1_send_recv_data(0xFF);
    if (rec_data == -1) {
        printf("Error: Failed to receive status byte\n");
        return 0xFF; // 返回错误值
    }
    status = rec_data;
    SPI_FLASH_DESELECT;
    return status;
}
void spi_flash_wait_for_ready(void){
    // 等待W25Q18F就绪
    while(spi_flash_read_status() & 0x01);
}
void spi_flash_write_enable(void){
    // 使能W25Q18F的写操作
    SPI_FLASH_SELECT;
    spi1_send_recv_data(W25Q_WRITE_ENABLE);//使能写命令
    SPI_FLASH_DESELECT;
}
void spi_flash_erase_4k_sector(uint32_t sector_addr){
    // 擦除W25Q18F的4KB扇区
    if(sector_addr % W25Q128_SECTOR_SIZE != 0){
        printf("Error: Invalid sector address\n");
        return;
    }
    spi_flash_write_enable();
    spi_flash_wait_for_ready();
    SPI_FLASH_SELECT;
    spi1_send_recv_data(W25Q_SECTOR_ERASE);//擦除命令
    spi1_send_recv_data(sector_addr >> 16);//高8位
    spi1_send_recv_data(sector_addr >> 8);//中8位
    spi1_send_recv_data(sector_addr);//低8位
    SPI_FLASH_DESELECT;
    spi_flash_wait_for_ready();//等待擦除完成
}
void spi_flash_write_page(uint32_t page_addr, uint8_t *data, uint16_t len){
    // 写入W25Q18F的页
    if(page_addr % W25Q128_PAGE_SIZE != 0){
        printf("Error: Invalid page address\n");
        return;
    }
    if(len > W25Q128_PAGE_SIZE){
        len = W25Q128_PAGE_SIZE;
    }
    spi_flash_write_enable();
    spi_flash_wait_for_ready();
    SPI_FLASH_SELECT;
    spi1_send_recv_data(W25Q_PAGE_PROGRAM);//编程命令
    //发送32位地址
    spi1_send_recv_data(page_addr >> 16);//高8位
    spi1_send_recv_data(page_addr >> 8);//中8位
    spi1_send_recv_data(page_addr);//低8位
    //发送数据
    for(uint16_t i = 0; i < len; i++){
        spi1_send_recv_data(data[i]);
    }
    SPI_FLASH_DESELECT;
    spi_flash_wait_for_ready();//等待编程完成
}
void spi_flash_read_page(uint32_t page_addr, uint8_t *data, uint16_t len){
    // 读取W25Q18F的页
    if(page_addr % W25Q128_PAGE_SIZE != 0){
        printf("Error: Invalid page address\n");
        return;
    }
    if(len > W25Q128_PAGE_SIZE){
        len = W25Q128_PAGE_SIZE;
    }
    SPI_FLASH_SELECT;
    spi1_send_recv_data(W25Q_READ_DATA);//读取命令
    //发送32位地址
    spi1_send_recv_data(page_addr >> 16);//高8位
    spi1_send_recv_data(page_addr >> 8);//中8位
    spi1_send_recv_data(page_addr);//低8位
    //接收数据
    for(uint16_t i = 0; i < len; i++){
        data[i] = spi1_send_recv_data(0xFF);
    }
    SPI_FLASH_DESELECT;
}
void spi_flash_random_read(uint32_t page_addr, uint8_t *data, uint16_t len){
    // 读取W25Q18F的随机数据
    if(page_addr > 0x00FFFFFF || page_addr < 0x0){
        printf("Error: Invalid random address\n");
        return;
    }
    SPI_FLASH_SELECT;
    spi1_send_recv_data(W25Q_READ_DATA);//读取命令
    //发送32位地址
    spi1_send_recv_data(page_addr >> 16);//高8位
    spi1_send_recv_data(page_addr >> 8);//中8位
    spi1_send_recv_data(page_addr);//低8位
    //接收数据
    for(uint16_t i = 0; i < len; i++){
        data[i] = spi1_send_recv_data(0xFF);
    }
    SPI_FLASH_DESELECT;
}
void test_spi_flash(void){
    uint16_t id = spi_flash_read_id();
    printf("W25Q18F ID: 0x%04X\r\n", id);
    spi_flash_erase_4k_sector(0);
    printf("W25Q18F 4KB扇区擦除完成\r\n");
    page_buf[0] = 0x55;
    page_buf[1] = 0xAA;
    spi_flash_write_page(0, page_buf, sizeof(page_buf));
    printf("W25Q18F 页写入完成\r\n");
    spi_flash_random_read(0, page_cache, 2);
    printf("W25Q18F 页读取完成\r\n");
    for(int i = 0; i < 2; i++){
        printf("0x%02X ", page_cache[i]);
    }
    printf("\r\n");
}
//读写管理

//写完一页，更新当前写入页地址
void update_page_index(void){
    memset(page_buf, 0xFF, sizeof(page_buf));//清空页缓冲区,防止强制写入时带入脏数据
    current_page_byte_index = 0;//重置当前页字节索引
    current_write_page_index ++;//更新当前写入页地址
    if(current_write_page_index >= W25Q128_SECTOR_SIZE / W25Q128_PAGE_SIZE){//扇区满，写到下一个扇区
        current_write_page_index = 0;//重置当前写入页地址
        current_write_sector_index ++;//更新当前写入扇区地址
    }
    if(current_write_sector_index >= W25Q128_SECTOR_COUNT){//扇区满，重置所有
        current_page_byte_index = 0;//重置当前页字节索引
        current_write_page_index = 0;//重置当前写入页地址
        current_write_sector_index = 0;//重置当前写入扇区地址
    }
}
//先写到页缓冲区，再写入flash
void spi_flash_write_page_buf(uint8_t *data, uint16_t len){
    // 写入W25Q18F的页缓冲区
    uint16_t write_index = 0;
    //写入数据到当前页缓冲区
    for(write_index = 0; write_index < len && current_page_byte_index < W25Q128_PAGE_SIZE; write_index ++){
        page_buf[current_page_byte_index ++] = data[write_index];
    }
    // printf("W25Q18F 写入buf%d字节，当前页地址：%02d，当前扇区地址：%02d，字节索引：%02d\r\n",
    //          write_index, current_write_page_index, current_write_sector_index, current_page_byte_index);
    //当前页满，写入flash
    if(current_page_byte_index >= W25Q128_PAGE_SIZE){//当前页满，写入flash缓冲区
        
        if(current_write_page_index == 0){//第k扇区的第0个页，要写入新扇区，需要擦除扇区
            spi_flash_erase_4k_sector(current_write_sector_index * W25Q128_SECTOR_SIZE);
        }
        spi_flash_write_page(current_write_sector_index * W25Q128_SECTOR_SIZE + current_write_page_index * W25Q128_PAGE_SIZE,
            page_buf, sizeof(page_buf));//写入当前页
        update_page_index();//更新当前写入页地址
        // printf("W25Q18F 页满触发自动写入，当前页地址：%02d，当前扇区地址：%02d，字节索引：%02d\r\n",
        //      current_write_page_index, current_write_sector_index, current_page_byte_index);
    }
    if(write_index < len){//还有数据未写入
        spi_flash_write_page_buf(data + write_index, len - write_index);
        // printf("W25Q18F 页满触发递归写入buf，当前页地址：%02d，当前扇区地址：%02d，字节索引：%02d\r\n",
        //      current_write_page_index, current_write_sector_index, current_page_byte_index);
    }
}
//强制写入当前页缓冲区数据到flash
void spi_flash_force_write(void){
    // 写入当前页缓冲区数据到flash
    if(current_write_page_index == 0){//当前扇区是新扇区，需要擦除扇区
        spi_flash_erase_4k_sector(current_write_sector_index * W25Q128_SECTOR_SIZE);
    }
    spi_flash_write_page(current_write_sector_index * W25Q128_SECTOR_SIZE + current_write_page_index * W25Q128_PAGE_SIZE,
        page_buf, sizeof(page_buf));//写入当前页
    // printf("W25Q18F 强制写入当前页，当前页地址：%02d，当前扇区地址：%02d，字节索引：%02d\r\n",
    //          current_write_page_index, current_write_sector_index, current_page_byte_index);
}

void spi_flash_record_TH(void){
    // 记录温度和湿度数据, 格式：[2026-05-13 12:00:00 25 40]
    //获取当前时间日期
    static uint8_t record_buf[10] = {0};
    RTC_DateTypeDef RTC_DateStructure;
    RTC_GetDate (RTC_Format_BCD, &RTC_DateStructure);
    RTC_TimeTypeDef RTC_TimeStructure;
    RTC_GetTime (RTC_Format_BCD, &RTC_TimeStructure);
    //获取当前温度和湿度
    temperature_humidity_sensor_measure();//测量温度和湿度
    //填充记录缓冲区
    memset(record_buf, 0, sizeof(record_buf));
    record_buf[0] = RTC_DateStructure.RTC_Year;
    record_buf[1] = RTC_DateStructure.RTC_Month;
    record_buf[2] = RTC_DateStructure.RTC_Date;
    record_buf[3] = RTC_TimeStructure.RTC_Hours;
    record_buf[4] = RTC_TimeStructure.RTC_Minutes;
    record_buf[5] = RTC_TimeStructure.RTC_Seconds;
    record_buf[6] = current_TH_data.temperature;
    record_buf[7] = current_TH_data.humidity;
    record_buf[8] = 0x00;   //2字节的结尾标志
    record_buf[9] = 0x00;
    //写入此条记录到页缓冲区
    spi_flash_write_page_buf(record_buf, sizeof(record_buf));
    printf("W25Q18F 温湿度记录完成 \r\n");
}
//查询温度和湿度记录
void spi_flash_record_TH_qury(void){
    uint8_t record_cache[10] = {0};
    uint32_t read_addr = 0;
    while(1){
        memset(record_cache, 0xFF, sizeof(record_cache));
        spi_flash_random_read(read_addr, record_cache, sizeof(record_cache));
        read_addr += sizeof(record_cache);
        if(record_cache[8] == 0x00 && record_cache[9] == 0x00){//查询到数据
            printf("W25Q18F 温湿度记录: %04hhx-%02hhx-%02hhx %02hhx:%02hhx:%02hhx %02d℃ %02d%%\r\n",
                record_cache[0], record_cache[1], record_cache[2],
                record_cache[3], record_cache[4], record_cache[5],
                record_cache[6], record_cache[7]);
        }else{//结尾标志，查询完成
            break;
        }
    }
}

