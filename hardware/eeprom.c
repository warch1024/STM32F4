#include "eeprom.h"
#include "soft_iic.h"
#include "stdio.h"

void eeprom_init(void){
    soft_iic_init();
}

uint8_t eeprom_write_byte(uint8_t iic_addr, uint8_t byte_addr, uint8_t byte){
    uint8_t ack = 3;
    soft_iic_start();
    soft_iic_send_addr(0x00, iic_addr);   //发送设备地址，写入模式
    ack = soft_iic_recv_ack();//接收应答
    if(ack != 0){
        return 4;
    }
    soft_iic_send_byte(byte_addr);   //发送字节地址
    ack = soft_iic_recv_ack();//接收应答
    if(ack != 0){
        return 5;
    }
    soft_iic_send_byte(byte);   //发送字节数据
    ack = soft_iic_recv_ack();//接收应答
    if(ack != 0){
        return 6;
    }
    soft_iic_stop();
    delay_ms(10);
    return ack;
}
int16_t eeprom_read_byte(uint8_t iic_addr, uint8_t byte_addr){//-1 = 未读取，0=读取失败，1=应答失败，2，3=其他错误，
    uint8_t ack = 3;
    int16_t byte = -1;
    soft_iic_start();
    soft_iic_send_addr(0x00, iic_addr);   //发送设备地址，写入模式
    ack = soft_iic_recv_ack();//接收应答
    if(ack != 0){
        return ack;
    }
    soft_iic_send_byte(byte_addr);   //发送字节地址
    ack = soft_iic_recv_ack();//接收应答
    if(ack != 0){
        return ack;
    }
    // soft_iic_idle();
    //重启读取模式
    soft_iic_start();
    soft_iic_send_addr(0x01, iic_addr);
    ack = soft_iic_recv_ack();
    if(ack != 0){
        return ack;
    }
    byte = soft_iic_recv_byte();
    soft_iic_nack();
    soft_iic_stop();
    return byte;
}

void eeprom_test(void){
    eeprom_init();
    uint8_t ack = eeprom_write_byte(EEPROM_ADDR, 0x00, 15);
    if(ack != 0){
        printf("eeprom_write_byte failed:%d\n", ack);
    }
    delay_ms(10);
    int16_t byte = eeprom_read_byte(EEPROM_ADDR, 0x00);
    if(byte != 15){
        printf("eeprom_read_byte failed, read: %d, expected: 15\n", byte);
    } else {
        printf("eeprom test passed, read: %d\n", byte);
    }
}

