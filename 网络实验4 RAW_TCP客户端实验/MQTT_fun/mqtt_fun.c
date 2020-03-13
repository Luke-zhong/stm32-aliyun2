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
extern uint8_t server_ip[4];//mqtt��������ip��ַ

/***************������Ϣ����غ���************************/

/**
* @brief  ͨ��TCP��ʽ�������ݵ�TCP������
* @param  buf�����׵�ַ
* @param  buflen���ݳ���
* @retval С��0��ʾ����ʧ��
*/

/*������Ϣ*/
int Subscribe_sendPacketBuffer(unsigned char* buf, int buflen)
{
  return send(SOCK_TCPS,buf,buflen);
}

/*������Ϣ*/
int Published_sendPacketBuffer(unsigned char* buf, int buflen)
{
  return send(SOCK_TCPC,buf,buflen);
}

/**
* @brief  ������ʽ����TCP���������͵�����
* @param  buf���ݴ洢�׵�ַ
* @param  count���ݻ���������
* @retval С��0��ʾ��������ʧ��
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
* @brief  ��һ��socket�����ӵ�������
* @param  ��
* @retval С��0��ʾ��ʧ��
*/
int Subscribe_open(void)
{
  int32_t ret;
		//�½�һ��Socket���󶨱��ض˿�5000
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
			//����TCP������
		 ret = connect(SOCK_TCPS,server_ip,1883);//�˿ڱ���Ϊ1883
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
		 ret = connect(SOCK_TCPC,server_ip,1883);//�˿ڱ���Ϊ1883
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
* @brief  �ر�socket
* @param  ��
* @retval С��0��ʾ�ر�ʧ��
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


/*****************ƴ�����ӱ���**********************/
//MQTT���Ӻ���
/************************************************************************
*
*password��ƴ�ӷ�ʽ
*1.ʹ�ð����Ƶ��������ɹ��ߣ�����Ŀ�ļ��пɼ�
*2.�ֶ�ƴ�ӣ�
*a:ƴ�Ӳ���-----------�ύ����������clientId��deviceName��
*  productKey��timestamp��timestampΪ�Ǳ�ѡ����������������ֵ����ƴ�ӡ�
*  ������ƴ�ӽ��Ϊ��clientId12345deviceNamedeviceproductKeyalxxxxxxxxx
*b:����------------ͨ��Client ID��ȷ���ļ��ܷ�����ʹ���豸deviceSecret��
*  ��ƴ�ӽ�����ܡ������豸��deviceSecretֵΪabc123�����ܼ����ʽΪ
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
	len = MQTTSerialize_connect(buf, buflen, &data);//�������ӱ���
	return;
}
/*****************ƴ�Ӷ��ı���**************************************/
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
/*********ƴ�ӷ�������******************/
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
/********ƴ��PING����*********************/
int  make_ping_msg(unsigned char* buf,int buflen)
{
   return MQTTSerialize_zero(buf, buflen, PINGREQ);
}
/******�����յ���ACK����*********/
int mqtt_decode_msg(unsigned char*buf)
{
	int rc = -1;
  MQTTHeader header = {0};
	header.byte = buf[0];
	rc = header.bits.type;
	return rc;
}
/****************ƴ��hamacsha1�ַ���**********************/
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

