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
void time_setting(void *para)
{
	char str[100];
	int no[6] = {0};
	int count,i;
	re_input1:
	count = input_panel("����:(��*��*��)",str,8);
	if(count == -1)
	{
		lcd_fill(0);
		return;
	}
	count = str_separation(str,no,3);
	if(count < 3)
	{
		lcd_fill(0);
		lcd_puts(0,0,"��ʽ����,������!",0);
		sleep(2000);
		goto re_input1;
	}

	re_input2:
	count = input_panel("ʱ��:(ʱ*��)",str,5);
	if(count == -1)
	{
		lcd_fill(0);
		return;
	}
	count = str_separation(str,no+3,2);
	if(count < 2)
	{
		lcd_fill(0);
		lcd_puts(0,0,"��ʽ����,������!",0);
		sleep(2000);
		goto re_input2;
	}
	sprintf(str,"AT+CCLK=\"%02d/%02d/%02d,%02d:%02d:30+00\"",no[0],no[1],no[2],no[3],no[4]);
	lcd_fill(0);
	if(sim900a_send_cmd(str,"OK",50)==GSM_OK)
	{
		lcd_puts(0,0,"���óɹ�!",0);
	}
	else
	{
		lcd_puts(0,0,"����ʧ��!",0);
	}
	sleep(2000);
	lcd_fill(0);
}

void passwd_setting(void *para)
{
	char str[20];
	int count,passwd;
	count = input_panel("����:(����,<9λ)",str,8);
	if(count == -1)
	{
		lcd_fill(0);
		return;
	}
	str[count] = 0;
	passwd = atoi(str);
	FLASH_Unlock();
	FLASH_ErasePage(PASSWD_ADDR);
	FLASH_ProgramWord(PASSWD_ADDR+4, passwd);
	FLASH_ProgramWord(PASSWD_ADDR, EXIST);
	FLASH_Lock();
	lcd_puts(0,0,"���óɹ�!",0);
	sleep(2000);
	lcd_fill(0);
}
void passwd_init(void)
{
	unsigned int available;
	available = *(unsigned int*)PASSWD_ADDR;
	if(available!=EXIST)
	{
		FLASH_Unlock();
		FLASH_ErasePage(PASSWD_ADDR);
		FLASH_ProgramWord(PASSWD_ADDR+4, 0);
		FLASH_ProgramWord(PASSWD_ADDR, EXIST);
		FLASH_Lock();
	}
}
int edit_pubulic_by_text(public_phone_t* public_in,char* text)
{
	int offset = text[4]-48-1;
	public_phone_t public;
	memcpy(&public,public_in,sizeof(public_phone_t));
	if(number_str_check(text+6)<0)
	{
		return -1;
	}
	if(strncmp(text,"�绰",4) == 0)
	{
		unsigned  int count = strlen(text+6);
		if(count <= 2)
		{
			public.phone[offset][0] = 0;
		}
		else
		{
			public.phone[offset][0] = EXIST;
			memcpy(public.phone[offset]+1,text+6,11);
		}
	}
	write_public_phone(&public);
	return 0;
}

int public_phone_setting(void *para_in)
{	
	int i,row = 0;
	public_phone_t *public = (public_phone_t*)PUBLIC_PHONE_ADDR;
	char str_buf[6][20] = {0};
	int (*func_list[])(void*)  = {
		edit_phone,
		edit_phone,
		edit_phone,
		edit_phone,
		edit_phone,
		edit_phone
	};
	void* para[] = {
		str_buf[0]+6,
		str_buf[1]+6,
		str_buf[2]+6,
		str_buf[3]+6,
		str_buf[4]+6,
		str_buf[5]+6
	};
	redo:
	for(i = 0;i<6;i++)
	if(public->phone[i][0] == EXIST)
	{
		memcpy(str_buf[i]+8,public->phone[i]+1,11);
		str_buf[i][19] = 0;
		sprintf(str_buf[i],"�绰%d:%s",i+1,str_buf[i]+8);
	}
	else
	{
		sprintf(str_buf[i],"�绰%d:",i+1);
	}
	i = show_menu_full_func(str_buf,func_list,para,6,0,row) ;
	if(i <= -1)
	{
		int ret = 0;

		ret = edit_pubulic_by_text(public, str_buf[i*(-1)-1]);
		if(ret<0)
		{
				lcd_fill(0);
				lcd_puts(0,0,"����ʧ��!",0);
				lcd_puts(0,2,str_buf[i*(-1)-1],0);
				sleep(1000);
				lcd_fill(0);
		}
		if(ret == 0)
		{
			lcd_fill(0);
			lcd_puts(0,0,"����ɹ�!",0);
			sleep(1000);
		}
		row = i*(-1)-1;
		goto redo;
	}
	return 0;
}
int wipe_all_data(void *para_in)
{
	lcd_fill(0);
	lcd_puts(0,0,"��ɾ����������",0);
	lcd_puts(0,2,"����?",0);
	while(1)
	{
		char key = KEY_VALUE;
		sleep(10);
		if(key == 'E')
		{
			wipe_all_rec();
			wipe_all_room();
			wipe_all_publici_phone();
			wipe_passwd();
			lcd_fill(0);
			lcd_puts(0,0,"�ѻָ���������",0);
			lcd_puts(0,2,"�����¿���!",0);
			sleep(2000);
		}
		if(key == 'Q')
		{
			break;
		}
	}
	lcd_fill(0);
	return 0;
}
int repeater_det(void *para_in)
{
	unsigned int detector_id = 0;
	unsigned int times = 0;
	alarm_swicth = 0;
	lcd_fill(0);
	lcd_puts(0,0,"�밴�����߲��Լ�!",0);
	sleep(500);
	while(1)
	{
		char key = KEY_VALUE;
		sleep(10);
		if(key == 'E')
		{
			break;
		}
		if(key == 'Q')
		{
			break;
		}
		if(received_315)
		{
			char str[30];
			unsigned int recv = NewReceive[0]+NewReceive[1]*256+NewReceive[2]*0x10000;
			if(recv == detector_id)
			{
				times++;
			}
			else
			{
				times = 1;
			}
			detector_id = recv; 
			sprintf(str, "ID:%d,%d��", detector_id,times);
			lcd_fill(0);
			lcd_puts(0,0,"�밴�м̲��Լ�!",0);
			lcd_puts(0,2,str,0);
			sleep(1500);
			received_315 = 0;
		}
	}
	lcd_fill(0);
	alarm_swicth = 1;
	return 0;
}
int settings(void* parameter)
{

	 const unsigned char menu[][20] = {
				 "1.ʱ������  ",
				 "2.��������  ",
				 "3.��������  ",
				 "4.��������  ",
				 "5.���ղ���  "
				};
	int (*func_list[])(void* para)  = {
	time_setting,
	passwd_setting,
	public_phone_setting,
	wipe_all_data,
	repeater_det
	};
	show_menu(menu, func_list,0,5);
	return 0;
}