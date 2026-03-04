#include "stm32f10x.h"
#include "led.h"
#include "beep.h"
#include "usart.h"
#include "delay.h"
#include "dht11.h"
#include "oled.h"
#include "key.h"
#include "Modules.h"
#include "TIM2.h"
#include "adcx.h"
#include "flash.h"
#include "stepmotor.h"
#include "MyRTC.h"
#include "usart2.h"
#include "usart3.h"
#include "gizwits_product.h"
#include "gizwits_protocol.h"
#include "TIM3.h"

 

#define KEY_Long1	11

#define KEY_1	1
#define KEY_2	2
#define KEY_3	3
#define KEY_4	4

#define FLASH_START_ADDR	0x0801f000	//写入的起始地址

SensorModules sensorData;								//声明传感器数据结构体变量
SensorThresholdValue Sensorthreshold;		//声明传感器阈值结构体变量
DriveModules driveData;									//声明驱动器状态结构体变量

uint8_t mode = 0;	//系统模式  1自动  2手动  3设置
u8 dakai;//串口3使用的传递变量
u8 Flag_dakai;//串口3接收标志位
uint8_t is_secondary_menu = 0;  // 0一级菜单，1二级菜单
uint8_t secondary_pos = 1;      // 二级菜单光标位置（1-3对应时/分/秒）
uint8_t secondary_type = 0;   // 二级菜单类型：0=RTC时间，1=定时开启，2=定时关闭

uint8_t send_data[] = "";
uint8_t send_data1[] = ""; 


//系统静态变量
static uint8_t count_a = 1;  //自动模式按键数
uint8_t count_m = 1;  //手动模式按键数
static uint8_t count_s = 1;	 //设置模式按键数

/**
  * @brief  显示菜单内容
  * @param  无
  * @retval 无
  */
enum 
{
	AUTO_MODE = 0,
	MANUAL_MODE,
	SETTINGS_MODE
	
}MODE_PAGES;

/**
  * @brief  显示菜单1的固定内容
  * @param  无
  * @retval 无
  */
void OLED_autoPage1(void)		//自动模式菜单第一页
{
	
	//显示”时间
	OLED_ShowChinese(0,0,53,16,1); 
	OLED_ShowChinese(16,0,54,16,1);
	OLED_ShowChar(32,0,':',16,1);
	
	//显示“温度：  C”
	OLED_ShowChinese(0,16,0,16,1);	//温
	OLED_ShowChinese(16,16,2,16,1);	//度
	OLED_ShowChar(32,16,':',16,1);
	
	 
	
	OLED_Refresh();
	
}
void OLED_autoPage2(void)   //自动模式菜单第二页
{
	
	//显示“定时开启”
	OLED_ShowChinese(0,0,57,16,1);
	OLED_ShowChinese(16,0,58,16,1);
	OLED_ShowChinese(32,0,40,16,1);
	OLED_ShowChinese(48,0,41,16,1);
	OLED_ShowChar(64,0,':',16,1);
	
	 
}



void SensorDataDisplay1(void)		//传感器数据显示第一页
{
    SensorScan();	//获取传感器数据
    char all_data[128];  // 足够大的缓冲区容纳所有数据
    const char* co_status = sensorData.CO ? "是" : "否";  // 确定一氧化碳状态
    
    // 一次性格式化所有数据到同一个字符串
    sprintf(all_data, " 温度: %d C\r\n 湿度: %d %%RH\r\n 光照: %d\r\n 一氧化碳浓度是否超标: %s\r\n",
            sensorData.temp,
            sensorData.humi,
            sensorData.lux,
            co_status);

    
    MyRTC_ReadTime();							//RTC读取时间
    OLED_ShowNum(40,0,MyRTC_Time[3],2,16,1);
    OLED_ShowChar(56,0,':',16,1);
    
  
    //一氧化碳数据
    if(sensorData.CO)
    {
        OLED_ShowChinese(72,48,50,16,1);	//异
        OLED_ShowChinese(88,48,52,16,1);	//常
    }
    else
    {
        OLED_ShowChinese(72,48,51,16,1);	//正
        OLED_ShowChinese(88,48,52,16,1);	//常
    }
}

