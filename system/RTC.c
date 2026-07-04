#include "RTC.h"
#include "stm32f4xx.h"
#include "stdio.h"
#include "DFA_event_queue.h"
#include "event_handlers.h"

#define RTC_INIT_KEY 0x000115211
#define ALARM_CYCLE 5 // 闹钟周期，单位：次

//用于修改日期时间和闹钟时间
RTC_DateTypeDef submit_date = {.RTC_Year = 0x26, .RTC_Month = 0x05, .RTC_Date = 0x08, .RTC_WeekDay = 0x05};
RTC_TimeTypeDef submit_time = {.RTC_Hours = 0x16, .RTC_Minutes = 0x08, .RTC_Seconds = 0x00};
RTC_AlarmTypeDef submit_alarm = {.RTC_AlarmTime = {.RTC_H12 = RTC_H12_PM, .RTC_Hours = 0x16, .RTC_Minutes = 0x09,
     .RTC_Seconds = 0x00}, 
     .RTC_AlarmMask = RTC_AlarmMask_DateWeekDay //每天都触发
};

void rtc_init(void){
    //1.开启PWR时钟
    RCC_APB1PeriphClockCmd (RCC_APB1Periph_PWR, ENABLE);
    //关闭PWR的RTC寄存器写保护
    PWR_BackupAccessCmd (ENABLE);

    if(RTC_ReadBackupRegister(RTC_BKP_DR0) == RTC_INIT_KEY){
        return;
    }
    // 写入秘钥,等待继续
    RTC_WaitForSynchro();
    // 写入初始化键
    RTC_WriteBackupRegister(RTC_BKP_DR0, RTC_INIT_KEY);
    //2.使能LSE,将其设置为RTC的时钟源
    RCC_LSEConfig(RCC_LSE_ON);
    while(RCC_GetFlagStatus (RCC_FLAG_LSERDY) != SET);//等待就绪
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);//选择LSE为RTC的时钟源
    //3.使能RTC的写访问
    RCC_RTCCLKCmd (ENABLE);
    RTC_WaitForSynchro();//写入秘钥,等待继续
    // 初始化RTC
    RTC_InitTypeDef RTC_InitStructure;
    //4.初始化RTC分频系数,产生1Hz时钟
    RTC_InitStructure. RTC_HourFormat = RTC_HourFormat_24;//24小时制
    RTC_InitStructure.RTC_AsynchPrediv=0x7f;//异步128分频
    RTC_InitStructure.RTC_SynchPrediv= 0xff;//同步256分频
    RTC_Init(&RTC_InitStructure);

    //5. 设置日期和时间
    rtc_set_time(RTC_H12_PM, 0x16, 0x08, 0x00);//下午4:08:00
    rtc_set_date(0x08, 0x08, 0x08, 0x06);//2026年5月8日星期五
}
//初始化RTC闹钟
void rtc_alarm_init(){
    /*允许RTC的A闹钟触发中断 */
    RTC_ITConfig(RTC_IT_ALRA, ENABLE);
    /* 清空标志位 */
    RTC_ClearFlag(RTC_FLAG_ALRAF);
    //使能外部中断控制线17的中断， exit17引脚连接到RTC的alarm引脚
    EXTI_ClearITPendingBit(EXTI_Line17);
    EXTI_InitTypeDef EXTI_InitStructure;
    EXTI_InitStructure. EXTI_Line = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;

    EXTI_Init(&EXTI_InitStructure);
    NVIC_InitTypeDef NVIC_InitStructure;
    /*使能闹钟的中断 */
    NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
void RTC_Alarm_IRQHandler(void){
    if(RTC_GetFlagStatus(RTC_FLAG_ALRAF) == SET){
        // 处理闹钟中断
        // 触发事件
        event_push_interface(alarm_handler, ALARM_CYCLE);
        // 清空标志位
        // printf("RTC_Alarm_IRQHandler\r\n");
        RTC_ClearFlag(RTC_FLAG_ALRAF);
        EXTI_ClearITPendingBit(EXTI_Line17);
    }
}

int8_t rtc_alarm_set(RTC_AlarmTypeDef RTC_AlarmStructure){
    ErrorStatus err;
    err = RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
    if(err != SUCCESS){
        return -1;
    }
    RTC_SetAlarm (RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);
    err = RTC_AlarmCmd(RTC_Alarm_A, ENABLE);
    if(err != SUCCESS){
        return -2;
    }
    return 1;
}
void rtc_alarm_disable(void){
    RTC_AlarmCmd(RTC_Alarm_A, DISABLE);
}



int8_t rtc_set_date(uint8_t year, uint8_t month, uint8_t day, uint8_t week){
    RTC_DateTypeDef RTC_DateStructure;
    RTC_DateStructure. RTC_Year = year;//2023
    RTC_DateStructure. RTC_Month = month;//12月
    RTC_DateStructure. RTC_Date = day;//25日
    RTC_DateStructure. RTC_WeekDay = week;//星期五
    return RTC_SetDate (RTC_Format_BCD, &RTC_DateStructure);
}

int8_t rtc_set_time(uint8_t RTC_H12, uint8_t hour, uint8_t minute, uint8_t second){
    RTC_TimeTypeDef RTC_TimeStructure;
    RTC_TimeStructure. RTC_H12 = RTC_H12;//下午
    RTC_TimeStructure. RTC_Hours = hour;//13:45:00
    RTC_TimeStructure. RTC_Minutes = minute;//45
    RTC_TimeStructure. RTC_Seconds = second;//00
    return RTC_SetTime (RTC_Format_BCD, &RTC_TimeStructure);
}
uint8_t rtc_get_RTC_H12(void){
    RTC_TimeTypeDef RTC_TimeStructure;
    RTC_GetTime (RTC_Format_BCD, &RTC_TimeStructure);
    return RTC_TimeStructure.RTC_H12;
}
uint8_t rtc_get_year(void){
    RTC_DateTypeDef RTC_DateStructure;
    RTC_GetDate (RTC_Format_BCD, &RTC_DateStructure);
    return RTC_DateStructure.RTC_Year;
}


uint8_t rtc_get_month(void){
    RTC_DateTypeDef RTC_DateStructure;
    RTC_GetDate (RTC_Format_BCD, &RTC_DateStructure);
    return RTC_DateStructure.RTC_Month;
}
uint8_t rtc_get_day(void){
    RTC_DateTypeDef RTC_DateStructure;
    RTC_GetDate (RTC_Format_BCD, &RTC_DateStructure);
    return RTC_DateStructure.RTC_Date;
}
uint8_t rtc_get_week(void){
    RTC_DateTypeDef RTC_DateStructure;
    RTC_GetDate (RTC_Format_BCD, &RTC_DateStructure);
    return RTC_DateStructure.RTC_WeekDay;
}

uint8_t rtc_get_hour(void){
    RTC_TimeTypeDef RTC_TimeStructure;
    RTC_GetTime (RTC_Format_BCD, &RTC_TimeStructure);
    return RTC_TimeStructure.RTC_Hours;
}

uint8_t rtc_get_minute(void){
    RTC_TimeTypeDef RTC_TimeStructure;
    RTC_GetTime (RTC_Format_BCD, &RTC_TimeStructure);
    return RTC_TimeStructure.RTC_Minutes;
}

uint8_t rtc_get_second(void){
    RTC_TimeTypeDef RTC_TimeStructure;
    RTC_GetTime (RTC_Format_BCD, &RTC_TimeStructure);
    return RTC_TimeStructure.RTC_Seconds;
}

void rtc_print_current_date_time(void){
    RTC_DateTypeDef RTC_DateStructure;
    RTC_GetDate (RTC_Format_BCD, &RTC_DateStructure);
    RTC_TimeTypeDef RTC_TimeStructure;
    RTC_GetTime (RTC_Format_BCD, &RTC_TimeStructure);
    printf("RTC date_time: %hhx-%hhx-%hhx %hhx:%hhx:%hhx\r\n",
           RTC_DateStructure.RTC_Year, RTC_DateStructure.RTC_Month, RTC_DateStructure.RTC_Date,
           RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds);
}

//将2位十进制数转换为BCD码
uint8_t dec_to_bcd(uint8_t decval){
    return ((decval / 10) << 4) | (decval % 10);
}



