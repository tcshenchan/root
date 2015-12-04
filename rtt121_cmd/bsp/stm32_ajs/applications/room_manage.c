#include <board.h>
#include <rtthread.h>
#include <finsh.h>
#include <stdio.h>
#include "led.h"

#include "jlx12864.h"
#include "gw.h"
#include "key.h"
int number_str_check(char* str)
{
	while(*str)
	{
		if((*str<'0')||(*str > '9'))
			return -1;
		str++;
	}
	return 1;
}
int edit_save_room_by_text(room_t* room_in,char* text)
{
	int offset = text[4]-48-1;
	room_t room;
	memcpy(&room,room_in,sizeof(room_t));
	if(number_str_check(text+6)<0)
	{
		return -1;
	}
	if(strncmp(text,"电话",4) == 0)
	{
		unsigned  int count = strlen(text+6);
		if(count <= 2)
		{
			room.phone[offset][0] = 0;
		}
		else
		{
			room.phone[offset][0] = EXIST;
			memcpy(room.phone[offset]+1,text+6,11);
		}
	}
	else if(strncmp(text,"探测",4) == 0)
	{
		unsigned  int id = atoi(text+6);
		if(id == 0)
		{
			room.detector[offset][0] = 0;
		}
		else
		{
			int nouse;
			if(find_detector(id, &nouse)>0)
			{
				char str_log[28];
				sprintf(str_log,"探测%d不能重复登记",offset+1);
				lcd_fill(0);
				lcd_puts(0,0,str_log,0);
				sleep(2000);
				lcd_fill(0);
				return -1;
			}
			room.detector[offset][0] = EXIST;
			memcpy(room.detector[offset]+1,&id, 3);
		}
	}
	if(find_write_room(&room)>0)
	{return 0;}
	else 
	{return -1;rt_kprintf("eeror write\r\n");}
}
int delete_signal(void *para)
{
	lcd_fill(0);
	lcd_puts(20,1,"确认删除房间?",0);
	while(1)
	{
		char key_inside = KEY_VALUE;
		sleep(10);
		if(key_inside == 'E')
		{

			return -1;//此处-1特殊用途。
			break;
		}
		else if(key_inside == 'Q')
		{
			break;
		}
	}
	return 0;
	
}
int edit_detector(void *para)
{
	unsigned int detector;
	int ret;
	ret = input_detector(&detector);
	if(ret != -1)
	{
		sprintf((char*)para,"%d", detector);
		return -1;  
	}
	return 0;
}
int edit_phone(void *para)
{
	unsigned int detector;
	int ret;
	char* str[20];
	sprintf(str,"%s",(char*)para);
	ret = modif_input_panel("输入电话:",str,11,1);
	if(ret != -1)
	{
		str[ret] = 0;
		sprintf((char*)para,"%s", str);
		return -1;
	}
	return 0;  
	
}
int show_room_info(room_t *room)
{	
	int i;
	char str_buf[10][20] = {0};
	int row = 0;
	room_t* room_find = room;
	int (*func_list[10])(void*)  = {
		delete_signal,
		edit_phone,
		edit_phone,
		edit_phone,
		edit_detector,
		edit_detector,
		edit_detector,
		edit_detector,
		edit_detector,
		edit_detector
	};
	void* para[10] = {
		0,
		str_buf[1]+6,
		str_buf[2]+6,
		str_buf[3]+6,
		str_buf[4]+6,
		str_buf[5]+6,
		str_buf[6]+6,
		str_buf[7]+6,
		str_buf[8]+6,
		str_buf[9]+6
	};
	redo:
	sprintf(str_buf[0],"房间:%d-%d-%d删除?",room_find->room_id[1],room_find->room_id[2],room_find->room_id[3]);
	for(i = 0;i<3;i++)
	if(room_find->phone[i][0] == EXIST)
	{
		memcpy(str_buf[i+1]+8,room_find->phone[i]+1,11);
		str_buf[i+1][19] = 0;
		sprintf(str_buf[i+1],"电话%d:%s",i+1,str_buf[i+1]+8);
	}
	else
	{
		sprintf(str_buf[i+1],"电话%d:",i+1);
	}
	
	for(i = 4;i<10;i++)
	{
		if(room_find->detector[i-4][0] == 0x5a)
		{
			sprintf(str_buf[i],"探测%d:%d",i-3,*(unsigned int*)room_find->detector[i-4]>>8);
		}
		else
		{
			sprintf(str_buf[i],"探测%d:",i-3);
		}
	}
	i = show_menu_full_func(str_buf,func_list,para,10,0,row);
	
	if(i < -1)
	{
		int result = edit_save_room_by_text(room, str_buf[i*(-1)-1]);
		if(result<0)
		{
				lcd_fill(0);
				lcd_puts(0,0,"保存失败!",0);
				lcd_puts(0,2,str_buf[i*(-1)-1],0);
				sleep(1000);
				lcd_fill(0);
		}
		if(result == 0)
		{
			lcd_fill(0);
			lcd_puts(0,0,"保存成功!",0);
			sleep(1000);
		}
		row = i*(-1)-1;
		goto redo;
	}
	else if(i == -1)
	{
		find_delete_room(room);
		lcd_fill(0);
		lcd_puts(0,0,"房间已删除!",0);
		sleep(2000);
		return -1;
	}
	return 0;
}
int room_add(void* para)
{
	char str[20];
	int count,i;
	room_t room = {0};
	if(input_room_id(&room) == (-1))
	{
		lcd_fill(0);
		return 0;
	}

	if(find_room(&room)!=0)
	{
		lcd_fill(0);
		lcd_puts(0,0,"该房间已存在!",0);
		sleep(1500);
		show_room_info((room_t*)(find_room(&room)));
		return 0;
	}
	room.room_id[0] = EXIST;
	count = write_room(&room);
	if(count==0)
	{
		lcd_fill(0);
		lcd_puts(0,0,"容量已满!",0);
		sleep(2000);
	}
	else
	{
		lcd_fill(0);
		lcd_puts(0,0,"已添加!",0);
		sleep(1000);
		show_room_info((room_t*)count);
	}
	
	lcd_fill(0);
	return 0;
}
int room_delete(void* para)
{
	char str[20];
	int count;
	room_t room = {0};
	if(input_room_id(&room) == (-1))
	{
		lcd_fill(0);
		return 0;
	}

	if(find_delete_room(&room)==0)
	{
		lcd_fill(0);
		lcd_puts(0,0,"该房间不存在!",0);
		sleep(2000);

	}
	else
	{
		lcd_fill(0);
		lcd_puts(0,0,"已删除!",0);
		sleep(2000);
	}
	lcd_fill(0);
	return 0;
}