void SensorDataDisplay2(void)		//传感器数据显示第二页
{
	//定时开启
	OLED_ShowNum(80,0,OPEN_HOUR,2,16,1);
	OLED_ShowChar(96,0,':',16,1);
	OLED_ShowNum(112,0,OPEN_MINUTE,2,16,1);
 

}

/**
  * @brief  显示手动模式设置界面1
  * @param  无
  * @retval 无
  */
void OLED_manualPage1(void)
{
	//显示“灯光”
	OLED_ShowChinese(16,0,28,16,1);	
	OLED_ShowChinese(32,0,29,16,1);	
	OLED_ShowChar(64,0,':',16,1);
 
	
}

/**
  * @brief  显示手动模式设置参数界面1
  * @param  无
  * @retval 无
  */
void ManualSettingsDisplay1(void)
{
	if(driveData.LED_Flag ==1)
	{
		OLED_ShowChinese(96,0,40,16,1); 	//开
	}
	else
	{
		OLED_ShowChinese(96,0,42,16,1); 	//关
	}
	 
}

/**
  * @brief  显示系统阈值设置界面1
  * @param  无
  * @retval 无
  */
void OLED_settingsPage1(void)
{
	//系统时间xxx
	OLED_ShowChinese(16,0,59,16,1);	
	OLED_ShowChinese(32,0,60,16,1);	
	OLED_ShowChinese(48,0,53,16,1);	
	OLED_ShowChinese(64,0,54,16,1);	
	 

	//显示“湿度阈值”
	OLED_ShowChinese(16,32,1,16,1);	
	OLED_ShowChinese(32,32,2,16,1);	
	OLED_ShowChinese(48,32,26,16,1);	
	OLED_ShowChinese(64,32,27,16,1);	
	OLED_ShowChar(80,32,':',16,1);
	 
	
}

void OLED_settingsPage2(void)
{
	//定时开启阈值
	OLED_ShowChinese(16,16,57,16,1);	
	OLED_ShowChinese(32,16,58,16,1);	
	OLED_ShowChinese(48,16,40,16,1);	
	OLED_ShowChar(64,16,':',16,1);

 

}
void OLED_settingsPage3(void)
{
	//时间阈值
	OLED_ShowChinese(30,0,59,16,1);	
	OLED_ShowChinese(46,0,60,16,1);	
	OLED_ShowChinese(62,0,53,16,1);	
 
}

void SettingsThresholdDisplay1(void)
{
	//显示温度阈值数值
	OLED_ShowNum(90,16, Sensorthreshold.tempValue, 2,16,1);
	//显示湿度阈值数值
	 
}

void SettingsThresholdDisplay2(void)
{
	//显示定时开启时间
	OLED_ShowNum(80, 16, OPEN_HOUR, 2, 16, 1);
  OLED_ShowChar(96, 16, ':', 16, 1);
  OLED_ShowNum(112, 16, OPEN_MINUTE, 2, 16, 1);
	
	 
}

void SettingsThresholdDisplay3(void)
{
	//显示时间
	OLED_ShowNum(30,32,MyRTC_Time[3],2,16,1);
	OLED_ShowChar(46,32,':',16,1);
	 
}

/**
  * @brief  记录自动模式界面下按KEY2的次数
  * @param  无
  * @retval 返回次数
  */
uint8_t SetAuto(void)  
{
	if(KeyNum == KEY_2)
	{
		KeyNum = 0;
		count_a++;
		if (count_a > 2)
		{
			count_a = 1;
		}
		OLED_Clear();
	}
	return count_a;
}

/**
  * @brief  记录手动模式界面下按KEY2的次数
  * @param  无
  * @retval 返回次数
  */
uint8_t SetManual(void)  
{
	if(KeyNum == KEY_2)
	{
		KeyNum = 0;
		count_m++;
		if (count_m == 1)
		{
			OLED_Clear();
		}
		if (count_m > 2)  		//一共可以控制的外设数量
		{
			OLED_Clear();
			count_m = 1;
		}
	}
	return count_m;
}

