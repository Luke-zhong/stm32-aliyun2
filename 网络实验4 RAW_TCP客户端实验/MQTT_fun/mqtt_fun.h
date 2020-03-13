#ifndef  __MQTT_FUN_H_
#define  __MQTT_FUN_H_
#include "types.h"
#define SOCK_TCPS         0
#define SOCK_TCPC         2
int transport_sendPacketBuffer(unsigned char* buf, int buflen);
int transport_sendPacketBuffer1(unsigned char* buf, int buflen);
int transport_getdata(unsigned char* buf, int count);
int transport_getdata1(unsigned char* buf, int count);
int transport_open(void);
int transport_open1(void);
int transport_close(void);
int transport_close1(void);
void make_con_msg(char* clientID,int keepalive, uint8 cleansession,char*username,char* password,unsigned char*buf,int buflen);
void make_sub_msg(char *Topic,unsigned char*msgbuf,int buflen);
void make_pub_msg(char *Topic,unsigned char*msgbuf,int buflen,char*msg);
int  make_ping_msg(unsigned char* buf,int buflen);
int mqtt_decode_msg(unsigned char*buf);
#endif
