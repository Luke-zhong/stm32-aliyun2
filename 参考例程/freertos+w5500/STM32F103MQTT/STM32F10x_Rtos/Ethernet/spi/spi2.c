#include "stm32f10x.h"
#include "spi2.h"
#include "stm32f10x_conf.h"
#include "socket.h"
#include "w5500.h"


void WIZ_SPI_Init(void)
{
	SPI_InitTypeDef   SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB |RCC_APB2Periph_AFIO , ENABLE);	
  // Port B output
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_SetBits(GPIOB, GPIO_Pin_12);
  /* Configure SPIy pins: SCK, MISO and MOSI */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	  /* SPI Config -------------------------------------------------------------*/
	  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	  SPI_InitStructure.SPI_CRCPolynomial = 7;

	  SPI_Init(SPI2, &SPI_InitStructure);
	  SPI_Cmd(SPI2, ENABLE);
}

// Connected to Data Flash
void WIZ_CS(uint8_t val)
{
	if (val == LOW) 
	{
   		GPIO_ResetBits(GPIOB, GPIO_Pin_12); 
	}
	else if (val == HIGH)
	{
   		GPIO_SetBits(GPIOB, GPIO_Pin_12); 
	}
}

uint8_t SPI2_SendByte(uint8_t byte)
{
	uint16_t err_count = 0;
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
	{
		err_count ++;
		if(err_count>3000)
		{
			LOG_E("ERROR:ethernet spi TXE timeout.\r\n");
			return 0;	
		}
	}
	err_count = 0;
	SPI_I2S_SendData(SPI2, byte);

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
	{
		err_count ++;
		if(err_count>3000)
		{
			LOG_E("ERROR:ethernet spi RXE timeout.\r\n");
			return 0;
		}
	}
	return SPI_I2S_ReceiveData(SPI2);
}


