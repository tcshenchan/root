/*
配置定义
*/

#ifndef __SIM900A_H
#define __SIM900A_H

#include "config.h"

#define swap16(x) (x&0XFF)<<8|(x&0XFF00)>>8		//高低字节交换宏定义


#define SIM900_STA PCin(4)
#define SIM900_ON  PCout(5)

#define GSM_NO_RESPONSE -1
#define GSM_OK 1
#define GSM_ERROR 0

int cstr_to_unicode(char* str,char* str_out, int len);
int sim900a_send_cmd(u8 *cmd,u8 *ack,u16 time_out_10ms);
void sim900a_init(void);
int sim900a_get_rssi(void);
int sim900a_get_time(char* str);
int sim900a_send_unicode_msg(char* phone, char* text_unic,int len);
int sim900a_call(char* phone, int time_out_10ms);
int sim900a_hang(void);
#endif





