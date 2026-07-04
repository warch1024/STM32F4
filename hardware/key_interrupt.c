#include "key_interrupt.h"
#include "stm32f4xx.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "led.h"
#include "tools.h"
#include "systick.h"

uint8_t KEY1_State = 0;
uint8_t KEY2_State = 0;
uint8_t KEY3_State = 0;
uint8_t KEY4_State = 0;


void key_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//1.使能PA0和PE2 PE3 PE4时钟
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOE,ENABLE);
	
	//2.初始化GPIO PA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//PA0
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//无上下拉电阻
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;//PE2 PE3 PE4
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//无上下拉电阻
	GPIO_Init(GPIOE,&GPIO_InitStructure);
}
void key_interrupt_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//1.使能PA0和PE2 PE3 PE4时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOE,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);
	
	//2.初始化GPIO PA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;//PA0
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//无上下拉电阻
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//输入模式
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;//PE2 PE3 PE4
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//无上下拉电阻
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	//3.配置EXTI线 PA0 ------------ EXTI0
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA,EXTI_PinSource0);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource2);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource3);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource4);
	
	//4.初始化EXTI线
	EXTI_InitStructure.EXTI_Line = EXTI_Line0|EXTI_Line2|EXTI_Line3|EXTI_Line4;//EXTI_Line0 EXTI_Line2 EXTI_Line3 EXTI_Line4
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//下降沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;//使能
	EXTI_Init(&EXTI_InitStructure);
	
	//5.初始化NVIC通道
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;//EXTI0_IRQn
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0;//优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0;//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;//EXTI2_IRQn
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x1;//优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x1;//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;//EXTI3_IRQn
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x2;//优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x2;//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能
	NVIC_Init(&NVIC_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;//EXTI4_IRQn
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x2;//优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x2;//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能
	NVIC_Init(&NVIC_InitStructure);
}

//中断服务函数EXTI0_IRQHandler
void EXTI0_IRQHandler(void)
{
    // 处理按键中断
    if(EXTI_GetITStatus(EXTI_Line0) == SET){
        // 按键0按下
        KEY1_State = 1;
        KEY2_State = 0;
        KEY3_State = 0;
        KEY4_State = 0;
        delay_ms(150);
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}
void EXTI2_IRQHandler(void)
{
    // 处理按键中断
    if(EXTI_GetITStatus(EXTI_Line2) == SET){
        // 按键2按下
        KEY1_State = 0;
        KEY2_State = 1;
        KEY3_State = 0;
        KEY4_State = 0;
        delay_ms(150);
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}
void EXTI3_IRQHandler(void)
{
    // 处理按键中断
    if(EXTI_GetITStatus(EXTI_Line3) == SET){
        // 按键3按下
        KEY1_State = 0;
        KEY2_State = 0;
        KEY3_State = 1;
        KEY4_State = 0;
        delay_ms(150);
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}
void EXTI4_IRQHandler(void)
{
    // 处理按键中断
    if(EXTI_GetITStatus(EXTI_Line4) == SET){
        // 按键4按下
        KEY1_State = 0;
        KEY2_State = 0;
        KEY3_State = 0;
        KEY4_State = 1;
        delay_ms(150);
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}
void clear_key_state(void)//清除按键状态
{
    KEY1_State = 0;
    KEY2_State = 0;
    KEY3_State = 0;
    KEY4_State = 0;
}
