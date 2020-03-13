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
	LOG_LEVEL_NONE=1,	//none log
	LOG_LEVEL_ERROR,	//only show error log
	LOG_LEVEL_WARING,	//show waring log above
	LOG_LEVEL_INFO,		//show info log above
	LOG_LEVEL_DEBUG,	//shoe debug log above
	LOG_LEVEL_VERBOSE  //show verbose log
}LOG_LEVEL_TypeDef;

#define LOG_LEVEL			LOG_LEVEL_VERBOSE

#define LOG_E				if(LOG_LEVEL>=LOG_LEVEL_ERROR)printf
#define LOG_W				if(LOG_LEVEL>=LOG_LEVEL_WARING)printf
#define LOG_I				if(LOG_LEVEL>=LOG_LEVEL_INFO)printf
#define LOG_D				if(LOG_LEVEL>=LOG_LEVEL_DEBUG)printf
#define LOG_V				if(LOG_LEVEL>=LOG_LEVEL_VERBOSE)printf

void LogUsartInit(void);

#endif

