#ifndef __USER_MQTT_H
#define __USER_MQTT_H


#define		MQTT_SOCKET		2
#define		MQTT_PORT		1883
#define		MQTT_CLIEND_ID	"******"	//ID
#define		MQTT_DOMAIN		"******"	//MQTT����������
#define 	MQTT_USER		"******"	//��½�û���
#define		MQTT_PASS		"******"	//��½����

#define 	MQTT_BUFFLEN	200
typedef void (* mqtt_connect_callback)(void);

void user_mqtt_task(void *pvParameter);
void user_mqtt_init(void);








#endif /*__USER_MQTT_H*/
