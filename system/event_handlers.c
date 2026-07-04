#include "event_handlers.h"
#include "led.h"
#include "beep.h"
#include "motor.h"
#include "CO2_sensor.h"
#include "uart.h"
#include "stdio.h"
#include "esp8266.h"
#include "rtc.h"
#include "systick.h"

void default_handler(int param_val){
    // ... 你可以自己加
    ERR_MSG("Default Handler\r\n");
}
void led_on(int param_val){
    // ... 你可以自己加
    switch(param_val){
        case 1: LED_SWITCH(1, ON); break;
        case 2: LED_SWITCH(2, ON); break;
        case 3: LED_SWITCH(3, ON); break;
        case 4: LED_SWITCH(4, ON); break;
        default: break;
    }
}
void led_off(int param_val){
    // ... 你可以自己加
    switch(param_val){
        case 1: LED_SWITCH(1, OFF); break;
        case 2: LED_SWITCH(2, OFF); break;
        case 3: LED_SWITCH(3, OFF); break;
        case 4: LED_SWITCH(4, OFF); break;
        default: break;
    }
}
void beep_on(int param_val){
    // ... 你可以自己加
    switch(param_val){
        case 1: BEEP_SWITCH(1, ON); break;
        // case 2: BEEP_SWITCH(2, ON); break;
        // case 3: BEEP_SWITCH(3, ON); break;
        // case 4: BEEP_SWITCH(4, ON); break;
        default: break;
    }
}
void beep_off(int param_val){
    // ... 你可以自己加
    switch(param_val){
        case 1: BEEP_SWITCH(1, OFF); break;
        // case 2: BEEP_SWITCH(2, OFF); break;
        // case 3: BEEP_SWITCH(3, OFF); break;
        // case 4: BEEP_SWITCH(4, OFF); break;
        default: break;
    }
}

void fan_on(int param_val){
    // ... 你可以自己加
    switch(param_val){
        case 1: MOTOR_SWITCH(1, ON); break;
        default: break;
    }
}
void fan_off(int param_val){
    // ... 你可以自己加
    switch(param_val){
        case 1: MOTOR_SWITCH(1, OFF); break;
        default: break;
    }
}
void fan_speed(int param_val){
    // ... 你可以自己加
    MOTOR1_SPEED_PERCENT(param_val);

}
void co2_auto(int param_val){
    // ... 你可以自己加
    CO2_sensor_control_mode = CO2_SENSOR_AUTO_CONTROL;
    ERR_MSG("CO2 Sensor Auto Control\r\n");
}
void co2_off(int param_val){
    // ... 你可以自己加
    CO2_sensor_control_mode = CO2_SENSOR_OFF_CONTROL;
    ERR_MSG("CO2 Sensor Off Control\r\n");
}

void co2_get_value(int param_val){
    // ... 你可以自己加
    if(CO2_sensor_control_mode == CO2_SENSOR_AUTO_CONTROL){
        usart1_send_string("CO2 Value: ");
        printf("%d", CO2_density);
        usart1_send_string("\r\n");
    }
    else{
        usart1_send_string("CO2 sensor was off");
        usart1_send_string("\r\n");
    }
}
void esp8266_net_init_task(int param_val){
    // ... 你可以自己加

}


