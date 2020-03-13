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
 ALIENTEK ��ӢSTM32F103������ FreeRTOSʵ��2-1
 FreeRTOS��ֲʵ��-�⺯���汾
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

//�������ȼ�
#define START_TASK_PRIO		1
//�����ջ��С	
#define START_STK_SIZE 		128  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);

//�������ȼ�
#define LED0_TASK_PRIO		2
//�����ջ��С	
#define LED0_STK_SIZE 		50  
//������
TaskHandle_t LED0Task_Handler;
//������
void led0_task(void *pvParameters);

//�������ȼ�
#define W5500_TASK_PRIO		3
//�����ջ��С	
#define W5500_STK_SIZE 		1500  
//������
TaskHandle_t W5500Task_Handler;
//������
void w5500_task(void *pvParameters);

/********private code********/
uint8 buffer[2048];



/************private code****************/
int main(void)
{
    //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4	 
	HAL_Init();                    	 	//��ʼ��HAL��    
    Stm32_Clock_Init(RCC_PLL_MUL9);   	//����ʱ��,72M
	delay_init(72);	    				//��ʱ������ʼ��	  
	uart_init(115200);					//��ʼ������
	LED_Init();		  					//��ʼ��LED
	 
	//������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������              
    vTaskStartScheduler();          //�����������
}

//��ʼ����������
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //�����ٽ���
    //����LED0����
    xTaskCreate((TaskFunction_t )led0_task,     	
                (const char*    )"led0_task",   	
                (uint16_t       )LED0_STK_SIZE, 
                (void*          )NULL,				
                (UBaseType_t    )LED0_TASK_PRIO,	
                (TaskHandle_t*  )&LED0Task_Handler);   
    //����LED1����
    xTaskCreate((TaskFunction_t )w5500_task,     
                (const char*    )"w5500_task",   
                (uint16_t       )W5500_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )W5500_TASK_PRIO,
                (TaskHandle_t*  )&W5500Task_Handler);         
    vTaskDelete(StartTask_Handler); //ɾ����ʼ����
    taskEXIT_CRITICAL();            //�˳��ٽ���
}

//LED0������ 
void led0_task(void *pvParameters)
{
    while(1)
    {
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET); 	//LED0��Ӧ����PB5���ͣ�������ͬ��LED0(0)
        HAL_GPIO_WritePin(GPIOE,GPIO_PIN_5,GPIO_PIN_SET);   	//LED1��Ӧ����PE5���ߣ��𣬵�ͬ��LED1(1)
        delay_ms(500);											//��ʱ500ms
        HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_SET);   	//LED0��Ӧ����PB5���ߣ��𣬵�ͬ��LED0(1)
        HAL_GPIO_WritePin(GPIOE,GPIO_PIN_5,GPIO_PIN_RESET); 	//LED1��Ӧ����PE5���ͣ�������ͬ��LED1(0)
        delay_ms(500);                                      	//��ʱ500ms 
    }
}   

//LED1������
void w5500_task(void *pvParameters)
{

  uint8 server_ip[4]={192,168,1,151};			// ����Զ�̷�����IP��ַ
  uint16 server_port=5000;								// ����Զ�̷������˿�
  uint16 local_port=6000;									// ��ʼ��һ�����ض˿�
  uint16 len=0;
	
	
	/***** GPIO��SPI��ʼ�� *****/
	GPIO_Configuration();			
	WIZ_SPI_Init();
		
	
	
	/***** Ӳ����W5500 *****/
	Reset_W5500();
	
	/***** W5500��IP��Ϣ��ʼ�� *****/
	set_default(); 														// ����Ĭ��MAC��IP��GW��SUB��DNS
	set_network();														// ���ó�ʼ��IP��Ϣ����ӡ����ʼ��8��Socket
	
	/***** ��W5500��Keepalive���� *****/
	//setkeepalive(0);
	
	printf("TCP Server IP: %d.%d.%d.%d \r\n",server_ip[0],server_ip[1],server_ip[2],server_ip[3]);
  printf("TCP Server Port: %d \r\n",server_port);	
	printf("W5500 Init Complete!\r\n");
  printf("Start TCP Client Test!\r\n"); 
  
	/*Socket״̬����MCUͨ����Sn_SR(0)��ֵ�����ж�SocketӦ�ô��ں���״̬
		Sn_SR״̬������
		0x00		SOCK_CLOSED
		0x13		SOCK_INIT
		0x14		SOCK_LISTEN
		0x17		SOCK_ESTABLISHED
		0x1C		SOCK_CLOSE_WAIT
		0x22		SOCK_UDP
	*/
	while(1)																				// Socket״̬��
	{	
		switch(getSn_SR(0))														// ��ȡsocket0��״̬
		{
			case SOCK_INIT:															// Socket���ڳ�ʼ�����(��)״̬
					connect(0, server_ip,server_port);			// ����Sn_CRΪCONNECT������TCP������������������
			break;
			case SOCK_ESTABLISHED:											// Socket�������ӽ���״̬
					if(getSn_IR(0) & Sn_IR_CON)   					
					{
						setSn_IR(0, Sn_IR_CON);								// Sn_IR��CONλ��1��֪ͨW5500�����ѽ���
					}
					// ���ݻػ����Գ������ݴ���λ������������W5500��W5500���յ����ݺ��ٻظ�������
					len=getSn_RX_RSR(0);										// len=Socket0���ջ������ѽ��պͱ�������ݴ�С
					if(len>0)
					{
						recv(0,buffer,len);										// W5500�������Է����������ݣ���ͨ��SPI���͸�MCU
						printf("%s\r\n",buffer);							// ���ڴ�ӡ���յ�������
						send(0,buffer,len);										// ���յ����ݺ��ٻظ���������������ݻػ�
					}
					// W5500�Ӵ��ڷ����ݸ��ͻ��˳����������Իس�����
					if(USART_RX_STA & 0x8000)								// �жϴ��������Ƿ�������
					{					   
						len=USART_RX_STA & 0x3fff;						// ��ȡ���ڽ��յ����ݵĳ���
						send(0,USART_RX_BUF,len);							// W5500��ͻ��˷�������
						USART_RX_STA=0;												// ���ڽ���״̬��־λ��0
						memset(USART_RX_BUF,0,len+1);					// ���ڽ��ջ�����0
					}
			break;
			case SOCK_CLOSE_WAIT:												// Socket���ڵȴ��ر�״̬
					close(0);																// �ر�Socket0
			break;
			case SOCK_CLOSED:														// Socket���ڹر�״̬
					socket(0,Sn_MR_TCP,local_port,Sn_MR_ND);		// ��Socket0��������ΪTCP����ʱģʽ����һ�����ض˿�
			break;
		}
	}
}
