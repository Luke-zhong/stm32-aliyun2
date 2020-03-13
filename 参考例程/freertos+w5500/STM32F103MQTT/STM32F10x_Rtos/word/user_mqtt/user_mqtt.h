#ifndef __USER_MQTT_H
#define __USER_MQTT_H


#define		MQTT_SOCKET		2
#define		MQTT_PORT		1883
#define		MQTT_CLIEND_ID	"******"	//ID
#define		MQTT_DOMAIN		"******"	//MQTT服务器域名
#define 	MQTT_USER		"******"	//登陆用户名
#define		MQTT_PASS		"******"	//登陆密码

#define 	MQTT_BUFFLEN	200
typedef void (* mqtt_connect_callback)(void);

void user_mqtt_task(void *pvParameter);
void user_mqtt_init(void);








#endif /*__USER_MQTT_H*/
