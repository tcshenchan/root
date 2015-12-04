#include <board.h>
#include <rtthread.h>
#include <finsh.h>
#include <stdio.h>
#include "led.h"

#include "jlx12864.h"
#include "gw.h"
#include "key.h"
#include "f315.h"

int input_detector(unsigned int *detector_id)
{
	char str[20];
	int count;
	*detector_id = 0;
	alarm_swicth = 0;
	count = input_panel("����ID:(С��9λ)",str,8);
	if(count == -1)
	{
		lcd_fill(0);
		return -1;
	}
	else if(count == 0)
	{
		lcd_fill(0);
		lcd_puts(0,0,"���߽���ID:",0);
		sprintf(str, "%d", *detector_id);
		lcd_puts(0,2,str,0);
		while(1)
		{
			char key = KEY_VALUE;
			sleep(10);
			if(received_315)
			{
				*detector_id = NewReceive[0]+NewReceive[1]*256+NewReceive[2]*0x10000;
				sprintf(str, "%d", *detector_id);
				lcd_puts(0,2,str,0);
				received_315 = 0;
			}
			if(key == 'E')
			{
				break;
			}
			if(key == 'Q')
			{
				lcd_fill(0);
				alarm_swicth = 1;
				return -1;
			}
		}
	}
	else
	{
		str[count] = 0;
		*detector_id = atoi(str);
	}
	alarm_swicth = 1;
	return 0;
}
int input_room_id(room_t* room)
{
	int count;
	char str[20];
	int no[3] = {0};
	re_input1:
	count = input_panel("���뷿��:(��*��*��)",str,8);
	if(count == -1)
	{
		lcd_fill(0);
		return -1;
	}
	count = str_separation(str,no,3);
	if(count < 3)
	{
		lcd_fill(0);
		lcd_puts(0,0,"��ʽ����,������!",0);
		sleep(2000);
		goto re_input1;
	}
	room->room_id[0] = EXIST;
	room->room_id[1] = no[0];
	room->room_id[2] = no[1];
	room->room_id[3] = no[2];
	return 0;
}
int detector_add(void* para)
{

	
	int count;
	unsigned int detector_id;
	room_t room = {0};
	if(input_detector(&detector_id)== (-1))
	{
		lcd_fill(0);
		return 0;
	}
	if(find_detector(detector_id, &count)>0)
	{
		char str_log[28];
		sprintf(str_log,"�����ظ��Ǽ�!");
		lcd_fill(0);
		lcd_puts(0,0,str_log,0);
		sleep(2000);
		lcd_fill(0);
		return 0;
	}
	re_input1:
	if(input_room_id(&room) == (-1))
	{
		lcd_fill(0);
		return 0;
	}
	count = find_room(&room);
	if(count==0)
	{
		lcd_fill(0);
		lcd_puts(0,0,"���䲻����,����!",0);
		sleep(2000);
		goto re_input1;
	}
	memcpy(&room,(room_t*)count,sizeof(room_t));
	{
		int i;
		for(i = 0; i < 6; i++)
		{
			if(room.detector[i][0]!=0x5a)
			{
				memcpy(room.detector[i]+1,&detector_id,3);
				room.detector[i][0] = 0x5a;
				find_write_room(&room);
				lcd_fill(0);
				lcd_puts(0,0,"�����!",0);
				sleep(2000);
				lcd_fill(0);
				return 0;
			}
		}
	}
	lcd_fill(0);
	lcd_puts(0,0,"�÷���̽��������!",0);
	sleep(2000);
	lcd_fill(0);
	return 0;
}
int detector_delete(void* para)
{
	char str[20];
	int count;
	int detector_id;
	if(input_detector(&detector_id)== -1)
	{
		lcd_fill(0);
		return 0;
	}

	count = find_delete_detector(detector_id);
	if(count>0)
	{
		room_t *room = (room_t*)count;
		sprintf(str,"����:%d-%d-%d",room->room_id[1],room->room_id[2],room->room_id[3]);
		lcd_fill(0);
		lcd_puts(0,0,str,0);
		lcd_puts(0,2,"��ɾ����̽����!",0);
		sleep(2000);
	}
	else
	{
		lcd_fill(0);
		lcd_puts(0,0,"̽����������!",0);
		sleep(2000);
	}
	lcd_fill(0);
	return 0;
}

int detector_query(void* para)
{
	char str[20];
	int offset;
	int count;
	int detector_id;
	if(input_detector(&detector_id)== -1)
	{
		lcd_fill(0);
		return 0;
	}
	count = find_detector(detector_id,&offset);
	if(count>0)
	{
		room_t *room = (room_t*)count;
		sprintf(str,"̽����id:%d",detector_id);
		lcd_puts(0,0,str,0);
		sprintf(str,"���ڷ���:%d-%d-%d",room->room_id[1],room->room_id[2],room->room_id[3]);
		lcd_puts(0,2,str,0);
		sprintf(str,"����λ��:��%d��",offset);
		lcd_puts(0,4,str,0);
		while(1)
		{
			char key = KEY_VALUE;
			if((key == 'Q')||(key == 'E')||(key == 'C'))
			{
				break;
			}
		}
	}
	else
	{
		lcd_fill(0);
		lcd_puts(0,0,"̽����������!",0);
		sleep(2000);
	}
	lcd_fill(0);
	return 0;
}
int detector_manage(void* para)
{
	 const unsigned char menu[][20] = {
				 "1.���̽����  ",
				 "2.ɾ��̽����  ",
				 "3.����̽����  "
				};
	int (*func_list[])(void*)  = {
		detector_add,
		detector_delete,
		detector_query  
	};
	show_menu(menu, func_list,0,3);
	return 0;
}