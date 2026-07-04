#include"stm32f4xx.h"
#include"key_interrupt.h"
#include"led.h"
#include"tools.h"
#include"mq_2.h"
#include "systick.h"
#include "timer.h"
#include "beep.h"
#include "pwm.h"
#include "motor.h"
#include "uart.h"
#include "retarget.h"
#include "CO2_sensor.h"
#include "DX_BT24.h"
#include "DFA_event_queue.h"
#include "esp8266.h"
#include "USART_config.h"
#include "esp8266_mqtt.h"
#include "RS485.h"
#include "ultra_sound.h"
#include "radar.h"
#include "temperature_humidity_sensor.h"
#include "IWDG.h"
#include "RTC.h"
#include "Flash.h"
#include "eeprom.h"
#include "oled_iic.h"
#include "spi_flash.h"
#include "SPI.h"
#include "RC522.h"
#include "rc522_app.h"
#include "SPI_LCD.h"



void init(void){
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    // 初始化Trie树
	led_gpio_init();
	key_gpio_init();
    key_interrupt_init();
    // mq2_gpio_init();
    systick_init();
    // timer_init();
    beep_gpio_init();
    // timer14_pwm_init();// led1pwm
    // timer1_pwm_init();//led4pwm
    // motor1_speed_regulation_init();
    usart1_init(115200);

    // if(USART2_TO_MODULE == USART2_CO2_SENSOR){
    //     CO2_sensor_init(9600);
    // }
    // else{
    //     RS485_init();
    // }
    // if(USART3_TO_MODULE == USART3_DX_BT24){
    //     dx_bt24_init(9600);
    // }
    // else if(USART3_TO_MODULE == USART3_ESP8266){
    //     // esp8266_usart_init(115200);
    // }
    //初始化ESP8266 MQTT
    // while(esp8266_mqtt_init()){
	// 	printf("esp8266_mqtt_init ...\r\n");
	// 	delay_ms(500);
	// }
    // printf("MQTT connect to broker success\r\n");
    // ultra_sound_init();
    temperature_humidity_sensor_init();
    // radar_init();
    // beep_pwm_init();
    // IWDG_Init(125);//初始化独立看门狗,超时时间为1秒
    rtc_init();//初始化RTC
    rtc_alarm_init();
    Flash_Init();
    ADC_common_init();
    impedance_ADC_init();
    light_sensor_ADC_init();
    trie_init();
}


int main(){
    SCB->VTOR = 0x8020000;//设置VTOR为Flash的基地址

    init();
    char str[] = "Hello World!\n";
    // tackle_mqtt_topic_msg_and_hearting();  // 检查MQTT主题消息并处理心跳包
    while(1){
        // radar_distance_show();
        // radar_distance_beeping();
        if(KEY1_State == 1){
            // oled_demo();
            while(usart1_send_byte('X') != 1);
            while(usart1_send_string(str) != 1);
            printf("KEY1 Pressed\r\n");
            flash_test();
            adc_test();
            clear_key_state();//清除按键状态
        }
        if(KEY2_State == 1){
            if(LED2_state = ~LED2_state){
                LED2_OFF;
            }
            else{
                LED2_ON;
            }
            light_sensor_regular_light();
            
            // IWDG_CheckResetFlag();//检查独立看门狗是否复位了CPU
            rtc_print_current_date_time();//打印当前日期和时间
            // spi_flash_record_TH_qury();
            // RC522_Main_app(0);//验证模式
            
            
            clear_key_state();//清除按键状态
        }
        if(KEY3_State == 1){
            int32_t ret = temperature_humidity_sensor_measure();
            if(ret == 0){
                printf("temperature_humidity_sensor_measure success, temperature:%d, humidity:%d, checksum:%d\n",
                     current_TH_data.temperature, current_TH_data.humidity, current_TH_data.checksum);
            }
            else{
                printf("temperature_humidity_sensor_measure failed, ret:%d\n", ret);
            }
            clear_key_state();//清除按键状态
        }
        if(KEY4_State == 1){
            spi_flash_force_write();
            int8_t ret = temperature_humidity_sensor_check();
            if(ret == 0){
                printf("temperature_humidity_sensor_check success\n");
            }
            else{
                printf("temperature_humidity_sensor_check failed\n");
            }

            clear_key_state();//清除按键状态
        }
        //根据CO2浓度设置通风速度比例
        run_event_task();//运行事件队列中的事件
        // IWDG_Reload();//喂狗
    }
}


