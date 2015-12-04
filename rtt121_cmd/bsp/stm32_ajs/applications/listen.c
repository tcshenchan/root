#include <board.h>
#include <rtthread.h>
#include <finsh.h>
#include <stdio.h>
#include "led.h"

#include "jlx12864.h"
#include "key.h"
#include "f315.h"
#include "gw.h"
#include "sim900a.h"
#include "audio.h"

#define TIME_GAP 400
room_t *alarm_room = {0};
int rssi = 0;
int net_sta_in = 0;
char time_str[30] = {0};
char alarm_str[30] = {0};//alarm_str[0]的bit0为电源断开标志，bit1为火灾抱紧标志，bit4为是否发出声音
char *time_str_display = 0;
int alarm_swicth = 1;
unsigned int voice_running = 0;
const char fire[] ="53F7623F95F453D1751F706B707E";

void alarm_to_phone(room_t *alarm_room, char* str_phone_num)
{
	//make a phone call
	rt_kprintf("make the phone call : %s\r\n", str_phone_num);
	if(sim900a_call(str_phone_num,4000)==GSM_OK)
	{
		voice_running = 0xff;
		if(alarm_room->room_id[1]/10 > 0)
		{wav_out(8000,wav_data[alarm_room->room_id[1]/10],2600); sleep(TIME_GAP);}
		wav_out(8000,wav_data[alarm_room->room_id[1]%10],2600); sleep(TIME_GAP);
		wav_out(8000,wav_data[13],2050);						sleep(TIME_GAP);

		if(alarm_room->room_id[2]/10 > 0)
		{wav_out(8000,wav_data[alarm_room->room_id[2]/10],2600); sleep(TIME_GAP);}
		wav_out(8000,wav_data[alarm_room->room_id[2]%10],2600);sleep(TIME_GAP);
		wav_out(8000,wav_data[14],6100);						sleep(TIME_GAP*(1.6));

		if(alarm_room->room_id[3]/10 > 0)
		{wav_out(8000,wav_data[alarm_room->room_id[3]/10],2600);sleep(TIME_GAP);}
		wav_out(8000,wav_data[alarm_room->room_id[3]%10],2600);sleep(TIME_GAP);
		wav_out(8000,wav_data[11],13400);		               sleep(3000);		


		// voice repeat
		if(alarm_room->room_id[1]/10 > 0)
		{wav_out(8000,wav_data[alarm_room->room_id[1]/10],2600); sleep(TIME_GAP);}
		wav_out(8000,wav_data[alarm_room->room_id[1]%10],2600); sleep(TIME_GAP);
		wav_out(8000,wav_data[13],2050);						sleep(TIME_GAP);

		if(alarm_room->room_id[2]/10 > 0)
		{wav_out(8000,wav_data[alarm_room->room_id[2]/10],2600); sleep(TIME_GAP);}
		wav_out(8000,wav_data[alarm_room->room_id[2]%10],2600);sleep(TIME_GAP);
		wav_out(8000,wav_data[14],6100);						sleep(TIME_GAP*(1.6));

		if(alarm_room->room_id[3]/10 > 0)
		{wav_out(8000,wav_data[alarm_room->room_id[3]/10],2600);sleep(TIME_GAP);}
		wav_out(8000,wav_data[alarm_room->room_id[3]%10],2600);sleep(TIME_GAP);
		wav_out(8000,wav_data[11],13400);		               sleep(3000);	

		voice_running = 0;		
	}
	sim900a_hang();
	
	sleep(2000);
	//then send the message
	if((strlen(str_phone_num)==11)&&(str_phone_num[0]=='1'))
	{
		char str_room[12];
		char str_unic[128];
		rt_kprintf("send msg:%s\r\n",str_phone_num);
		sprintf(str_room, "%d-%d-%d",alarm_room->room_id[1],alarm_room->room_id[2],alarm_room->room_id[3]);
		cstr_to_unicode(str_room,str_unic,strlen(str_room));
		strcat(str_unic,fire);
		sim900a_send_unicode_msg(str_phone_num,str_unic,strlen(str_unic));
	}	
}
void listen_thread_entry(void* parameter)
{
	while(1)
	{
		int i;
		sleep(10);
		if(dc_detecte()==0)
		{
			if((alarm_str[0]&0x0f) == 0)
			{
				sprintf(alarm_str+1,"警告:电源断开!!");
				alarm_str[0]=  0x11;
			}
			breakdown_led(1);
		}
		else if((alarm_str[0]&0x0f) == 1)
		{
			alarm_str[0] = 0;
			breakdown_led(0);
		}
		else
		{
			breakdown_led(0);
		}
		if(received_315)
		{
			if(alarm_swicth)
			{
				int offset;
				rec_t rec;
				int detector = NewReceive[0]+NewReceive[1]*256+NewReceive[2]*0x10000;
				alarm_room = find_detector(detector,&offset);
				rt_kprintf("fire on %d\r\n",detector);

				if(alarm_room)
				{
					//add records
					rec_t rec;
					rec.available = EXIST;
					memcpy(rec.date, time_str,15);
					sprintf(rec.text, "%d-%d-%d,%d",alarm_room->room_id[1],alarm_room->room_id[2],alarm_room->room_id[3],detector);
					add_rec(&rec);

					//put message on screen
					sprintf(alarm_str+1,"警告:%d-%d-%d房火灾",alarm_room->room_id[1],alarm_room->room_id[2],alarm_room->room_id[3]);
					alarm_str[0] = 0x12;
				}
			}
			sleep(2000);
			received_315 = 0;
		}
	}
}
void listen_thread_run(void)
{
			rt_thread_t thread;
			thread = rt_thread_create("listen",
                                   listen_thread_entry, RT_NULL,
                                   1024*2, 16, 20);
			rt_thread_startup(thread);
}
void gsm_thread_entry(void* para)
{
	int i ;
	sim900a_init();
	sleep(3000);
	//sim900a_send_msg("18030844038",fire,strlen(fire));
	//sim900a_send_unicode_msg("18030844038",fire,strlen(fire));
	// sprintf(alarm_msg[0].phone,"18030844038");
	// sprintf(alarm_msg[0].text,"hello");
	// alarm_msg[0].available = 1;
	//while(1)sleep(10);

	while(1)
	{
		sleep(2000);
		if(alarm_room)
		{
			public_phone_t *public = (public_phone_t*)PUBLIC_PHONE_ADDR;
			//personal phone
			for( i = 0; i < 3; i++)
			{
				if(alarm_room->phone[i][0] == EXIST)
				{
					char str_phone_num[12];
					memcpy(str_phone_num, alarm_room->phone[i]+1,11);
					str_phone_num[11] = 0;
					alarm_to_phone(alarm_room,str_phone_num );
				}
			}

			//public phone
			for(i = 0; i<6; i++)
			{
				if(public->phone[i][0] == EXIST)
				{
					char str_phone_num[12];
					memcpy(str_phone_num, public->phone[i]+1,11);
					str_phone_num[11] = 0;
					alarm_to_phone(alarm_room,str_phone_num );
				}
			}
			alarm_room = 0;
		}
		rssi = sim900a_get_rssi();
		if (sim900a_send_cmd_fast("AT+COPS?","CHINA MOBILE",10)==GSM_OK)
		{
			net_sta_in = 1;
		}
		else if(sim900a_send_cmd_fast("AT+COPS?","CHN-UNICOM",10)==GSM_OK)
		{
			net_sta_in = 2;
		}
		else
		{
			net_sta_in = 0;
		}
		if(sim900a_get_time(time_str)==GSM_OK)
		{
			time_str_display = time_str;
		}
		else
		{
			time_str_display = 0;
		}
	}
}
void gsm_thread_run(void)
{
			rt_thread_t thread;
			thread = rt_thread_create("gsm",
                                   gsm_thread_entry, RT_NULL,
                                   1024*6, 24, 20);
			rt_thread_startup(thread);
}