/**
  * @brief  记录阈值界面下按KEY2的次数
  * @param  无
  * @retval 返回次数
  */
uint8_t SetSelection(void)
{
    uint8_t old_count_s = count_s;  // 临时保存切换前页面

    if(KeyNum == KEY_2 && is_secondary_menu == 0)
    {
        KeyNum = 0;
        count_s++;
        if (count_s > 6)
        {
            count_s = 1;
        }

        // 仅跨界面切换清屏（界面1?界面2），界面1内部不清屏
        if( (old_count_s <=4 && count_s >=5) || (old_count_s >=5 && count_s <=4) )
        {
            OLED_Clear();
            OLED_Refresh();
            delay_ms(5);
        }
    }
    return count_s;
}

/**
  * @brief  显示手动模式界面的选择符号
  * @param  num 为显示的位置
  * @retval 无
  */
void OLED_manualOption(uint8_t num)
{
	switch(num)
	{
		case 1:	
			OLED_ShowChar(0, 0,'>',16,1);
			OLED_ShowChar(0,16,' ',16,1);
			OLED_ShowChar(0,32,' ',16,1);
			OLED_ShowChar(0,48,' ',16,1);
			break;
	 
	}
}

/**
  * @brief  显示阈值界面的选择符号
  * @param  num 为显示的位置
  * @retval 无
  */
void OLED_settingsOption(uint8_t num)
{
	static uint8_t prev_num = 1; 

    // 清除上一次光标（仅操作光标位置，不影响数据）
    switch(prev_num)
    {
        case 1: OLED_ShowChar(0, 0, ' ', 16, 1); break; // 系统时间行
        case 2: OLED_ShowChar(0, 16, ' ', 16, 1); break; // 温度阈值行
        
        default: break;
    }
	switch(num)
	{
		case 1:	
			OLED_ShowChar(0, 0,'>',16,1);
			OLED_ShowChar(0,16,' ',16,1);
			OLED_ShowChar(0,32,' ',16,1);
			OLED_ShowChar(0,48,' ',16,1);
			break;
		case 2:	
			OLED_ShowChar(0, 0,' ',16,1);
			OLED_ShowChar(0,16,'>',16,1);
			OLED_ShowChar(0,32,' ',16,1);
			OLED_ShowChar(0,48,' ',16,1);
			break;
		 
		 
		default: break;
	}
	 prev_num = num;
    OLED_Refresh(); // 仅刷新光标，数据区域无变化
}

/**
  * @brief  自动模式控制函数
  * @param  无
  * @retval 无
  */
void AutoControl(void)//自动控制
{
	if( (sensorData.temp > Sensorthreshold.tempValue)    // 温度超标
 || (sensorData.humi > Sensorthreshold.humiValue)    // 湿度超标
 || (sensorData.CO == 1) )                            // 一氧化碳超标（假设1=超标）
  {
   BEEP_ON; // 任一超标，需要开蜂鸣器
  }
  else
		BEEP_OFF;
		
  
}

/**
  * @brief  手动模式控制函数
  * @param  无
  * @retval 无
  */
void ManualControl(uint8_t num)
{
	switch(num)
	{
		case 1:  
            if(KeyNum == KEY_3)
            {
                driveData.LED_Flag = 1;  
                KeyNum = 0;  
                printf("[按键] KEY3按下，LED_Flag置1\n");  
            }
            if(KeyNum == KEY_4)
            {
                driveData.LED_Flag = 0; 
                KeyNum = 0;  
                printf("[按键] KEY4按下，LED_Flag置0\n"); 
            }
            break;

		 
	}
}

/**
  * @brief  控制函数
  * @param  无
  * @retval 无
  */
void Control_Manager(void)
{
    if(driveData.LED_Flag )
    {	
        LED_On(); 
        printf("           [Control] LED_Flag=%d（LED开启）\n", driveData.LED_Flag);
    }
   
		 
}

/**
  * @brief  阈值设置函数
  * @param  无
  * @retval 无
  */
