#ifndef __RTC_H__
#define __RTC_H__
#include "stm32f4xx.h"

extern int32_t RTC_INIT_KEY;
extern RTC_DateTypeDef submit_date;
extern RTC_TimeTypeDef submit_time;
extern RTC_AlarmTypeDef submit_alarm;



void rtc_init(void);
void rtc_alarm_init(void);
int8_t rtc_alarm_set(RTC_AlarmTypeDef RTC_AlarmStructure);
void rtc_alarm_disable(void);

void rtc_print_current_date_time(void);
int8_t rtc_set_date(uint8_t year, uint8_t month, uint8_t day, uint8_t week);
int8_t rtc_set_time(uint8_t RTC_H12, uint8_t hour, uint8_t minute, uint8_t second);

uint8_t rtc_get_RTC_H12(void);
uint8_t rtc_get_year(void);
uint8_t rtc_get_month(void);
uint8_t rtc_get_day(void);
uint8_t rtc_get_week(void);
uint8_t rtc_get_hour(void);
uint8_t rtc_get_minute(void);
uint8_t rtc_get_second(void);
uint8_t dec_to_bcd(uint8_t decval);


#endif




