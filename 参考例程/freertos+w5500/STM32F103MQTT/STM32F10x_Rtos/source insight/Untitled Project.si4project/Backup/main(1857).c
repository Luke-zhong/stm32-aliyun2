#include "stm32f10x_conf.h"

void run(void *pvParameters)
{
	printf("run task start...\r\n");
	while(1)
	{
		printf("dns...\r\n");
		//if(do_dns("iot.diqi.sh") == 1)
		{
			printf("dns success.\r\n");
			while(1);
		}
		//else
		{
			vTaskDelay(1000);
			printf("run.\r\n");
		}
	}
}
void test(void *pvParameters)
{
	uint8_t PHY;
	while(1)
	{
		LOG_D("PHY:%x.\r\n",PHY);
		vTaskDelay(500);
	}
}
void init(void *pvParameters)
{
 	pvParameters = pvParameters;
	printf("init task...\r\n");
	LOG_I("init success!\r\n");
	xTaskCreate(test,"check_link",512,NULL,1,NULL);
	xTaskCreate(run,"ethernet_task",512,NULL,1,NULL);  //创建以太网任务
	vTaskDelete(NULL);
}


int main(void)
{
	LogUsartInit();
	LOG_I("usart init success.\r\n");
	xTaskCreate(init,"init_task",512,NULL,1,NULL);  //创建开始任务
	vTaskStartScheduler();	//启动任务调度器
	while(1);
}
