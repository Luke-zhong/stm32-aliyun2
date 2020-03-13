#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "ult.h"
#include "w5500.h"
#include "socket.h"
#include "dns.h"
#include "config.h"
#include "tcp_client.h"

//uint8 domain_name[];
uint8 DNS_GET_IP[4];
uint8 server_ip[4];
uint16 MSG_ID = 0x1122;
extern uint8 BUFPUB[1024];
uint8 dns_num=0;
extern CONFIG_MSG ConfigMsg;
void do_tcp_client(void);


int dns_makequery(uint16 op, uint8 * name, uint8 * buf, uint16 len)
{
  uint8  *cp;
  uint8   *cp1;
  //	int8   sname[MAX_DNS_BUF_SIZE];
  uint8  *dname;
  uint16 p;
  uint16 dlen;
  
  cp = buf;
  
  MSG_ID++;
  *(uint16*)&cp[0] = htons(MSG_ID);
  p = (op << 11) | 0x0100;			/* Recursion desired */
  *(uint16*)&cp[2] = htons(p);
  *(uint16*)&cp[4] = htons(1);
  *(uint16*)&cp[6] = htons(0);
  *(uint16*)&cp[8] = htons(0);
  *(uint16*)&cp[10]= htons(0);
  
  cp += sizeof(uint16)*6;
  //	strcpy(sname, name);
  dname = name;
  dlen = strlen((char*)dname);
  for (;;)
  {
    /* Look for next dot */
    cp1 = (unsigned char*)strchr((char*)dname, '.');
    
    if (cp1) len = cp1 - dname;	/* More to come */
    else len = dlen;			/* Last component */
    
    *cp++ = len;				/* Write length of component */
    if (len == 0) break;
    
    /* Copy component up to (but not including) dot */
    strncpy((char *)cp, (char*)dname, len);
    cp += len;
    if (!cp1)
    {
      *cp++ = 0;			/* Last one; write null and finish */
      break;
    }
    dname += len+1;
    dlen -= len+1;
  }
  
  *(uint16*)&cp[0] = htons(0x0001);				/* type */
  *(uint16*)&cp[2] = htons(0x0001);				/* class */
  cp += sizeof(uint16)*2;
  
  return ((int)((uint32)(cp) - (uint32)(buf)));
}


int parse_name(uint8 * msg, uint8 * compressed, /*char * buf,*/ uint16 len)
{
  uint16 slen;		/* Length of current segment */
  uint8  * cp;
  int16  clen = 0;		/* Total length of compressed name */
  int16  indirect = 0;	/* Set if indirection encountered */
  int16  nseg = 0;		/* Total number of segments in name */
  int8   name[MAX_DNS_BUF_SIZE];
  int8   *buf;
  
  buf = name;
  
  cp = compressed;
  
  for (;;)
  {
    slen = *cp++;	/* Length of this segment */
    
    if (!indirect) clen++;
    
    if ((slen & 0xc0) == 0xc0)
    {
      if (!indirect)
        clen++;
      indirect = 1;
      /* Follow indirection */
      cp = &msg[((slen & 0x3f)<<8) + *cp];
      slen = *cp++;
    }
    
    if (slen == 0)	/* zero length == all done */
      break;
    
    len -= slen + 1;
    
    if (len <= 0) return -1;
    
    if (!indirect) clen += slen;
    
    while (slen-- != 0) *buf++ = (int8)*cp++;
    *buf++ = '.';
    nseg++;
  }
  
  if (nseg == 0)
  {
    /* Root name; represent as single dot */
    *buf++ = '.';
    len--;
  }
  
  *buf++ = '\0';
  len--;
  
  return clen;	/* Length of compressed message */
}


uint8 * dns_question(uint8 * msg, uint8 * cp)
{
  int16 len;
  //	int8  xdata name[MAX_DNS_BUF_SIZE];
  
  len = parse_name(msg, cp, /*name,*/ MAX_DNS_BUF_SIZE);
  
  if (len == -1) return 0;
  
  cp += len;
  cp += 2;		/* type */
  cp += 2;		/* class */
  
  return cp;
}

