#include "stm32f10x_conf.h"
#include "user_mqtt.h"
#include "socket.h"
//#include "MQTTConnect.h"
#include "MQTTPacket.h"
/*
	brief: include DNS,connect mqtt,subscribe��publish��and receive server data.
	parameter: pvParameter
	return : NONE
*/
MQTTPacket_connectData user_MQTTPacket_ConnectData;
uint8_t mqtt_buff[200];
static int transport_getdata(uint8_t *buf,int count)
{
	return recv(MQTT_SOCKET, buf, count);
}
void user_mqtt_init(void)
{
	user_MQTTPacket_ConnectData.cleansession = 1;
	user_MQTTPacket_ConnectData.keepAliveInterval = 20;
	user_MQTTPacket_ConnectData.MQTTVersion = 3; //mqtt version
	user_MQTTPacket_ConnectData.struct_version = 0; //must be 0
	user_MQTTPacket_ConnectData.username.cstring = MQTT_USER;
	user_MQTTPacket_ConnectData.password.cstring = MQTT_PASS;
	user_MQTTPacket_ConnectData.clientID.cstring = MQTT_CLIEND_ID;
}
uint8_t mqtt_remote_ip[4];
/*mqtt���Ӻ���
	��������
	���أ�1�����ӳɹ���0����ʧ��
*/
static uint8_t mqtt_connect()
{
	uint8_t SessionFlg,ConnAckFlg;
	uint8_t count,ret;
	int rc;
	while(1)
	{
		rc=MQTTSerialize_connect(mqtt_buff, MQTT_BUFFLEN, &user_MQTTPacket_ConnectData);
		if(rc == 0){
			LOG_E("MQTT> serialize connect packet error: %d.\r\n",rc);
			return 0;
		}
		LOG_I("MQTT> serialize packet length : %d.\r\n",rc);
		ret=send(MQTT_SOCKET, mqtt_buff, rc);
		if(ret != rc){
			LOG_E("MQTT> send error:%d.rc:%d.\r\n",ret,rc);
			return 0;
		}
		//�ȴ�Ӧ���ź�
		do{
			while(MQTTPacket_read(mqtt_buff, MQTT_BUFFLEN, transport_getdata) != CONNACK)
			{
				vTaskDelay(500);
				count++;
				if(count > 10)
				{
					count = 0;
					LOG_E("MQTT> connect timerout.\r\n");
				}
			}
		}
		while((MQTTDeserialize_connack(&SessionFlg,&ConnAckFlg,mqtt_buff,MQTT_BUFFLEN)!=1 \
																		|| ConnAckFlg!=0));
		return 1;
	}	
}
/*mqtt��������
	����������
	���أ�1���ɹ�,0:ʧ��
*/
static uint8_t mqtt_subscribe(char *sub,int qos)
{
	unsigned short submsgid;
	int subcount;
	int granted_qos;

	uint8_t dup=0;
	uint8_t ret,count;
	int rc;
	MQTTString topic;
	topic.cstring = sub;
	while(1)
	{
		rc = MQTTSerialize_subscribe(mqtt_buff,MQTT_BUFFLEN,  dup,0,1,&topic,0);
		if( rc < 0)
		{
			LOG_E("MQTT> serialize subscribe error.\r\n");
			return 0;
		}
		ret = send(MQTT_SOCKET,mqtt_buff,rc);
		if(ret != rc)
		{
			LOG_E("MQTT> send subscribe error.\r\n");	
		}
		do{
			while(MQTTPacket_read(mqtt_buff,MQTT_BUFFLEN,transport_getdata) != SUBACK)
			{
				vTaskDelay(500);
				if(++count > 10){
					count =0 ;
					LOG_E("MQTT> wait suback timerout.\r\n");
					return 0;
				}
			}
			LOG_D("MQTT> MQTTPacket read SUBACK.\r\n");
			rc = MQTTDeserialize_suback(&submsgid,1,&subcount,&granted_qos,mqtt_buff,MQTT_BUFFLEN);
		}
		while(granted_qos != 0);
		return 1;
	}
}
/*
	MQTT ����ping����
	��������
	���أ�1�����ͳɹ���0������ʧ��
*/
static int mqtt_ping()
{
	int rc;
	uint8_t ret;
	while(1)
	{
		rc = MQTTSerialize_pingreq(mqtt_buff, MQTT_BUFFLEN);
		ret = send(MQTT_SOCKET,mqtt_buff,rc);
		if(rc != ret)
		{
			LOG_E("MQTT> ping send error.\r\n");
			return 0;
		}
		return 1;
	}
}
/*mqtt�������񣬵�tcp�ɹ���������ʱ���ָ������񣬷����ǹ���״̬
��LINK�Ͽ�ʱ�����������
����������TCP�ɹ����ӵĻ����ϣ�����mqtt������->�����������->
���أ�����������������쳣���ش�������
*/
static uint8_t mqtt() 
{
	uint8_t *payload_in;
	uint8_t ret;
	uint8_t dup,retained;
	uint16_t mssageid;
	int qos,rc,payloadlen_in;
	MQTTString topoc;
	//topoc.cstring = "fdj/iot/control/12345";
	if( mqtt_connect() )
	{
		LOG_I("MQTT> connected.\r\n");
		if( mqtt_subscribe("subscribe_test",0))
		{
			LOG_I("MQTT> subscribe success.\r\n");
			while(1) //��ʼ�������ݣ���������
			{
				if(!mqtt_ping())
				{
					LOG_E("MQTT> ping error.\r\n");
					return 0;
				}
				if(MQTTPacket_read(mqtt_buff,MQTT_BUFFLEN, transport_getdata) == PUBLISH)
				{
					rc = MQTTDeserialize_publish(&dup, &qos, &retained, &mssageid, &topoc,\
						&payload_in, &payloadlen_in, mqtt_buff, MQTT_BUFFLEN);
					printf("message arrived %d: %s\n\r", payloadlen_in, payload_in);
				}
			}
			vTaskDelay(1000);
		}
		else
		{
			LOG_E("MQTT> subscribe failed.\r\n");
		}
		LOG_I("-----------test end---------------\r\n");
		while(1);
	}
	else
	{
		LOG_E("MQTT> connect fail.\r\n");
		return 0;
	}
}

