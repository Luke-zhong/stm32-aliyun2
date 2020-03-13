#ifndef __SPI2_H
#define __SPI2_H

#include "stm32f10x.h"

#define LOW 0
#define HIGH 1

void WIZ_SPI_Init(void);
void WIZ_CS(uint8_t val);
uint8_t SPI2_SendByte(uint8_t byte);
#endif

