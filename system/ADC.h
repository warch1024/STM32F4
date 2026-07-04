#ifndef __ADC_H__
#define __ADC_H__
#include "stm32f4xx.h"

void ADC_common_init(void);
void impedance_ADC_init(void);
void light_sensor_ADC_init(void);
uint16_t ADC_read(ADC_TypeDef * ADCx);

void adc_test(void);
void light_sensor_regular_light(void);

#endif


