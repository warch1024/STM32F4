#include "ultra_sound.h"
#include "stm32f4xx.h"
#include "systick.h"

void ultra_sound_init(void){
    // 初始化超声波传感器GPIO引脚
    GPIO_InitTypeDef GPIO_ultra_sound_init;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    GPIO_ultra_sound_init.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_ultra_sound_init.GPIO_OType = GPIO_OType_PP;
    GPIO_ultra_sound_init.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_ultra_sound_init.GPIO_PuPd = GPIO_PuPd_NOPULL; //默认下拉
    
    GPIO_ultra_sound_init.GPIO_Pin = GPIO_Pin_6; //TRIG=E6
    GPIO_Init(GPIOE, &GPIO_ultra_sound_init);
    GPIO_ultra_sound_init.GPIO_Pin = GPIO_Pin_8; //Echo=A8
    GPIO_ultra_sound_init.GPIO_Mode = GPIO_Mode_IN;
    GPIO_Init(GPIOA, &GPIO_ultra_sound_init);
}

int32_t ultra_sound_measure(void){
    // 测量超声波传感器距离
    uint32_t distance = 0;
    uint32_t time = 0;
    // 触发超声波传感器
    ULTRA_SOUND_TRIGGER_SWITCH(TRIGGER);
    delay_us(20);//延时得大于10us，否则会导致超声波传感器触发失败
    ULTRA_SOUND_TRIGGER_SWITCH(TRIGGER_CLEAR);
    // delay_us(10);
    // 等待超声波传感器返回电平
    while(ULTRA_SOUND_ECHO_STATUS == Bit_RESET){
        time++;
        delay_us(1);
        if(time > 1000000){
            return -1;
        }
    }
    // printf("time = %d\r\n", time);
    time = 0;
    while(ULTRA_SOUND_ECHO_STATUS){
        delay_us(9);    //9us是3mm距离
        time++;
        if(time > 1000000){
            return -2;
        }
    }
    // printf("time2 = %d\r\n", time);
    // 计算超声波传感器距离
    distance = (time * 3) / 2;
    // 返回超声波传感器距离
    return distance;
}


//保存举例数据
void ultra_sound_save_to_flash(uint32_t distance){
    // 保存举例数据
    // 例如：将距离保存到EEPROM
    // ...
}
