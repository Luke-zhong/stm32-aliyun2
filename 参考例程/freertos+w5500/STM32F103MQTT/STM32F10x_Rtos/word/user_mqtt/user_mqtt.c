#include "stm32f10x_conf.h"
#include "user_mqtt.h"
#include "socket.h"
//#include "MQTTConnect.h"
#include "MQTTPacket.h"
/*
	brief: include DNS,connect mqtt,subscribe，publish，and receive server data.
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
/*mqtt连接函数
	参数：空
	返回：1：连接成功，0连接失败
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
		//等待应答信号
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
/*mqtt订阅主题
	参数：主题
	返回：1：成功,0:失败
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
	MQTT 发送ping报文
	参数：空
	返回：1：发送成功，0：发送失败
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
/*mqtt运行任务，当tcp成功建立链接时，恢复该任务，否则是挂起状态
当LINK断开时，挂起该任务
该任务建立在TCP成功连接的基础上：连接mqtt服务器->订阅相关主题->
返回：正常情况阻塞任务，异常返回错误类型
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
			while(1) //开始接收数据，阻塞任务
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
			continue; //未插入网线结束本次循环
		}
		phy_flag = 1;
		if(!dns_flag)
		{
			if(user_dns(MQTT_DOMAIN,mqtt_remote_ip))	//dns解析
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
				case SOCK_ESTABLISHED:	//TCP正常建立连接
					LOG_I("MQTT> tcp connnect.\r\n");
					//MQTT 连接封包
					rc = mqtt(); //阻塞任务，除非mqtt过程出现异常
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