int room_query(void* para)
{
	char str[20];
	int count;
	room_t room = {0};
	if(input_room_id(&room) == (-1))
	{
		lcd_fill(0);
		return 0;
	}
	count = find_room(&room);
	if(count==0)
	{
		lcd_fill(0);
		lcd_puts(0,0,"该房间不存在!",0);
		sleep(2000);
		lcd_fill(0);
		return 0;
	}
	show_room_info((room_t*)count);
	lcd_fill(0);
	return 0;
}
int scan_room(void* para)
{
	int count;
	int i;
	unsigned int room_list[200][2];
	int (*func_list[200])(void*);
	room_t*  room[200];
	void *menu;
	redo:
	count = find_all_room(room_list);
	menu = rt_malloc(count*20);
	for(i = 0; i < count; i++)
	{
		char* room_id = (char*)(&room_list[i][1]);
		sprintf(((char*)menu)+20*i,"%d.%d-%d-%d",i+1,room_id[1],room_id[2],room_id[3]);
		func_list[i] = show_room_info;
		room[i] = (room_t*)(room_list[i][0]);
	}
	rt_free(menu);
	if(count > 0)
	{
		i = show_menu(menu,func_list,room,count);
		if(i < 0)
		{
			goto redo;
		}
	}
	else
	{
		lcd_fill(0);
		lcd_puts(0,0,"数据为空!",0);
		sleep(2000);
		lcd_fill(0);
	}
	return 0;
}
int room_manage(void* para)
{
	 const unsigned char menu[][20] = {
				 "1.添加房间  ",
				 "2.删除房间  ",
				 "3.查找房间  ",
				 "4.浏览房间  "
				};
	int (*func_list[4])(void*)  = {
	room_add,
	room_delete,
	room_query,
	scan_room
	};
	show_menu(menu,func_list,0,4);
	return 0;
}