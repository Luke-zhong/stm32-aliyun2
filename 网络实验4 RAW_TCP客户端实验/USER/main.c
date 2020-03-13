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
 ALIENTEK ENC28J60 网络实验
 基于RAW API的TCP Client实验
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/


uint8 buffer[2048]={"TKKMt4nMF8U.iot-as-mqtt.cn-shanghai.aliyuncs.com"};/*定义一个2KB的缓存*/
																																				/*DNS解析域名*/
int main(void)
{	 
	
	uint8 *domain_name;
	uint8 dns_retry_cnt=0;
  uint8 dns_ok=0;
	uint8 len,dns_flag=0;										// 定义串口输入的数据长度、初始化DNS标志位
	
	Systick_Init(72);

	GPIO_Configuration();
	uart_init(115200);
	
	WIZ_SPI_Init();
  Reset_W5500();
	
	/***** W5500的IP信息初始化 *****/
	set_default(); 
	set_network();						
		
	printf("W5500 Init Complete!\r\n");
   printf("Start Ping Test!\r\n");
	//ntpclient_init();


 while(1)
  {	
//		dns_flag=1;													// DNS标志位置1
//		if(dns_flag==1)
//		{
//			
//			if(dns_num>=6)											// DNS次数≥6
//			{
//				dns_flag=0;												// DNS标志位清0
//				dns_num=0;												// dns_num清0
//			}
//			else
//				do_dns(buffer);										// DNS过程
//			
//		}
	
	MQTT_CON_ALI();
	   printf("connecting1.......\r\n");
	}	
}

