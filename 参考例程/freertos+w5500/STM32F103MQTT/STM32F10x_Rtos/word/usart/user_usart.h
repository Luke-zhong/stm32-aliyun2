#ifndef __USER_USART_H
#define __USER_USART_H

extern QueueHandle_t log_queue_handle;


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
	#define	LOG_E(format,...)	\
			taskENTER_CRITICAL();printf("E:"format,##__VA_ARGS__);taskEXIT_CRITICAL();
#endif

#if LOG_LEVEL	>= LOG_LEVEL_WARING
	#define	LOG_W(format,...)	\
			taskENTER_CRITICAL();printf("W:"format,##__VA_ARGS__);taskEXIT_CRITICAL();
#endif

#if LOG_LEVEL	>= LOG_LEVEL_INFO
	#define	LOG_I(format,...)	\
			taskENTER_CRITICAL();printf("I:"format,##__VA_ARGS__);taskEXIT_CRITICAL();
#endif

#if LOG_LEVEL	>= LOG_LEVEL_DEBUG
	#define	LOG_D(format,...)	\
			taskENTER_CRITICAL();printf("D:"format,##__VA_ARGS__);taskEXIT_CRITICAL();
#endif

#if LOG_LEVEL	>= LOG_LEVEL_VERBOSE
	#define	LOG_V(format,...)	\
			taskENTER_CRITICAL();printf("V:"format,##__VA_ARGS__);taskEXIT_CRITICAL();
#endif



void LogUsartInit(void);

#endif