void ThresholdSettings(uint8_t num)
{
	switch (num)
	{
		case 2:
			if (KeyNum == KEY_3)
			{
				KeyNum = 0;
				Sensorthreshold.tempValue += 1;
				if (Sensorthreshold.tempValue > 40)
				{
					Sensorthreshold.tempValue = 20;
				}
			}
		 
			break;
			
		 
        default: break;
	}
}

void FLASH_ReadThreshold()
{
	Sensorthreshold.tempValue = FLASH_R(FLASH_START_ADDR);	//从指定页的地址读FLASH;
	Sensorthreshold.humiValue = FLASH_R(FLASH_START_ADDR+2);	//从指定页的地址读FLASH;
	Sensorthreshold.luxValue = FLASH_R(FLASH_START_ADDR+4);	//从指定页的地址读FLASH;
	 
}

/**
  * @brief  机智云按键配网
  * @param  无
  * @retval 无
  */
void ScanGizwitsMode(void)
{
	if(!KEY3)
	{
		delay_ms(20);
		if(!KEY3)
		{
			//显示“热点配网”
	    OLED_ShowChinese(32,16,44,16,1); 	//热
	    OLED_ShowChinese(48,16,45,16,1);	//点                               
	  
			OLED_Refresh();
		}
	}
	else if(!KEY4)
	{
		delay_ms(20);
		if(!KEY4)
		{
			//显示“一键配网”
	    OLED_ShowChinese(32,16,46,16,1); 	//一
	    OLED_ShowChinese(48,16,47,16,1);	//键
	    OLED_ShowChinese(64,16,48,16,1);	//配
	   
		}
	}
}


