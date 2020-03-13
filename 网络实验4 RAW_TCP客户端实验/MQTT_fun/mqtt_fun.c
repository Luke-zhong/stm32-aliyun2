#include "mqtt_fun.h"
#include "socket.h"
#include "usart.h"
#include "w5500.h"
#include "ult.h"
#include "device.h"
#include "types.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "MQTTPacket.h"
#include "StackTrace.h"
extern int MQTTSerialize_zero(unsigned char* buf, int buflen, unsigned char packettype);
extern uint8_t server_ip[4];//mqtt服务器的ip地址

/***************发送消息的相关函数************************/

/**
* @brief  通过TCP方式发送数据到TCP服务器
* @param  buf数据首地址
* @param  buflen数据长度
* @retval 小于0表示发送失败
*/

/*订阅消息*/
int Subscribe_sendPacketBuffer(unsigned char* buf, int buflen)
{
  return send(SOCK_TCPS,buf,buflen);
}

/*发布消息*/
int Published_sendPacketBuffer(unsigned char* buf, int buflen)
{
  return send(SOCK_TCPC,buf,buflen);
}

/**
* @brief  阻塞方式接受TCP服务器发送的数据
* @param  buf数据存储首地址
* @param  count数据缓冲区长度
* @retval 小于0表示接收数据失败
*/
int Subscribe_getdata(unsigned char* buf, int count)
{
	
     return recv(SOCK_TCPS,buf,count);
	
}

int Published_getdata(unsigned char* buf, int count)
{
  return recv(SOCK_TCPC,buf,count);
	
}

/**
* @brief  打开一个socket并连接到服务器
* @param  无
* @retval 小于0表示打开失败
*/
int Subscribe_open(void)
{
  int32_t ret;
		//新建一个Socket并绑定本地端口5000
  ret = socket(SOCK_TCPS,Sn_MR_TCP,50000,0x00);
  if(ret != 1){
    printf("%d:Socket Error\r\n",SOCK_TCPS);
    while(1);
  }else{
    printf("%d:Opened\r\n",SOCK_TCPS);
  }
	

	while(getSn_SR(SOCK_TCPS)!=SOCK_ESTABLISHED)
	{
		 printf("connecting\r\n");
			//连接TCP服务器
		 ret = connect(SOCK_TCPS,server_ip,1883);//端口必须为1883
	}
  if(ret != 1){
    printf("%d:Socket Connect Error\r\n",SOCK_TCPS);
    while(1);
  }else{
    printf("%d:Connected\r\n",SOCK_TCPS);
  }		
	return 0;
}

int Published_open(void)
{
  int32_t ret;
  
  ret = socket(SOCK_TCPC,Sn_MR_TCP,5001,0x00);
	
  if(ret != 1){
    printf("%d:Socket1 Error1\r\n",SOCK_TCPC);
    while(1);
  }else{
    printf("%d:socket1 Opened\r\n",SOCK_TCPC);
  }

  
	while(getSn_SR(SOCK_TCPC)!=SOCK_ESTABLISHED)
	{
		 ret = connect(SOCK_TCPC,server_ip,1883);//端口必须为1883
	}
  if(ret != 1){
    printf("%d:Socket Connect1 Error\r\n",SOCK_TCPC);
    while(1);
  }else{
    printf("%d:Connected1\r\n",SOCK_TCPC);
  }		
	return 0;
}

/**
* @brief  关闭socket
* @param  无
* @retval 小于0表示关闭失败
*/
int Subscribe_close(void)
{
 disconnect(SOCK_TCPS);
	printf("close0\n\r");

	while(getSn_SR(SOCK_TCPC)!=SOCK_CLOSED)
	{
		;
	}
  return 0;
}


int Published_close(void)
{
  disconnect(SOCK_TCPC);
	printf("close1\n\r");

	while(getSn_SR(SOCK_TCPC)!=SOCK_CLOSED)
	{
		;
	}
  return 0;
}


/*****************拼接连接报文**********************/
//MQTT连接函数
/************************************************************************
*
*password的拼接方式
*1.使用阿里云的密码生成工具，在项目文件夹可见
*2.手动拼接：
*a:拼接参数-----------提交给服务器的clientId、deviceName、
*  productKey和timestamp（timestamp为非必选参数）参数及参数值依次拼接。
*  本例中拼接结果为：clientId12345deviceNamedeviceproductKeyalxxxxxxxxx
*b:加密------------通过Client ID中确定的加密方法，使用设备deviceSecret，
*  将拼接结果加密。假设设备的deviceSecret值为abc123，加密计算格式为
*  hmacsha1(abc123,clientId12345deviceNamedeviceproductKeyalxxxxxxxxxx)
*
************************************************************************/
void make_con_msg(char* clientID,int keepalive, uint8 cleansession,char*username,char* password,unsigned char*buf,int buflen)
{
	int32_t len;
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.clientID.cstring = "stm32|securemode=3,signmethod=hmacsha1|";//clientID;
	data.keepAliveInterval = 180;//keepalive;
	data.cleansession = 1;//cleansession;
	data.username.cstring = "FY-STM32&a1ZCY7ACE5k";//username;
	data.password.cstring = "CD22A5844A0721998804B5EC6916DB96FE683180";//password;
	len = MQTTSerialize_connect(buf, buflen, &data);//构造链接报文
	return;
}
/*****************拼接订阅报文**************************************/
void make_sub_msg(char *Topic,unsigned char*msgbuf,int buflen)
{
	int msgid = 1;
	int req_qos = 0;
	unsigned char topic[100];
    MQTTString topicString= MQTTString_initializer;
	memcpy(topic,Topic,strlen(Topic));
	topicString.cstring = (char*)topic;
	//topicString.lenstring.len=4;
	MQTTSerialize_subscribe(msgbuf, buflen, 0, msgid, 1, &topicString, &req_qos);
	return;
}
/*********拼接发布报文******************/
void make_pub_msg(char *Topic,unsigned char*msgbuf,int buflen,char*msg)
{
	unsigned char topic[100];
	int msglen = strlen(msg);
	MQTTString topicString = MQTTString_initializer;
	memset(topic,0,sizeof(topic));
	memcpy(topic,Topic,strlen(Topic));
	topicString.cstring = (char*)topic;
	MQTTSerialize_publish(msgbuf, buflen, 0, 2, 0, 0, topicString, (unsigned char*)msg, msglen); 
	return;
}
/********拼接PING报文*********************/
int  make_ping_msg(unsigned char* buf,int buflen)
{
   return MQTTSerialize_zero(buf, buflen, PINGREQ);
}
/******解析收到的ACK报文*********/
int mqtt_decode_msg(unsigned char*buf)
{
	int rc = -1;
  MQTTHeader header = {0};
	header.byte = buf[0];
	rc = header.bits.type;
	return rc;
}
/****************拼接hamacsha1字符串**********************/
/**void hamacsha1(void)
{
	//MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	char buffer[2048];
	char mesbuf[2048];
	char *clientId = "192.168.207.115";
	char *deviceName = "MQTT1";
	char *productKey = "TKKMt4nMF8U";
	char *secret = "A2VmnqJab2XAZlntAxwjLxW0nNmOpHaQ";
	int timestap = 789;
	sprintf(buffer,"clientId%sdeviceName%sproductKey%stimestap%d",clientId,deviceName,productKey,timestap);
	printf("%s\r\n",buffer);

}**/

