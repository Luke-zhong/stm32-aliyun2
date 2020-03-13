#include "stm32f10x_conf.h"


uint8_t dns_ip[4] = {114,114,114,114};
uint8_t dns_buff[512];
uint8_t dns_rip[4];
void w5500_gpio_init()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
}
void run(void *pvParameters)
{
	uint32_t receive_data;

	LOG_I("run task start...\r\n");
	while(1)
	{
		/*if(DNS_run(dns_ip,"www.baidu.com",dns_rip)==0)
		{
			LOG_E("DNS failed.\r\n");
		}
		LOG_D("DNS:%d.%d.%d.%d.\r\n",dns_rip[0],dns_rip[1],dns_rip[2],dns_rip[3]);*/
		if(user_dns("www.baidu.com",dns_rip))
		{
			LOG_I("DNS:%d.%d.%d.%d.",dns_rip[0],dns_rip[1],dns_rip[2],dns_rip[3]);
			vTaskDelay(20000);
		}
		else{
			LOG_D("dns fail.retry...\r\n");
			vTaskDelay(1000);
		}
			//vTaskDelay(1000);		
	}
}

void getStaticTask(void *pvParameters)
{
	uint8_t PHY;
	uint8_t HW_S=0;
	LOG_I("PHY check task.\r\n");
	while(1)
	{
		PHY = getPHYCFGR();	//��ȡ��·��״̬
		//LOG_D("PHY:%x.\r\n",PHY);
		if((PHY & 0x01) && (HW_S!=2))	//�ж���·���Ƿ�����
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
	w5500_gpio_init();
	WIZ_SPI_Init();		//��ʼ��W5500����W5500�Ĵ�����ȡ��������д��
	GPIO_ResetBits(GPIOB,GPIO_Pin_9);
	vTaskDelay(2);
	GPIO_SetBits(GPIOB,GPIO_Pin_9);
	vTaskDelay(2000);
	
	DNS_init(3,dns_buff); //DNS��ʼ��
	if(!wizchip_init(NULL,NULL))	//w5500оƬ��ʼ��
	{
		LOG_I("wizchip init success.\r\n");
	}
	else
	{
		LOG_I("wizchip init fail.\r\n");
	}
	//xTaskCreate(getStaticTask,"check_link",512,NULL,1,NULL);
	xTaskCreate(run,"ethernet_task",2048,NULL,1,NULL);  //������̫������
	vTaskDelete(NULL);
}


int main(void)
{
	LogUsartInit();
	LOG_I("usart init success.\r\n");

	xTaskCreate(init,"init_task",512,NULL,1,NULL);  //������ʼ����
	vTaskStartScheduler();	//�������������
	while(1);
}
