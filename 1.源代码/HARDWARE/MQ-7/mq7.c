#include "mq7.h"

/*****************辰哥单片机设计******************
											STM32
 * 文件			:	MQ-7一氧化碳传感器c文件                   
 * 版本			: V1.0
 * 日期			: 2024.8.22
 * MCU			:	STM32F103C8T6
 * 接口			:	见代码							
 * BILIBILI	:	辰哥单片机设计
 * CSDN			:	辰哥单片机设计
 * 作者			:	辰哥

**********************BEGIN***********************/

void MQ7_Init(void)
{
	#if MODE
	{
		GPIO_InitTypeDef GPIO_InitStructure;
		
		RCC_APB2PeriphClockCmd (MQ7_AO_GPIO_CLK, ENABLE );	// 打开 ADC IO端口时钟
		GPIO_InitStructure.GPIO_Pin = MQ7_AO_GPIO_PIN;					// 配置 ADC IO 引脚模式
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		// 设置为模拟输入
		
		GPIO_Init(MQ7_AO_GPIO_PORT, &GPIO_InitStructure);				// 初始化 ADC IO

		ADCx_Init();
	}
	#else
	{
		GPIO_InitTypeDef GPIO_InitStructure;
		
		RCC_APB2PeriphClockCmd (MQ7_DO_GPIO_CLK, ENABLE );	// 打开连接 传感器DO 的单片机引脚端口时钟
		GPIO_InitStructure.GPIO_Pin = MQ7_DO_GPIO_PIN;			// 配置连接 传感器DO 的单片机引脚模式
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			// 设置为上拉输入
		
		GPIO_Init(MQ7_DO_GPIO_PORT, &GPIO_InitStructure);				// 初始化 
		
	}
	#endif
	
}

#if MODE
uint16_t MQ7_ADC_Read(void)
{
	//设置指定ADC的规则组通道，采样时间
	return ADC_GetValue(ADC_CHANNEL, ADC_SampleTime_55Cycles5);
}
#endif

uint16_t MQ7_GetData(void)
{
	
	#if MODE
	uint32_t  tempData = 0;
	for (uint8_t i = 0; i < MQ7_READ_TIMES; i++)
	{
		tempData += MQ7_ADC_Read();
		delay_ms(5);
	}

	tempData /= MQ7_READ_TIMES;
	return tempData;
	
	#else
	uint16_t tempData;
	tempData = !GPIO_ReadInputDataBit(MQ7_DO_GPIO_PORT, MQ7_DO_GPIO_PIN);
	return tempData;
	#endif
}


float MQ7_GetData_PPM(void)
{
	#if MODE
	float  tempData = 0;
	

	for (uint8_t i = 0; i < MQ7_READ_TIMES; i++)
	{
		tempData += MQ7_ADC_Read();
		delay_ms(5);
	}
	tempData /= MQ7_READ_TIMES;
	
	float Vol = (tempData*5/4096);
	float RS = (5-Vol)/(Vol*0.5);
	float R0=6.64;
	
	float ppm = pow(11.5428*R0/RS, 0.6549f);
	
	return ppm;
	#endif
}

