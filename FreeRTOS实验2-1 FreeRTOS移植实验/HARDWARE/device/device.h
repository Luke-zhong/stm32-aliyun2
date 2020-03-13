#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "stm32f1xx.h"
#include"stdio.h"
#define DEVICE_ID "W5500"

#define FW_VER_HIGH  		1
#define FW_VER_LOW    	0

#define WIZ_SCLK				GPIO_PIN_13	// out
#define WIZ_MISO				GPIO_PIN_14	// in
#define WIZ_MOSI				GPIO_PIN_15	// out

#define WIZ_SCS			    GPIO_PIN_5	// out
#define WIZ_RESET		    GPIO_PIN_1 	// out
#define WIZ_INT			    GPIO_PIN_2	// in

typedef  void (*pFunction)(void);
void GPIO_Configuration(void);
void set_network(void);
void set_default(void);
void Reset_W5500(void);

void reboot(void);
void get_config(void);
#endif

