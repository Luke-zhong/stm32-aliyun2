#include "stm32f10x_conf.h"

void run(void *pvParameters)
{
	printf("run task start...\r\n");
	while(1)
	{
		printf("dns...\r\n");
		if(do_dns("iot.diqi.sh") == 1)
		{
			printf("dns success.\r\n");
			while(1);
		}
		else
		{
			vTaskDelay(1000);
			printf("run.\r\n");
		}
	}
}
void init(void *pvParameters)
{
 	pvParameters = pvParameters;
	printf("init task...\r\n");
	WIZ_SPI_Init(); //w5500 spi初始化
	printf("W5500 SPI init success.\r\n");
	GPIO_Configuration(); //W5500外部GPIO初始化
	Reset_W5500(); //硬重启W5500

	
	printf("reset w5500...\r\n");
	set_default();
	set_network();
	xTaskCreate(run,"run_task",512,NULL,1,NULL);  //创建运行任务
	vTaskDelete(NULL);
}


int main(void)
{
	LogUsartInit();
	LOG_D("usart init success.\r\n");
	xTaskCreate(init,"LED_TASK1",512,NULL,1,NULL);  //创建开始任务
	vTaskStartScheduler();	//启动任务调度器
	while(1);
}
