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
	WIZ_SPI_Init(); //w5500 spi��ʼ��
	GPIO_Configuration(); //W5500�ⲿGPIO��ʼ��
	Reset_W5500(); //Ӳ����W5500

	set_default();
	set_network();
	
	LOG_D("log usart init.\r\n");
	xTaskCreate(task1,"LED_TASK1",40,NULL,1,NULL); 
	vTaskStartScheduler();	//�������������
	LOG_E("task start error!\r\n");
	while(1);
}
