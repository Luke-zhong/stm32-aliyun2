#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "stm32f10x.h"
#include"stdio.h"
#define DEVICE_ID "W5500"

#define FW_VER_HIGH  		1
#define FW_VER_LOW    	0

#define WIZ_SCLK				GPIO_Pin_13	// out
#define WIZ_MISO				GPIO_Pin_14	// in
#define WIZ_MOSI				GPIO_Pin_15	// out

#define WIZ_SCS			    GPIO_Pin_5	// out
#define WIZ_RESET		    GPIO_Pin_1 	// out
#define WIZ_INT			    GPIO_Pin_2	// in

typedef  void (*pFunction)(void);
void GPIO_Configuration(void);
void set_network(void);
void set_default(void);
void Reset_W5500(void);

void reboot(void);
void get_config(void);
#endif