uint8 * dns_answer(uint8 * msg, uint8 * cp)
{
  int16 len, type;
  //	int8  xdata name[MAX_DNS_BUF_SIZE];
  
  len = parse_name(msg, cp, /*name,*/ MAX_DNS_BUF_SIZE);
  
  if (len == -1) return 0;
  
  cp += len;
  type = ntohs(*((uint16*)&cp[0]));
  cp += 2;		/* type */
  cp += 2;		/* class */
  cp += 4;		/* ttl */
  cp += 2;		/* len */
  
  switch (type)
  {
    case TYPE_A:
      DNS_GET_IP[0] = *cp++;
      DNS_GET_IP[1] = *cp++;
      DNS_GET_IP[2] = *cp++;
      DNS_GET_IP[3] = *cp++;
      break;
    case TYPE_CNAME:
    case TYPE_MB:
    case TYPE_MG:
    case TYPE_MR:
    case TYPE_NS:
    case TYPE_PTR:
      /* These types all consist of a single domain name */
      /* convert it to ascii format */
      len = parse_name(msg, cp, /*name,*/ MAX_DNS_BUF_SIZE);
      if (len == -1) return 0;
      
      cp += len;
      break;
    case TYPE_HINFO:
      len = *cp++;
      cp += len;
      
      len = *cp++;
      cp += len;
      break;
    case TYPE_MX:
      cp += 2;
      /* Get domain name of exchanger */
      len = parse_name(msg, cp,/* name,*/ MAX_DNS_BUF_SIZE);
      if (len == -1) return 0;
      
      cp += len;
      break;
    case TYPE_SOA:
      /* Get domain name of name server */
      len = parse_name(msg, cp,/* name,*/ MAX_DNS_BUF_SIZE);
      if (len == -1) return 0;
      
      cp += len;
      
      /* Get domain name of responsible person */
      len = parse_name(msg, cp,/* name,*/ MAX_DNS_BUF_SIZE);
      if (len == -1) return 0;
      
      cp += len;
      
      cp += 4;
      cp += 4;
      cp += 4;
      cp += 4;
      cp += 4;
      break;
    case TYPE_TXT:
      /* Just stash */
      break;
    default:
    /* Ignore */
    break;
  }
  
  return cp;
}


uint8 parseMSG(struct dhdr * pdhdr, uint8 * pbuf)
{
  uint16 tmp;
  uint16 i;
  uint8 * msg;
  uint8 * cp;
  
  msg = pbuf;
  memset(pdhdr, 0, sizeof(pdhdr));
  
  pdhdr->id = ntohs(*((uint16*)&msg[0]));
  tmp = ntohs(*((uint16*)&msg[2]));
  if (tmp & 0x8000) pdhdr->qr = 1;
  
  pdhdr->opcode = (tmp >> 11) & 0xf;
  
  if (tmp & 0x0400) pdhdr->aa = 1;
  if (tmp & 0x0200) pdhdr->tc = 1;
  if (tmp & 0x0100) pdhdr->rd = 1;
  if (tmp & 0x0080) pdhdr->ra = 1;
  
  pdhdr->rcode = tmp & 0xf;
  pdhdr->qdcount = ntohs(*((uint16*)&msg[4]));
  pdhdr->ancount = ntohs(*((uint16*)&msg[6]));
  pdhdr->nscount = ntohs(*((uint16*)&msg[8]));
  pdhdr->arcount = ntohs(*((uint16*)&msg[10]));
  
  /* Now parse the variable length sections */
  cp = &msg[12];
  
  /* Question section */
  for (i = 0; i < pdhdr->qdcount; i++)
  {
    cp = dns_question(msg, cp);
  }
  
  /* Answer section */
  for (i = 0; i < pdhdr->ancount; i++)
  {
    cp = dns_answer(msg, cp);
  }
  
  /* Name server (authority) section */
  for (i = 0; i < pdhdr->nscount; i++)
  {
    ;
  }
  
  /* Additional section */
  for (i = 0; i < pdhdr->arcount; i++)
  {
    ;
  }
  
  if(pdhdr->rcode == 0) return 1;		// No error
  else return 0;
}

