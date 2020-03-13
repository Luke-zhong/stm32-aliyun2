#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "device.h"
#include "w5500.h"
#include "socket.h"
#include "string.h"
#include "spi.h"
/************************************************
 ALIENTEK 精英STM32F103开发板 FreeRTOS实验2-1
 FreeRTOS移植实验-库函数版本
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/

//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		128  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);

//任务优先级
#define LED0_TASK_PRIO		2
//任务堆栈大小	
#define LED0_STK_SIZE 		50  
//任务句柄
TaskHandle_t LED0Task_Handler;
//任务函数
void led0_task(void *pvParameters);

//任务优先级
#define W5500_TASK_PRIO		3
//任务堆栈大小	
#define W5500_STK_SIZE 		1500  
//任务句柄
TaskHandle_t W5500Task_Handler;
//任务函数
void w5500_task(void *pvParameters);

/********private code********/
uint8 buffer[2048];



/************private code****************/
int main(void)
{
    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4	 
	HAL_Init();                    	 	//初始化HAL库    
    Stm32_Clock_Init(RCC_PLL_MUL9);   	//设置时钟,72M
	delay_init(72);	    				//延时函数初始化	  
	uart_init(115200);					//初始化串口
	LED_Init();		  					//初始化LED
	 
	//创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
    vTaskStartScheduler();          //开启任务调度
}

//开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
    //创建LED0任务
    xTaskCreate((TaskFunction_t )led0_task,     	
                (const char*    )"led0_task",   	
                (uint16_t       )LED0_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )LED0_TASK_PRIO,	
                (TaskHandle_t*  )&LED0Task_Handler);   
    //创建LED1任务
    xTaskCreate((TaskFunction_t )w5500_task,     
                (const char*    )"w5500_task",   
                (uint16_t       )W5500_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )W5500_TASK_PRIO,
                (TaskHandle_t*  )&W5500Task_Handler);         
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}

//LED0任务函数 
void led0_task(void *pvParameters)
{
    while(1)
    {
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET); 	//LED0对应引脚PB5拉低，亮，等同于LED0(0)
        HAL_GPIO_WritePin(GPIOE,GPIO_PIN_5,GPIO_PIN_SET);   	//LED1对应引脚PE5拉高，灭，等同于LED1(1)
        delay_ms(500);											//延时500ms
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);   	//LED0对应引脚PB5拉高，灭，等同于LED0(1)
        HAL_GPIO_WritePin(GPIOE,GPIO_PIN_5,GPIO_PIN_RESET); 	//LED1对应引脚PE5拉低，亮，等同于LED1(0)
        delay_ms(500);                                      	//延时500ms 
    }
}   

//LED1任务函数
void w5500_task(void *pvParameters)
{

  uint8 server_ip[4]={192,168,1,151};			// 配置远程服务器IP地址
  uint16 server_port=5000;								// 配置远程服务器端口
  uint16 local_port=6000;									// 初始化一个本地端口
  uint16 len=0;
	
	
	/***** GPIO、SPI初始化 *****/
	GPIO_Configuration();			
	WIZ_SPI_Init();
		
	
	
	/***** 硬重启W5500 *****/
	Reset_W5500();
	
	/***** W5500的IP信息初始化 *****/
	set_default(); 														// 设置默认MAC、IP、GW、SUB、DNS
	set_network();														// 配置初始化IP信息并打印，初始化8个Socket
	
	/***** 打开W5500的Keepalive功能 *****/
	//setkeepalive(0);
	
	printf("TCP Server IP: %d.%d.%d.%d \r\n",server_ip[0],server_ip[1],server_ip[2],server_ip[3]);
  printf("TCP Server Port: %d \r\n",server_port);	
	printf("W5500 Init Complete!\r\n");
  printf("Start TCP Client Test!\r\n"); 
  
	/*Socket状态机，MCU通过读Sn_SR(0)的值进行判断Socket应该处于何种状态
		Sn_SR状态描述：
		0x00		SOCK_CLOSED
		0x13		SOCK_INIT
		0x14		SOCK_LISTEN
		0x17		SOCK_ESTABLISHED
		0x1C		SOCK_CLOSE_WAIT
		0x22		SOCK_UDP
	*/
	while(1)																				// Socket状态机
	{	
		switch(getSn_SR(0))														// 获取socket0的状态
		{
			case SOCK_INIT:															// Socket处于初始化完成(打开)状态
					connect(0, server_ip,server_port);			// 配置Sn_CR为CONNECT，并向TCP服务器发出连接请求
			break;
			case SOCK_ESTABLISHED:											// Socket处于连接建立状态
					if(getSn_IR(0) & Sn_IR_CON)   					
					{
						setSn_IR(0, Sn_IR_CON);								// Sn_IR的CON位置1，通知W5500连接已建立
					}
					// 数据回环测试程序：数据从上位机服务器发给W5500，W5500接收到数据后再回给服务器
					len=getSn_RX_RSR(0);										// len=Socket0接收缓存中已接收和保存的数据大小
					if(len>0)
					{
						recv(0,buffer,len);										// W5500接收来自服务器的数据，并通过SPI发送给MCU
						printf("%s\r\n",buffer);							// 串口打印接收到的数据
						send(0,buffer,len);										// 接收到数据后再回给服务器，完成数据回环
					}
					// W5500从串口发数据给客户端程序，数据需以回车结束
					if(USART_RX_STA & 0x8000)								// 判断串口数据是否接收完成
					{					   
						len=USART_RX_STA & 0x3fff;						// 获取串口接收到数据的长度
						send(0,USART_RX_BUF,len);							// W5500向客户端发送数据
						USART_RX_STA=0;												// 串口接收状态标志位清0
						memset(USART_RX_BUF,0,len+1);					// 串口接收缓存清0
					}
			break;
			case SOCK_CLOSE_WAIT:												// Socket处于等待关闭状态
					close(0);																// 关闭Socket0
			break;
			case SOCK_CLOSED:														// Socket处于关闭状态
					socket(0,Sn_MR_TCP,local_port,Sn_MR_ND);		// 打开Socket0，并配置为TCP无延时模式，打开一个本地端口
			break;
		}
	}
}
