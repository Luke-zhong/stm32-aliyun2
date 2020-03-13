#include "stm32f10x_conf.h"

void LogUsartInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,  ENABLE);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin = LOG_USART_TX_GPIO_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LOG_USART_TX_GPIO_PORT,&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Pin = LOG_USART_RX_GPIO_PIN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LOG_USART_RX_GPIO_PORT,&GPIO_InitStruct);
	
	USART_InitStruct.USART_BaudRate = LOG_USART_BAUDRATE;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_Init(LOG_USART,&USART_InitStruct);
	USART_Cmd(LOG_USART,ENABLE);
}

int fputc(int ch,FILE *f)
{
	while(USART_GetFlagStatus(LOG_USART, USART_FLAG_TC) == RESET);
	USART_SendData(LOG_USART, (unsigned char)ch);
	return ch;
}
	