int main(void)
{ 
  SystemInit();//配置系统时钟为72M	
	delay_init(72);
	ADCx_Init();
	LDR_Init();
	DHT11_Init();
	MQ7_Init();
	 
	gizwitsInit();
	TIM3_Int_Init(72-1,1000-1);
	gizwitsSetMode(WIFI_AIRLINK_MODE); //默认一键配网
	delay_ms(200);
	ScanGizwitsMode();//机智云配网模式
  delay_ms(500);
	OLED_Clear();
  //flash读取
	FLASH_ReadThreshold();
	
	    if (Sensorthreshold.tempValue > 40 || Sensorthreshold.humiValue >80 || 
        Sensorthreshold.luxValue > 300 || OPEN_HOUR > 23 || OPEN_MINUTE > 59 || CLOSE_HOUR > 23||CLOSE_MINUTE>59||MyRTC_Time[3]>23||MyRTC_Time[4]>59||MyRTC_Time[5]>59)
    {
        FLASH_W(FLASH_START_ADDR, 35, 80, 100, 20,21,20,22,20,20,00);
				FLASH_ReadThreshold();
    }
	
	
	
	TIM2_Init(72-1,1000-1);
  MyRTC_Init();	
	printf("Start \n");
	USART3_SendString("AF:30");   //音量调到最大
	delay_ms(300);
	USART3_SendString("A7:00001");  //欢迎使用
	delay_ms(1000);
	
  while (1)
  {	
		SensorScan();	//获取传感器数据	
		userHandle();//上报
		
		
		switch(mode)
		{
			case AUTO_MODE:
				if(SetAuto() ==1 )
				{
					OLED_autoPage1();	//显示主页面1固定信息
					SensorDataDisplay1();	//显示传感器1数据
				}
				else
				{
					OLED_autoPage2();	//显示主页面2固定信息
					SensorDataDisplay2();	//显示传感器2数据
				}
				AutoControl();
				
				/*按键1按下时切换模式*/
				if (KeyNum == KEY_1)   //系统模式mode  1自动  2手动  3设置
				{
					KeyNum = 0;
					mode = MANUAL_MODE;
					count_m = 1;
					OLED_Clear();
				}
				
				if (KeyNum == KEY_Long1)
				{
					KeyNum = 0;
					mode = SETTINGS_MODE;
					count_s = 1;
					OLED_Clear();
				}
				
				Control_Manager();
				 
				break;
				
			case MANUAL_MODE:
				OLED_manualOption(SetManual());
				ManualControl(SetManual());
				if (SetManual() <= 4)		
				{	
					OLED_manualPage1();
					ManualSettingsDisplay1();
				}
				
				if (KeyNum == KEY_1)   //系统模式mode  0手动  1自动（默认）
				{
					KeyNum = 0;
					mode = AUTO_MODE;
					count_a = 1;
					OLED_Clear();
				}
				Control_Manager();
				userHandle();//上报
				gizwitsHandle((dataPoint_t *)&currentDataPoint);//下发
				break;
			

 case SETTINGS_MODE:
{
    // 1. 静态变量：标记页面固定文字是否已初始化（避免重复绘制）
    static uint8_t is_threshold_page_inited = 0;  // 阈值页面（case2-4）文字初始化标记
    static uint8_t is_timing_page_inited = 0;     // 定时页面（case5-6）文字初始化标记
    static uint8_t first_enter_case5 = 1;         // 定时开启页面首次进入清屏标记
    static uint8_t first_enter_case6 = 1;         // 定时关闭页面首次进入清屏标记
    
    uint8_t old_count_s = count_s;                // 切换前页面编号（用于判断跨页面）
    uint8_t curr_count_s = SetSelection();        // 当前页面编号（含KEY2切换逻辑）
    uint8_t refresh_needed = 1;                   // 动态元素刷新标志
    uint8_t sys_time_back_refresh = 0;            // 系统时间二级菜单返回刷新标志

    // -------------------------- 二级菜单逻辑（时/分/秒调整） --------------------------
    if (is_secondary_menu == 1)
    {
				//-------------------------------------2
        // 按键2：切换光标位置（时→分→秒 / 时→分）
        if (KeyNum == KEY_2)
        {
            KeyNum = 0;
            secondary_pos++;							
					//移动光标
            // 清除旧光标（仅操作光标区域，不碰文字）--移动光标
            switch(secondary_type)																//secondary_type--KEY3改变
            {
                case 0: // RTC时间：清除时/分/秒光标
                    OLED_ShowChar(30, 16, ' ', 16, 1);
                    OLED_ShowChar(62, 16, ' ', 16, 1);
                    OLED_ShowChar(94, 16, ' ', 16, 1);
                    secondary_pos = (secondary_pos > 3) ? 1 : secondary_pos;
                    break;
                case 1: // 定时开启：清除时/分光标
                case 2: // 定时关闭：清除时/分光标
                    OLED_ShowChar(80, (secondary_type==1?0:32), ' ', 16, 1);
                    OLED_ShowChar(112, (secondary_type==1?0:32), ' ', 16, 1);
                    secondary_pos = (secondary_pos > 2) ? 1 : secondary_pos;
                    break;
            }
        }

				
				//-------------------------------------1
        // 按键3：数值+1
        else if (KeyNum == KEY_3)
        {
            KeyNum = 0;
            switch(secondary_type)
            {
                case 0: // RTC时间调整
                    switch(secondary_pos)
                    {
                        case 1: MyRTC_Time[3] = (MyRTC_Time[3]+1)%24; break; // 时+1
                        case 2: MyRTC_Time[4] = (MyRTC_Time[4]+1)%60; break; // 分+1
                        case 3: MyRTC_Time[5] = (MyRTC_Time[5]+1)%60; break; // 秒+1
                    }
                    MyRTC_SetTime(); // 写入硬件时钟
                    break;
                case 1: // 定时开启调整
                    if (secondary_pos == 1) OPEN_HOUR = (OPEN_HOUR+1)%24;
                    else OPEN_MINUTE = (OPEN_MINUTE+1)%60;
                    break;
                case 2: // 定时关闭调整
                    if (secondary_pos == 1) CLOSE_HOUR = (CLOSE_HOUR+1)%24;
                    else CLOSE_MINUTE = (CLOSE_MINUTE+1)%60;
                    break;
            }
        }

        // 按键4：数值-1
        else if (KeyNum == KEY_4)
        {
            KeyNum = 0;
            switch(secondary_type)
            {
                case 0: // RTC时间调整
                    switch(secondary_pos)
                    {
                        case 1: MyRTC_Time[3] = (MyRTC_Time[3]+23)%24; break; // 时-1
                        case 2: MyRTC_Time[4] = (MyRTC_Time[4]+59)%60; break; // 分-1
                        case 3: MyRTC_Time[5] = (MyRTC_Time[5]+59)%60; break; // 秒-1
                    }
                    MyRTC_SetTime(); // 写入硬件时钟
                    break;
                case 1: // 定时开启调整
                    if (secondary_pos == 1) OPEN_HOUR = (OPEN_HOUR+23)%24;
                    else OPEN_MINUTE = (OPEN_MINUTE+59)%60;
                    break;
                case 2: // 定时关闭调整
                    if (secondary_pos == 1) CLOSE_HOUR = (CLOSE_HOUR+23)%24;
                    else CLOSE_MINUTE = (CLOSE_MINUTE+59)%60;
                    break;
            }
        }

        // 按键1：返回一级菜单（仅刷新动态元素，不重绘固定文字）
        else if (KeyNum == KEY_1)
        {
            KeyNum = 0;
            // 系统时间二级菜单返回：清除二级菜单残留
            if (secondary_type == 0)
            {

              sys_time_back_refresh = 1;
							OLED_Clear();
            }
            // 定时二级菜单返回：清除光标
            else
            {
                OLED_ShowChar(80, (secondary_type==1?0:32), ' ', 16, 1);
                OLED_ShowChar(112, (secondary_type==1?0:32), ' ', 16, 1);
            }

     

            is_secondary_menu = 0; // 退出二级菜单
            secondary_pos = 1;
            refresh_needed = 1;
        }

        // 显示二级菜单动态内容（光标+数据）
        if (is_secondary_menu == 1)
        {
            switch(secondary_type)
            {
                case 0: // RTC时间设置界面
                    OLED_settingsPage3(); // 仅绘制时间设置框架（固定文字，仅1次）
                    SettingsThresholdDisplay3(); // 刷新时间数据
                    // 绘制当前光标
                    switch(secondary_pos)
                    {
                        case 1: OLED_ShowChar(30, 16, 'v', 16, 1); break;
                        case 2: OLED_ShowChar(62, 16, 'v', 16, 1); break;
                        case 3: OLED_ShowChar(94, 16, 'v', 16, 1); break;
                    }
              
            }
        }

        // 执行刷新（仅动态元素）
        if (sys_time_back_refresh || refresh_needed)
        {
            OLED_Refresh();
            sys_time_back_refresh = 0;
            refresh_needed = 0;
        }
    }

    // -------------------------- 一级菜单逻辑（阈值/定时页面切换） --------------------------
    else
    {
        // 1. 固定文字初始化：仅在首次进入或跨页面切换时绘制（不重复刷新）
        // 阈值页面（case2-4）：绘制“温度阈值”“湿度阈值”等固定文字
        if (curr_count_s >= 1 && curr_count_s <= 4)
        {
            if (is_threshold_page_inited == 0 || old_count_s >= 5)
            {
                OLED_settingsPage1(); // 仅绘制1次固定文字
                is_threshold_page_inited = 1; // 标记已初始化
                is_timing_page_inited = 0;    // 重置定时页面标记
            }
        }
        

        // 2. 仅刷新动态元素：光标 + 数据（不碰固定文字）
        OLED_settingsOption(curr_count_s); // 刷新光标位置（仅左侧1列，不覆盖文字）
        
        // 阈值页面（case2-4）：处理数值修改并刷新数据
        if (curr_count_s >= 1 && curr_count_s <= 4)
        {
            ThresholdSettings(curr_count_s); // KEY3/KEY4修改阈值
            SettingsThresholdDisplay1();     // 仅刷新阈值数值（不绘制文字）
            OLED_Refresh();                  // 仅更新光标和数值到屏幕
        }
        // 定时页面（case5-6）：刷新定时数据
        

        
    }
}
		userHandle();//上报
		gizwitsHandle((dataPoint_t *)&currentDataPoint);//下发
break;
default: break;

		}
		userHandle();//上报
		gizwitsHandle((dataPoint_t *)&currentDataPoint);//下发
	}
}

