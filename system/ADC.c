#include "ADC.h"
#include "LED.h"
#include "stdio.h"
#include "stm32f4xx.h"

/*ADCommon,只能初始化一次,不能重复初始化,否则会导致错误，
 * ADC_DeInit()只能在common初始化前调用,否则会导致错误
 */
void ADC_common_init(void) {
    ADC_DeInit();
    ADC_CommonInitTypeDef ADC_Common_InitStructer;
    ADC_Common_InitStructer.ADC_Mode = ADC_Mode_Independent;
    ADC_Common_InitStructer.ADC_Prescaler = ADC_Prescaler_Div4;  // 21MHz
    ADC_Common_InitStructer.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_Common_InitStructer.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;  // 采样间隔
    ADC_Common_InitStructer.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInit(&ADC_Common_InitStructer);
}
void impedance_ADC_init(void) {
    // 初始化ADC1时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    // pa5,adc1in5
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    // 初始化ADC1引脚
    GPIO_InitTypeDef GPIO_InitStructer;
    GPIO_InitStructer.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructer.GPIO_Mode = GPIO_Mode_AN;
    // GPIO_InitStructer.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructer.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructer);

    // ADC_DeInit();

    // 初始化ADC1通道
    ADC_InitTypeDef ADC_InitStructer;
    ADC_InitStructer.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructer.ADC_ScanConvMode = DISABLE;                                // 多通道扫描
    ADC_InitStructer.ADC_ContinuousConvMode = DISABLE;                          // 关闭连续转换
    ADC_InitStructer.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;  // 关闭外部触发源
    ADC_InitStructer.ADC_DataAlign = ADC_DataAlign_Right;                       // 右对齐
    ADC_InitStructer.ADC_NbrOfConversion = 1;                                   // 转换次数为1
    // ADC_InitStructer.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;    //外部触发源为T1_CC1
    ADC_Init(ADC1, &ADC_InitStructer);
    ADC_Cmd(ADC1, ENABLE);
    // adc1通道5,采样时间3个周期,转换次数为1,右对齐
    ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_3Cycles);
}
void light_sensor_ADC_init(void) {
    // 初始化ADC1时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
    // pf7,adc3 in5
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    // 初始化ADC3引脚
    GPIO_InitTypeDef GPIO_InitStructer;
    GPIO_InitStructer.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructer.GPIO_Mode = GPIO_Mode_AN;
    // GPIO_InitStructer.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructer.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStructer);

    // 初始化ADC1通道
    ADC_InitTypeDef ADC_InitStructer;
    ADC_InitStructer.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructer.ADC_ScanConvMode = DISABLE;                                // 多通道扫描
    ADC_InitStructer.ADC_ContinuousConvMode = DISABLE;                          // 关闭连续转换
    ADC_InitStructer.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;  // 关闭外部触发源
    ADC_InitStructer.ADC_DataAlign = ADC_DataAlign_Right;                       // 右对齐
    ADC_InitStructer.ADC_NbrOfConversion = 1;                                   // 转换次数为1
    // ADC_InitStructer.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;    //外部触发源为T1_CC1
    ADC_Init(ADC3, &ADC_InitStructer);
    ADC_Cmd(ADC3, ENABLE);
    // adc3通道5,采样时间3个周期,转换次数为1,右对齐
    ADC_RegularChannelConfig(ADC3, ADC_Channel_5, 1, ADC_SampleTime_3Cycles);
}

uint16_t ADC_read(ADC_TypeDef* adc_x) {
    ADC_SoftwareStartConv(adc_x);
    while (ADC_GetFlagStatus(adc_x, ADC_FLAG_EOC) == RESET)
        ;
    return ADC_GetConversionValue(adc_x);
}

void adc_test(void) {
    // impedance_ADC_init();
    uint16_t adc_value = ADC_read(ADC1);
    // 打印adc_value
    printf("adc_value = %d, adc_voltage_impedance = %0.2f （mV） \r\n", adc_value,
           (float)adc_value * (3300 / 4096.0));
    // light_sensor_ADC_init();
    uint16_t light_adc_value = ADC_read(ADC3);
    // 打印adc_value
    printf("adc_value = %d, adc_voltage_light = %0.2f （mV） \r\n", light_adc_value,
           (float)light_adc_value * (3300 / 4096.0));
}

void light_sensor_regular_light(void) {
    while (1) {
        uint16_t light_adc_value = ADC_read(ADC3);
        // 打印adc_value
        printf("light_adc_value = %d, adc_voltage_light = %0.2f （mV） \r\n", light_adc_value,
               (float)light_adc_value * (3300 / 4096.0));
        //光线充足
        if(light_adc_value > 0 && light_adc_value < 500){
            LED_SWITCH(1, OFF);
            LED_SWITCH(2, OFF);
            LED_SWITCH(3, OFF);
            LED_SWITCH(4, OFF);
        }
        if (light_adc_value > 500 && light_adc_value < 1000) {
            LED_SWITCH(1, ON);
            LED_SWITCH(2, OFF);
            LED_SWITCH(3, OFF);
            LED_SWITCH(4, OFF);
        } else if (light_adc_value > 1000 && light_adc_value < 2000) {
            LED_SWITCH(1, ON);
            LED_SWITCH(2, ON);
            LED_SWITCH(3, OFF);
            LED_SWITCH(4, OFF);
        } else if (light_adc_value > 2000 && light_adc_value < 2900) {
            LED_SWITCH(1, ON);
            LED_SWITCH(2, ON);
            LED_SWITCH(3, ON);
            LED_SWITCH(4, OFF);
        } else if (light_adc_value > 2900) {
            LED_SWITCH(1, ON);
            LED_SWITCH(2, ON);
            LED_SWITCH(3, ON);
            LED_SWITCH(4, ON);
        }
    }
}
