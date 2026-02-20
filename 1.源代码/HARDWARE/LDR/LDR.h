#ifndef __LDR_H
#define	__LDR_H
#include "stm32f10x.h"
#include "adcx.h"
#include "delay.h"
#include "math.h"



#define LDR_READ_TIMES	10  //光照传感器ADC循环读取次数

/***************根据自己需求更改****************/
// LDR GPIO宏定义
#define		LDR_GPIO_CLK							RCC_APB2Periph_GPIOA
#define 	LDR_GPIO_PORT							GPIOA
#define 	LDR_GPIO_PIN							GPIO_Pin_0
/*********************END**********************/


#define   ADC_CHANNEL_LDR               ADC_Channel_0

//#define    ADC_IRQ                       ADC3_IRQn
//#define    ADC_IRQHandler                ADC3_IRQHandler

void LDR_Init(void);
uint16_t LDR_Average_Data(void);
uint16_t LDR_LuxData(void);

#endif /* __ADC_H */

