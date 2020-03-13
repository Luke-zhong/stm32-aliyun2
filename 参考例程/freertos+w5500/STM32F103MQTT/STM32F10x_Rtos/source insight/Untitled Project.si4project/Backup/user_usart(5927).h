#ifndef __USER_USART_H
#define __USER_USART_H

#define LOG_USART_TX_GPIO_PORT			GPIOA
#define LOG_USART_TX_GPIO_PIN			GPIO_Pin_9
#define LOG_USART_TX_GPIO_CLK			RCC_APB2Periph_GPIOA
#define LOG_USART_RX_GPIO_PORT			GPIOA
#define LOG_USART_RX_GPIO_PIN			GPIO_Pin_10
#define	LOG_USART_RX_GPIO_CLK			RCC_APB2Periph_GPIOA

#define LOG_USART						USART1
#define LOG_USART_BAUDRATE				115200

typedef enum{
	LOG_LEVEL_OFF=0,	//none log
	LOG_LEVEL_ERROR,	//only show error log
	LOG_LEVEL_WARING,	//show waring log above
	LOG_LEVEL_INFO,		//show info log above
	LOG_LEVEL_DEBUG,	//shoe debug log above
	LOG_LEVEL_VERBOSE  //show verbose log
}LOG_LEVEL_TypeDef;

#define LOG_LEVEL			LOG_LEVEL_VERBOSE

#if LOG_LEVEL	>= LOG_LEVEL_ERROR
	#define	LOG_E	printf
#endif

#if LOG_LEVEL	>= LOG_LEVEL_WARING
	#define	LOG_W	printf
#endif

#if LOG_LEVEL	>= LOG_LEVEL_INFO
	#define	LOG_I	printf
#endif

#if LOG_LEVEL	>= LOG_LEVEL_DEBUG
	#define	LOG_D	printf
#endif

#if LOG_LEVEL	>= LOG_LEVEL_VERBOSE
	#define	LOG_V	printf
#endif



void LogUsartInit(void);

#endif

