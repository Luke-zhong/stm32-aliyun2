#include "stm32f10x_conf.h"


void task1(void *pvParameters)
{
 	pvParameters = pvParameters;
	printf("task1.\r\n");
	while(1)
	{
		vTaskDelay(1000);
		LOG_E("test task1..\r\n");
	}
}


int main(void)
{
	LogUsartInit();
	WIZ_SPI_Init(); //w5500 spi初始化
	GPIO_Configuration(); //W5500外部GPIO初始化
	Reset_W5500(); //硬重启W5500

	set_default();
	set_network();
	
	LOG_D("log usart init.\r\n");
	xTaskCreate(task1,"LED_TASK1",40,NULL,1,NULL); 
	vTaskStartScheduler();	//启动任务调度器
	LOG_E("task start error!\r\n");
	while(1);
}
