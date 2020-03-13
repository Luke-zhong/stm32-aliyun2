#include "spi.h"
#include "config.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK精英STM32开发板
//SPI驱动 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2015/4/27
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
 
//以下是SPI模块的初始化代码，配置成主机模式，访问SD Card/W25Q64/NRF24L01						  
//SPI口初始化
//这里针是对SPI2的初始化
SPI_HandleTypeDef SPI2_Handler;

void WIZ_SPI_Init(void)
{
// 	GPIO_InitTypeDef GPIO_InitStructure;
//  	SPI_InitTypeDef  SPI_InitStructure;

//	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );//PORTB时钟使能 
//	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_SPI2,  ENABLE );//SPI2时钟使能 	
//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;				 //PB12上拉 防止W25X的干扰
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);					//初始化指定IO
// 	GPIO_SetBits(GPIOB,GPIO_Pin_12);						//上拉				
// 	
// 
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //PB13/14/15复用推挽输出 
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB

// 	GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);  //PB13/14/15上拉

//	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
//	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
//	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
//	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//串行同步时钟的空闲状态为高电平
//	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
//	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
//	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;		//定义波特率预分频的值:波特率预分频值为256
//	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
//	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
//	
//	SPI_Init(SPI2, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
//	SPI_Cmd(SPI2, ENABLE); //使能SPI外设
//	//SPI2_SendByte(0xff);//启动传输		

  SPI2_Handler.Instance = SPI2;
  SPI2_Handler.Init.Mode = SPI_MODE_MASTER;
  SPI2_Handler.Init.Direction = SPI_DIRECTION_2LINES;
  SPI2_Handler.Init.DataSize = SPI_DATASIZE_8BIT;
  SPI2_Handler.Init.CLKPolarity = SPI_POLARITY_HIGH;
  SPI2_Handler.Init.CLKPhase = SPI_PHASE_2EDGE;
  SPI2_Handler.Init.NSS        = SPI_NSS_SOFT;
  SPI2_Handler.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  SPI2_Handler.Init.FirstBit = SPI_FIRSTBIT_MSB;
  SPI2_Handler.Init.TIMode = SPI_TIMODE_DISABLE;
  SPI2_Handler.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  SPI2_Handler.Init.CRCPolynomial = 7;

  __HAL_SPI_ENABLE(&SPI2_Handler);
		 
 

}   

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
   GPIO_InitTypeDef GPIO_Initure;

  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_SPI2_CLK_ENABLE();

  GPIO_Initure.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_Initure.Mode = GPIO_MODE_AF_PP;
  GPIO_Initure.Pull = GPIO_PULLUP;
  GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB,&GPIO_Initure);
}

//SPI 速度设置函数
//SpeedSet:
//SPI_BaudRatePrescaler_2   2分频   
//SPI_BaudRatePrescaler_8   8分频   
//SPI_BaudRatePrescaler_16  16分频  
//SPI_BaudRatePrescaler_256 256分频 
  
void SPI2_SetSpeed(u8 SPI_BaudRatePrescaler)
{
  	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));//判断有效性
    __HAL_SPI_DISABLE(&SPI2_Handler);            //关闭SPI
    SPI2_Handler.Instance->CR1&=0XFFC7;          //位3-5清零，用来设置波特率
    SPI2_Handler.Instance->CR1|=SPI_BaudRatePrescaler;//设置SPI速度
    __HAL_SPI_ENABLE(&SPI2_Handler);             //使能SPI

} 

void WIZ_CS(uint8_t val)
{
	if (val == LOW) 
	{
   		HAL_GPIO_WritePin(GPIOB, WIZ_SCS,GPIO_PIN_RESET); 	// GPIOB的WIZ_SCS（GPIO_Pin_12）引脚清零拉低
	}
	else if (val == HIGH)
	{
   		HAL_GPIO_WritePin(GPIOB, WIZ_SCS,GPIO_PIN_RESET); 		// GPIOB的WIZ_SCS（GPIO_Pin_12）引脚置1拉高
	}
}

//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPI2_SendByte(u8 TxData)
{		
  
    u8 Rxdata;
  
  HAL_SPI_TransmitReceive(&SPI2_Handler,&TxData,&Rxdata,1,1000);
 
  return Rxdata;	
}