void user_mqtt_task(void *pvParameter)
{
	uint8_t mqtt_state=1;
	int32_t ret,rc;
	uint8_t phy=0;
	uint8_t dns_flag=0,phy_flag=1;
	for(;;)
	{
		if( !((phy=getPHYCFGR())&0X01) ){
			if(!phy_flag)
				continue;
			LOG_I("please insert the wire.\r\n");
			phy_flag = 0;
			continue; //δ�������߽�������ѭ��
		}
		phy_flag = 1;
		if(!dns_flag)
		{
			if(user_dns(MQTT_DOMAIN,mqtt_remote_ip))	//dns����
			{
				LOG_D("MQTT> dns get ip:%d.%d.%d.%d.\r\n",\
					mqtt_remote_ip[0],mqtt_remote_ip[1],mqtt_remote_ip[2],mqtt_remote_ip[3]);
				dns_flag = 1;
			}
			else 
				vTaskDelay(1000);
		}
		else
		{
			vTaskDelay(500);
			switch (getSn_SR(MQTT_SOCKET))
			{
				case SOCK_CLOSED:
					LOG_D("MQTT> SOCK_CLOSE.\r\n");
					if((ret = socket(MQTT_SOCKET, Sn_MR_TCP, 3000, NULL)) != MQTT_SOCKET){
						LOG_E("MQTT> socket open failed : %d.\r\n",ret);
						break;
					}
					break;
				case SOCK_ESTABLISHED:	//TCP������������
					LOG_I("MQTT> tcp connnect.\r\n");
					//MQTT ���ӷ��
					rc = mqtt(); //�������񣬳���mqtt���̳����쳣
					if(rc == 0)
					{
						LOG_E("MQTT> ERROR.reopen socket.\r\n");
						close(MQTT_SOCKET);
					}
				break;
				case SOCK_CLOSE_WAIT:
					LOG_D("MQTT> SOCK_CLOSE_WAIT.\r\n");
					break;
				case SOCK_INIT:
					LOG_D("MQTT> socket state SOCK_INIT.\r\n");
					if(ret = (uint32_t)connect(MQTT_SOCKET, mqtt_remote_ip, MQTT_PORT) != SOCK_OK){
						LOG_E("MQTT> socket connect faile : %d.\r\n",ret);
						break;
					}
					break;
				default :
					LOG_I("MQTT> unknow mqtt socke SR:%x.\r\n",getSn_SR(MQTT_SOCKET));
					break;
			}
		}
	}
}
