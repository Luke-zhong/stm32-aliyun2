#include "usart.h"
#include "device.h"
#include "spi.h"
#include "ult.h"
#include "w5500.h"
#include "socket.h"
#include "string.h"
#include "dns.h"
#include "tcp_client.h"
#include "ntp.h"
/************************************************
 ALIENTEK ENC28J60 ����ʵ��
 ����RAW API��TCP Clientʵ��
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/


uint8 buffer[2048]={"TKKMt4nMF8U.iot-as-mqtt.cn-shanghai.aliyuncs.com"};/*����һ��2KB�Ļ���*/
																																				/*DNS��������*/
int main(void)
{	 
	
	uint8 *domain_name;
	uint8 dns_retry_cnt=0;
  uint8 dns_ok=0;
	uint8 len,dns_flag=0;										// ���崮����������ݳ��ȡ���ʼ��DNS��־λ
	
	Systick_Init(72);

	GPIO_Configuration();
	uart_init(115200);
	
	WIZ_SPI_Init();
  Reset_W5500();
	
	/***** W5500��IP��Ϣ��ʼ�� *****/
	set_default(); 
	set_network();						
		
	printf("W5500 Init Complete!\r\n");
   printf("Start Ping Test!\r\n");
	//ntpclient_init();


 while(1)
  {	
//		dns_flag=1;													// DNS��־λ��1
//		if(dns_flag==1)
//		{
//			
//			if(dns_num>=6)											// DNS������6
//			{
//				dns_flag=0;												// DNS��־λ��0
//				dns_num=0;												// dns_num��0
//			}
//			else
//				do_dns(buffer);										// DNS����
//			
//		}
	
	MQTT_CON_ALI();
	   printf("connecting1.......\r\n");
	}	
}

