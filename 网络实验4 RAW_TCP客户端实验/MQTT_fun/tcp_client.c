#include "tcp_client.h"
#include "device.h"
#include "spi2.h"
#include "ult.h"
#include "socket.h"
#include "w5500.h"
#include <string.h>
#include <stdio.h>
#include "mqtt_fun.h"
#include "MQTTPacket.h"
#include "cJSON.h"

  uint8 mqtt_server_ip[4] = {139,196,135,135};			// ����Զ�̷�����IP��ַ
	uint16 server_port=1883;								// ����Զ�̷������˿�
  uint16 local_port=1883;									// ��ʼ��һ�����ض˿�
  uint16 len=0;
	uint8  msgbuf[1024];
	uint8 CONNECT_FLAG=1;
	uint8 SUB_FLAG = 1;
	uint8 PUB_FLAG = 1;
	char ser_cmd[240];
/**************************/
	
	unsigned char dup;
	int qos;
	unsigned char retained;
	unsigned short mssageid;
	int payloadlen_in;
	unsigned char* payload_in;
	MQTTString receivedTopic;
	char topic[100]={"/TKKMt4nMF8U/MQTT1/mqtt"};//���÷�����������
	char new_topic[100];
	int count=0;
	char rebuf[1024];
	void hamacsha1(void);
	
