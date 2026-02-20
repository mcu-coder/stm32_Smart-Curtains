#ifndef __MQ7_H
#define	__MQ7_H
#include "stm32f10x.h"
#include "adcx.h"
#include "delay.h"
#include "math.h"

/*****************辰哥单片机设计******************
											STM32
 * 文件			:	MQ-7一氧化碳传感器h文件                   
 * 版本			: V1.0
 * 日期			: 2024.8.22
 * MCU			:	STM32F103C8T6
 * 接口			:	见代码							
 * BILIBILI	:	辰哥单片机设计
 * CSDN			:	辰哥单片机设计
 * 作者			:	辰哥

**********************BEGIN***********************/

#define MQ7_READ_TIMES	10  //MQ-7传感器ADC循环读取次数

//模式选择	
//模拟AO:	1
//数字DO:	0
#define	MODE 	0

/***************根据自己需求更改****************/
// MQ-7 GPIO宏定义
#if MODE
#define		MQ7_AO_GPIO_CLK								RCC_APB2Periph_GPIOA
#define 	MQ7_AO_GPIO_PORT							GPIOA
#define 	MQ7_AO_GPIO_PIN								GPIO_Pin_0
#define   ADC_CHANNEL               		ADC_Channel_0	// ADC 通道宏定义

#else
#define		MQ7_DO_GPIO_CLK								RCC_APB2Periph_GPIOA
#define 	MQ7_DO_GPIO_PORT							GPIOA
#define 	MQ7_DO_GPIO_PIN								GPIO_Pin_1			

#endif
/*********************END**********************/


void MQ7_Init(void);
uint16_t MQ7_GetData(void);
float MQ7_GetData_PPM(void);

#endif /* __ADC_H */

