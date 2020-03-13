#include "device.h"
#include "config.h"
#include "socket.h"
#include "ult.h"
#include "w5500.h"
#include <stdio.h> 
#include <string.h>

CONFIG_MSG  ConfigMsg, RecvMsg;
uint8 txsize[MAX_SOCK_NUM] = {4,2,2,2,2,2,2,0};
uint8 rxsize[MAX_SOCK_NUM] = {4,2,2,2,2,2,2,0};
extern uint8 MAC[6];//public buffer for DHCP, DNS, HTTP
uint8 pub_buf[1460];
uint8 mac[6]={0x00,0x08,0xdc,0x11,0x11,0x11};
uint8 lip[4]={192,168,0,10};
uint8 sub[4]={255,255,255,0};
uint8 gw[4]={192,168,0,1};
uint8 dns[4]={8,8,8,8};

void GPIO_Configuration(void)
{
	// 建立GPIO的初始化结构体
//  GPIO_InitTypeDef GPIO_InitStructure;
//	
//	// 开启GPIOE
//  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOE, ENABLE);

//  // PA0/1/2/3设置
//  GPIO_InitStructure.GPIO_Pin =GPIO_Pin_5; // 选定PB5
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		// 通信速度设为50MHz
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		// 选择推挽输出
//  GPIO_Init(GPIOB, &GPIO_InitStructure);							// 完成配置 
//  GPIO_SetBits(GPIOB, GPIO_Pin_5);		
//	
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;	   
//  GPIO_Init(GPIOE, &GPIO_InitStructure);	  	      			 

    GPIO_InitTypeDef GPIO_Initure;

  __HAL_RCC_GPIOB_CLK_ENABLE();           	//开启GPIOB时钟
  __HAL_RCC_GPIOE_CLK_ENABLE();           	//开启GPIOE时钟
	
  GPIO_Initure.Pin=GPIO_PIN_5; 				//PB5
  GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  	//推挽输出
  GPIO_Initure.Pull=GPIO_PULLUP;          	//上拉
  GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;    //高速
  HAL_GPIO_Init(GPIOB,&GPIO_Initure);

  GPIO_Initure.Pin=GPIO_PIN_1 | GPIO_PIN_2; 				//PE5
  HAL_GPIO_Init(GPIOE,&GPIO_Initure);
}

void Reset_W5500(void)
{
  HAL_GPIO_WritePin(GPIOB, WIZ_RESET,GPIO_PIN_RESET);
  Delay_us(2);  
  HAL_GPIO_WritePin(GPIOB, WIZ_RESET,GPIO_PIN_SET);
  Delay_ms(1600);
}

/*void reboot(void)
{
  pFunction Jump_To_Application;
  uint32 JumpAddress;
  JumpAddress = *(vu32*) (0x00000004);
  Jump_To_Application = (pFunction) JumpAddress;
  Jump_To_Application();
}*/

void set_network(void)
{
  setSHAR(ConfigMsg.mac);/*配置Mac地址*/
  setSUBR(ConfigMsg.sub);/*配置子网掩码*/
  setGAR(ConfigMsg.gw);/*配置默认网关*/
  setSIPR(ConfigMsg.lip);/*配置Ip地址*/

  sysinit(txsize, rxsize); /*初始化8个socket*/
  
  setRTR(2000);/*设置溢出时间值*/
  setRCR(3);/*设置最大重新发送次数*/
  
  getSIPR (lip);
  printf(" IP : %d.%d.%d.%d\r\n", lip[0],lip[1],lip[2],lip[3]);
  getSUBR(sub);
  printf(" SN : %d.%d.%d.%d\r\n", sub[0],sub[1],sub[2],sub[3]);
  getGAR(gw);
  printf(" GW : %d.%d.%d.%d\r\n", gw[0],gw[1],gw[2],gw[3]); 
}

void set_default(void)
{  
  memcpy(ConfigMsg.lip, lip, 4);
  memcpy(ConfigMsg.sub, sub, 4);
  memcpy(ConfigMsg.gw,  gw, 4);
  memcpy(ConfigMsg.mac, mac,6);
  memcpy(ConfigMsg.dns,dns,4);

  ConfigMsg.dhcp=0;
  ConfigMsg.debug=1;
  ConfigMsg.fw_len=0;
  
  ConfigMsg.state=NORMAL_STATE;
  ConfigMsg.sw_ver[0]=FW_VER_HIGH;
  ConfigMsg.sw_ver[1]=FW_VER_LOW;  
}
/*void write_config_to_eeprom(void)
{
  uint8 data;
  uint16 i,j;
  uint16 dAddr=0;
  for (i = 0, j = 0; i < (uint8)(sizeof(ConfigMsg)-4);i++) 
  {
    data = *(uint8 *)(ConfigMsg.mac+j);
    at24c16_write(dAddr, data);
    dAddr += 1;
    j +=1;
  }
}*/
/*void get_config(void)
{
  uint8 tmp=0;
  uint16 i;
  for (i =0; i < CONFIG_MSG_LEN; i++) 
  {
    tmp=at24c16_read(i);
    *(ConfigMsg.mac+i) = tmp;
  }
  if((ConfigMsg.mac[0]==0xff)&&(ConfigMsg.mac[1]==0xff)&&(ConfigMsg.mac[2]==0xff)&&(ConfigMsg.mac[3]==0xff)&&(ConfigMsg.mac[4]==0xff)&&(ConfigMsg.mac[5]==0xff))
  {
    set_default();
  }
}*/
