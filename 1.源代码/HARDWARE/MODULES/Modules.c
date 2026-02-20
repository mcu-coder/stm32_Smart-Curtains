# include "Modules.h"


/**
  * @brief  传感器数据扫描
  * @param  无
  * @retval 无
  */

void SensorScan(void)
{
	DHT11_Read_Data(&sensorData.temp, &sensorData.humi); 
	
	
}