void submit_time_hour(int param_val){
    // ... 你可以自己加
    submit_time.RTC_Hours = dec_to_bcd(param_val);
    if(param_val > 12){
        submit_time.RTC_H12 = RTC_H12_PM;
    }
    else{
        submit_time.RTC_H12 = RTC_H12_AM;
    }
    printf("submit_time.RTC_Hours: %hhx\r\n", submit_time.RTC_Hours);
}
void submit_time_min(int param_val){
    // ... 你可以自己加
    submit_time.RTC_Minutes = dec_to_bcd(param_val);
    printf("submit_time.RTC_Minutes: %hhx\r\n", submit_time.RTC_Minutes);
}
void submit_time_sec(int param_val){
    // ... 你可以自己加
    submit_time.RTC_Seconds = dec_to_bcd(param_val);
    printf("submit_time.RTC_Seconds: %hhx\r\n", submit_time.RTC_Seconds);
}
    void submit_date_year(int param_val){
    // ... 你可以自己加
    submit_date.RTC_Year = dec_to_bcd(param_val);
    printf("submit_date.RTC_Year: %hhx\r\n", submit_date.RTC_Year);
}
void submit_date_month(int param_val){
    // ... 你可以自己加
    submit_date.RTC_Month = dec_to_bcd(param_val);
    printf("submit_date.RTC_Month: %hhx\r\n", submit_date.RTC_Month);
}
void submit_date_day(int param_val){
    // ... 你可以自己加
    submit_date.RTC_Date = dec_to_bcd(param_val);
    printf("submit_date.RTC_Date: %hhx\r\n", submit_date.RTC_Date);
}
void submit_date_week(int param_val){
    // ... 你可以自己加
    submit_date.RTC_WeekDay = dec_to_bcd(param_val);
    printf("submit_date.RTC_WeekDay: %hhx\r\n", submit_date.RTC_WeekDay);
}
void set_time(int param_val){
    // ... 你可以自己加
    //先解锁RTC
    PWR_BackupAccessCmd (ENABLE);
    //3.使能RTC的写访问
    RCC_RTCCLKCmd (ENABLE);
    //写入秘钥,等待继续
    RTC_WaitForSynchro();
    int8_t ret = rtc_set_time(submit_time.RTC_H12, submit_time.RTC_Hours, submit_time.RTC_Minutes, submit_time.RTC_Seconds);
    if(ret == 1){
        printf("set_time success\r\n");
    }
    else{
        printf("set_time failed, ret:%d\r\n", ret);
    }
}
void set_date(int param_val){
    // ... 你可以自己加
    //先解锁RTC
    PWR_BackupAccessCmd (ENABLE);
    //3.使能RTC的写访问
    RCC_RTCCLKCmd (ENABLE);
    //写入秘钥,等待继续
    RTC_WaitForSynchro();
    //5.设置日期
    int8_t ret = rtc_set_date(submit_date.RTC_Year, submit_date.RTC_Month, submit_date.RTC_Date, submit_date.RTC_WeekDay);
    if(ret == 1){
        printf("set_date success\r\n");
    }
    else{
        printf("set_date failed, ret:%d\r\n", ret);
    }
}
//闹钟相关
void submit_alarm_hour(int param_val){
    // ... 你可以自己加
    if(param_val >12){
        submit_alarm.RTC_AlarmTime.RTC_H12 = RTC_H12_PM;
    }
    else{
        submit_alarm.RTC_AlarmTime.RTC_H12 = RTC_H12_AM;
    }
    submit_alarm.RTC_AlarmTime.RTC_Hours = dec_to_bcd(param_val);
    printf("submit_alarm.RTC_AlarmTime.RTC_Hours: %hhx\r\n", submit_alarm.RTC_AlarmTime.RTC_Hours);
}

void submit_alarm_min(int param_val){
    // ... 你可以自己加
    submit_alarm.RTC_AlarmTime.RTC_Minutes = dec_to_bcd(param_val);
    printf("submit_alarm.RTC_AlarmTime.RTC_Minutes: %hhx\r\n", submit_alarm.RTC_AlarmTime.RTC_Minutes);
}
void submit_alarm_sec(int param_val){
    // ... 你可以自己加
    submit_alarm.RTC_AlarmTime.RTC_Seconds = dec_to_bcd(param_val);
    printf("submit_alarm.RTC_AlarmTime.RTC_Seconds: %hhx\r\n", submit_alarm.RTC_AlarmTime.RTC_Seconds);
}   
void set_alarm(int param_val){
    // ... 你可以自己加
    //先解锁RTC
    PWR_BackupAccessCmd (ENABLE);
    //3.使能RTC的写访问
    RCC_RTCCLKCmd (ENABLE);
    //写入秘钥,等待继续
    RTC_WaitForSynchro();
    //5.设置闹钟
    int8_t ret = rtc_alarm_set(submit_alarm);
    if(ret == 1){
        printf("set_alarm success:  %hhx:%hhx:%hhx\r\n", 
            submit_alarm.RTC_AlarmTime.RTC_Hours, submit_alarm.RTC_AlarmTime.RTC_Minutes, 
            submit_alarm.RTC_AlarmTime.RTC_Seconds);
           }
    else{
        printf("set_alarm failed, ret:%d\r\n", ret);
    }
}
//闹钟处理函数
void alarm_handler(int param_val){
    // ... 你可以自己加
    if(param_val < 0){
        param_val = 1;
    }
    while(param_val--){
        LED1_ON
        BEEP1_ON
        delay_ms(100);
        LED1_OFF
        BEEP1_OFF
        delay_ms(100);
        LED1_ON
        BEEP1_ON
        delay_ms(100);
        LED1_OFF
        BEEP1_OFF
        delay_ms(2000);
        delay_ms(2000);
    }
}