uint8 dns_query(uint8 s, uint8 * name)
{
  static uint32 dns_wait_time = 0;
  struct dhdr dhp;	/*����һ���ṹ��������������ͷ��Ϣ*/
  uint8 ip[4];
  uint16 len, port;
  switch(getSn_SR(s))		/*��ȡsocket��״̬*/
  {
    case SOCK_UDP:			/*��socket*/
      if ((len = getSn_RX_RSR(s)) > 0)
      {
        if (len > MAX_DNS_BUF_SIZE) len = MAX_DNS_BUF_SIZE;	
        len = recvfrom(s, BUFPUB, len, ip, &port);		/*����UDP��������ݲ�����BUFPUB��������*/
        if(parseMSG(&dhp, BUFPUB))										/*����DNS����Ӧ��Ϣ*/
        {
          close(s);																		/*�ر�Socket*/
          return DNS_RET_SUCCESS;											/*����DNS���������ɹ���ֵ*/
        }
        else 
          dns_wait_time = DNS_RESPONSE_TIMEOUT;				/*DNS�ȴ���Ӧʱ������Ϊ��ʱ*/
      }
      else
      {											/*û���յ�DNS��������UDP�ظ�*/
        Delay_ms(1000);			/*�������Ƶ���Ĳ�ѯ����ʱ1s*/
        dns_wait_time++;		/*DNS�ȴ���Ӧʱ���1*/
        //if(ConfigMsg.debug) printf("dns wait time=%d\r\n", dns_wait_time);
      }
      if(dns_wait_time >= DNS_RESPONSE_TIMEOUT)   // ���DNS�ȴ�ʱ�䳬��3��
      {
        close(s);
        return DNS_RET_FAIL;								/*����DNS��������ʧ�ܵ�ֵ*/
      }
      break;
    case SOCK_CLOSED:												/*Socket�ر�*/
      dns_wait_time = 0;
      socket(s, Sn_MR_UDP, 3000, 0); /*��W5500��Socket��3000�˿ڲ�����ΪUDPģʽ*/
      //if(ConfigMsg.debug) printf("dns socket init over\r\n");
      len = dns_makequery(0, name, BUFPUB, MAX_DNS_BUF_SIZE);/*����DNS�����Ĳ����뻺��BUFPUB��*/
      //if(ConfigMsg.debug) printf("dns make query\r\n");
      sendto(s, BUFPUB, len, EXTERN_DNS_SERVERIP, IPPORT_DOMAIN);/*����DNS�����ĸ�DNS������*/
      break;         
  }
  return DNS_RET_PROGRESS;
}

uint16 do_dns(uint8 *domain_name)
{
  uint8 dns_retry_cnt=0;
  uint8 dns_ok=0;
	uint8 flag=0;
  if( (dns_ok==1) ||  (dns_retry_cnt > DNS_RETRY))
  {
    return 0;
  }
  else if(memcmp(ConfigMsg.dns,"\x00\x00\x00\x00",4))//�ж�DNS��������IP��ַ�Ƿ�����
  {
    switch(dns_query(SOCK_DNS,domain_name))//����DNS�����ĺͽ���DNS��Ӧ����
    {
      case DNS_RET_SUCCESS:								//DNS���������ɹ�
        dns_ok=1;														//DNS���б�־λ��1
        memcpy(ConfigMsg.rip,DNS_GET_IP,4);//�ѽ�������IP��ַ���Ƹ�ConfigMsg.rip
        dns_retry_cnt=0;										//DNS�����Ĵ�����0
				//printf("%d.%d.%d.%d\r\n",ConfigMsg.rip[0],ConfigMsg.rip[1],ConfigMsg.rip[2],ConfigMsg.rip[3]);
			if(flag==0)
			{
				memcpy(server_ip,ConfigMsg.rip,4);	//�ѽ������ĵ�ַ��ֵ��MQTT��server_ip
				printf("%d.%d.%d.%d\r\n",server_ip[0],server_ip[1],server_ip[2],server_ip[3]);
				flag++;;
				Delay_s(2);
				while(1)
				{
						MQTT_CON_ALI();//MQTT����
				}
			}
        break;
      case DNS_RET_FAIL:											//DNS��������ʧ��
        dns_ok=0;															//DNS���б�־λ��0
        dns_retry_cnt++;											//DNS�����Ĵ�����1
        printf("Fail! Please check your network configuration or DNS server.\r\n");
        break;
      default:
        break;
    }
  }
  else																	//���DNS������IPΪ0.0.0.0
    printf("Invalid DNS server [%d.%d.%d.%d]\r\n",ConfigMsg.dns[0],ConfigMsg.dns[1],ConfigMsg.dns[2],ConfigMsg.dns[3]);
		dns_num++;
	
	return 0;
}



