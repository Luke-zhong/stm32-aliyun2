#include "stm32f10x_conf.h"


uint8_t dns_ip[4] = {114,114,114,114};
uint8_t dns_buff[512];
uint8_t dns_rip[4];
void run(void *pvParameters)
{
	printf("run task start...\r\n");
	while(1)
	{
		printf("dns...\r\n");
		DNS_run(dns_ip,"www.baidu.com",dns_rip);
		printf("DNS:%d.%d.%d.%d.\r\n",dns_rip[0],dns_rip[1],dns_rip[2],dns_rip[3]);
		vTaskDelay(5000);
	}
}
void getStaticTask(void *pvParameters)
{
	uint8_t PHY;
	uint8_t HW_S=0;
	printf("PHY check task.\r\n");
	while(1)
	{
		PHY = getPHYCFGR();	//获取链路层状态
		if((PHY & 0x01) && (HW_S!=2))	//判断链路层是否正常
		{
			LOG_I("ethernet connect.\r\n");
			HW_S = 2;
		}
		else if(!(PHY&0x01) && (HW_S!=1))
		{
			HW_S = 1;
			LOG_I("ethernet disconnect.\r\n");
		}
		vTaskDelay(200);
	}
}
void init(void *pvParameters)
{
 	pvParameters = pvParameters;
	printf("init task...\r\n");
	LOG_I("init success!\r\n");
	xTaskCreate(getStaticTask,"check_link",512,NULL,1,NULL);
	xTaskCreate(run,"ethernet_task",512,NULL,1,NULL);  //创建以太网任务
	vTaskDelete(NULL);
}


int main(void)
{
	LogUsartInit();
	LOG_I("usart init success.\r\n");
	WIZ_SPI_Init();		//初始化W5500，从W5500寄存器获取数据重新写入
	DNS_init(0,dns_buff); //DNS初始化
	if(wizchip_init(NULL,NULL))
		LOG_I("wizchip init success.\r\n");
	else
		LOG_E("wizchip init fail.\r\n");
	
	xTaskCreate(init,"init_task",512,NULL,1,NULL);  //创建开始任务
	vTaskStartScheduler();	//启动任务调度器
	while(1);
}