void MQTT_CON_ALI(void)
{
	int len;
   switch(getSn_SR(0))										// ��ȡsocket0��״̬
   {
	    
	  case SOCK_INIT:			                  // Socket���ڳ�ʼ�����(��)״̬
		 connect(0, mqtt_server_ip,server_port);		// ����Sn_CRΪCONNECT������TCP������������������
		 printf("connecting.......\r\n");	
		 break;
	  
		 case SOCK_ESTABLISHED:								// Socket�������ӽ���״̬
					if(getSn_IR(0) & Sn_IR_CON)   					
					{
						setSn_IR(0, Sn_IR_CON);				// Sn_IR��CONλ��1��֪ͨW5500�����ѽ���
					}	
				memset(msgbuf,0,sizeof(msgbuf));
        if((len=getSn_RX_RSR(0))==0)
        {					
			  if(1==CONNECT_FLAG)
				{			
					
					/*MQTTƴ�����ӱ���
					*���ݰ�����ƽ̨MQTT�豸�����ֲ�����
					*/					
					//void make_con_msg(char* clientID,int keepalive, uint8 cleansession,char*username,char* password,unsigned char*buf,int buflen)
					
					make_con_msg("stm32|securemode=3,signmethod=hmacsha1|",180	, 1,
					"FY-STM32&a1ZCY7ACE5k","CD22A5844A0721998804B5EC6916DB96FE683180",msgbuf,sizeof(msgbuf));
					
					
					
					CONNECT_FLAG = 0;
					send(0,msgbuf,sizeof(msgbuf));
					printf("mqtt%4s\r\n",msgbuf);
					Delay_s(2);	
					while((len=getSn_RX_RSR(0))==0)
					{
					 Delay_s(2);	
					 send(0,msgbuf,117);
					};
					 recv(0,msgbuf,len);
					while(mqtt_decode_msg(msgbuf)!=CONNACK)//�ж��ǲ���CONNACK
					{
					  printf("wait ack\r\n");
					}
				}else if(SUB_FLAG == 1)
         {	
           memset(msgbuf,0,sizeof(msgbuf));					 
		        make_sub_msg(topic,msgbuf,sizeof(msgbuf));
					// make_pub_msg(topic,msgbuf,sizeof(msgbuf),"hello");
					 send(0,msgbuf,sizeof(msgbuf));// ���յ����ݺ��ٻظ���������������ݻػ�
					 SUB_FLAG = 0;
					  Delay_s(2);
					 while((len=getSn_RX_RSR(0))==0)
					 {
						 Delay_s(2);	
						  send(0,msgbuf,sizeof(msgbuf));
					  };
					 recv(0,msgbuf,len);
						while(mqtt_decode_msg(msgbuf)!=SUBACK)//�ж��ǲ���SUBACK
					{
					  printf("wait suback\r\n");
					}
					  TIM_Cmd(TIM2, ENABLE);
					  printf("send sub\r\n");	
					
					}
				 #if 1
				 else
				 {
					 //count++;
					 // Delay_s(2);
					 if(count>10000)
					 {
            count = 0;						
				    make_ping_msg(msgbuf,sizeof(msgbuf));
					  send(0,msgbuf,sizeof(msgbuf));	
	 
					 while((len=getSn_RX_RSR(0))==0)
					 {
						  //Delay_s(2);	
						  //send(0,msgbuf,sizeof(msgbuf));
						 printf("wait pingresponse");
						
					  };
					  recv(0,msgbuf,len);
						printf("ping len : %d\r\n",len);
						if(len>2)
						{
						  if(PUBLISH==mqtt_decode_msg(msgbuf+2))
							{
							  printf("publish\r\n");
						    MQTTDeserialize_publish(&dup, &qos, &retained, &mssageid, &receivedTopic,&payload_in, &payloadlen_in, msgbuf+2, len-2);
						  // printf("message arrived %d: %s\n\r", payloadlen_in, payload_in);
							 memset(topic,0,sizeof(topic));
							 memset(ser_cmd,0,sizeof(ser_cmd));
						   memcpy(topic,receivedTopic.lenstring.data,receivedTopic.lenstring.len);
							 replace_string(new_topic,topic , "request","response");
							 printf("topic:%s\r\n",topic);
			         strcpy(ser_cmd,(const char *)payload_in);
						  //parse_topic(ser_cmd);
			        // printf("message is %s\r\n",ser_cmd);
							 memset(msgbuf,0,sizeof(msgbuf));
							 make_pub_msg(new_topic,msgbuf,sizeof(msgbuf),"hello");
							 send(0,msgbuf,sizeof(msgbuf));	
							}
						}
					}
				 }
				 #endif
				 #if 0
				  if(PUB_FLAG==1)
				 {
				    memset(msgbuf,0,sizeof(msgbuf));					 
		       // make_sub_msg(topic,msgbuf,sizeof(msgbuf));
					 make_pub_msg(topic,msgbuf,sizeof(msgbuf),"hello");
					 if(count == 10000)
					 {	 
						 PUB:				 
					 send(0,msgbuf,sizeof(msgbuf));	// ���յ����ݺ��ٻظ���������������ݻػ�
					
					  Delay_s(2);
					// while((len=getSn_RX_RSR(0))==0)
					//  {
					// Delay_s(2);	
					//send(0,msgbuf,sizeof(msgbuf));
					//		printf("puback\r\n");
					//  };
					// recv(0,msgbuf,len);
				  //	if(mqtt_decode_msg(msgbuf)!=PUBACK)//�ж��ǲ���SUBACK
					//  {
					//			goto PUB;
					//	    printf("wait Puback\r\n");
					//	}
					  printf("send Pub\r\n");	
				  }
				 }	
#endif				 
			 } 
				 #if 1
				  if((len=getSn_RX_RSR(0))>0)
					{
					   recv(0,msgbuf,len);
						 if(PUBLISH== mqtt_decode_msg(msgbuf))
						 {	 
							printf("publish\r\n");
						  MQTTDeserialize_publish(&dup, &qos, &retained, &mssageid, &receivedTopic,&payload_in, &payloadlen_in, msgbuf, len);
						  // printf("message arrived %d: %s\n\r", payloadlen_in, payload_in);
							memset(topic,0,sizeof(topic));
						  memcpy(topic,receivedTopic.lenstring.data,receivedTopic.lenstring.len);
							replace_string(new_topic,topic , "request","response");
							 
							 printf("topic:%s\r\n",topic);
							 
							 memset(ser_cmd,0,sizeof(ser_cmd));
			         memcpy(ser_cmd,(const char *)payload_in,strlen((char*)payload_in));
							 memset(msgbuf,0,sizeof(msgbuf));
							 make_pub_msg(new_topic,msgbuf,sizeof(msgbuf),rebuf);
							 send(0,msgbuf,sizeof(msgbuf));	
						 //printf("%s\n",msgbuf);
						 }else if(PINGRESP== mqtt_decode_msg(msgbuf))
						 {	 	  
							 if(len>2)
							 {
							     if(PUBLISH==mqtt_decode_msg(msgbuf+2))
							{
							  printf("publish\r\n");
						    MQTTDeserialize_publish(&dup, &qos, &retained, &mssageid, &receivedTopic,&payload_in, &payloadlen_in, msgbuf+2, len-2);
						  // printf("message arrived %d: %s\n\r", payloadlen_in, payload_in);
							  memset(topic,0,sizeof(topic));
						   memcpy(topic,receivedTopic.lenstring.data,receivedTopic.lenstring.len);
							 replace_string(new_topic,topic,"request","response");
							 printf("topic:%s\r\n",topic);
						  	memset(ser_cmd,0,sizeof(ser_cmd));
			         strcpy(ser_cmd,(const char *)payload_in);
			        // printf("message is %s\r\n",ser_cmd);
					 	   //parse_topic(ser_cmd);
							 memset(msgbuf,0,sizeof(msgbuf));
							 make_pub_msg(new_topic,msgbuf,sizeof(msgbuf),"hello");
							 send(0,msgbuf,sizeof(msgbuf));	
							}
							 }
							 
						 }else
						 {
						   printf("wait publish\r\n");
						 }
					}
				//	printf("send ping\r\n");
				  
				 #endif	
			break;
			case SOCK_CLOSE_WAIT:												// Socket���ڵȴ��ر�״̬
					close(0);																// �ر�Socket0
			break;
			case SOCK_CLOSED:														// Socket���ڹر�״̬
					socket(0,Sn_MR_TCP,local_port,Sn_MR_ND);// ��Socket0��������ΪTCP����ʱģʽ����һ�����ض˿�
			break;
		}

}

/********�ַ����滻********/
void replace_string(char *result, char *source, char* s1, char *s2)
{
char *q=NULL;
char *p=NULL;
p=source;
while((q=strstr(p, s1))!=NULL)
{
strncpy(result, p, q-p);
result[q-p]= '\0';//very important, must attention!
strcat(result, s2);
strcat(result, q+strlen(s1));
strcpy(p,result);
}
strcpy(result, p);
}
