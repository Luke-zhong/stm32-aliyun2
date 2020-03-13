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
	LOG_D("log usart init.\r\n");
	xTaskCreate(task1,"LED_TASK1",40,NULL,1,NULL); 
	vTaskStartScheduler();	//启动任务调度器
	LOG_E("task start error!\r\n");
	while(1);
}
