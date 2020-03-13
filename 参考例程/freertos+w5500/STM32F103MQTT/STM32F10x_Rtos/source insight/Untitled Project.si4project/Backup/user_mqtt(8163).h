#ifndef __USER_MQTT_H
#define __USER_MQTT_H

#define		MQTT_SOCKET		2
#define		MQTT_PORT		1883
#define		MQTT_DOMAIN		"iot.diqi.sh"
#define 	MQTT_USER		"fdjadmin"
#define		MQTT_PASS		"123456"

void user_mqtt_task(void *pvParameter);
void user_mqtt_init(void);








#endif /*__USER_MQTT_H*/
