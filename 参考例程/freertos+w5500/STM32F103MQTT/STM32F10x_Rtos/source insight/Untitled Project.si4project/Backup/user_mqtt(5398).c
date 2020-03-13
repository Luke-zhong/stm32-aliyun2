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
	user_MQTTPacket_ConnectData.clientID.cstring = "123";
}
uint8_t mqtt_remote_ip[4];

void user_mqtt_task(void *pvParameter)
{
	uint8_t mqtt_state=1;
	int32_t ret;
	uint8_t mqtt_buff[200];
	int mqtt_bufflen = sizeof(mqtt_buff);
	while(1)
	{
		if(user_dns(MQTT_DOMAIN,mqtt_remote_ip))	//dns解析
		{
			LOG_D("MQTT> dns get ip:%d.%d.%d.%d.\r\n",\
						mqtt_remote_ip[0],mqtt_remote_ip[1],mqtt_remote_ip[2],mqtt_remote_ip[3]);
			while(mqtt_state)
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
					case SOCK_ESTABLISHED:
						LOG_I("MQTT> mqtt tcp connnect.\r\n");
						//MQTT 连接封包
						ret=MQTTSerialize_connect(mqtt_buff, mqtt_bufflen, &user_MQTTPacket_ConnectData);
						ret=send(MQTT_SOCKET, mqtt_buff, 4);
						if(ret != mqtt_bufflen){
							LOG_E("MQTT> send error:%d.",ret);
						}
						while(MQTTPacket_read(mqtt_buff, mqtt_bufflen, transport_getdata) != CONNACK);
						LOG_I("MQTT> mqtt connect.\r\n");
						break;
					case SOCK_CLOSE_WAIT:
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
		else
		{
			vTaskDelay(1000);
		}
	}
}
