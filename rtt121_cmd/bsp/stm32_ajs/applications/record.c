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
int show_rec_info(rec_t *rec)
{	
	lcd_fill(0);
	lcd_puts(0,0,"时间:   (按X删除)",0);
	lcd_puts(8,2,rec->date,0);
	lcd_puts(0,4,"房间,探测器:",0);
	lcd_puts(8,6,rec->text,0);
	if(rec->available != EXIST)
	{
		lcd_fill(0);
		lcd_puts(0,0,"不存在!",0);
		sleep(1000);
		lcd_fill(0);
		return ;
	}
	while(1)
	{
		char key = KEY_VALUE;
		sleep(10);
		
		if(key == 'Q')
		{
			lcd_fill(0);
			break;
		}
		if(key == 'C')
		{
			rec_t rec_to;
			memcpy(&rec_to,rec,sizeof(rec_t));
			find_delete_rec(&rec_to);
			lcd_fill(0);
			lcd_puts(0,0,"已删除!",0);
			sleep(1000);
			lcd_fill(0);
			return -1;
		}
	}
	lcd_fill(0);
	return 0;
}
int wipe_rec_ret(void *para)
{
	wipe_all_rec();
	lcd_fill(0);
	lcd_puts(0,0,"已清空!",0);
	sleep(1000);
	lcd_fill(0);
	return -1;
}
int scan_recording(void* para)
{
	int count;
	int i;
	unsigned int rec_list[201][2];
	void (*func_list[201])(void*);
	rec_t*  rec[201];
	void *menu;

	redo:
	count = find_all_rec(rec_list);
	menu = rt_malloc((count+1)*20);
	for(i = 0; i < count; i++)
	{
		char* rec_date = ((rec_t*)rec_list[i][0])->date;
		sprintf(((char*)menu)+20*i,"%d.%s",i+1,rec_date);
		func_list[count - i -1] = show_rec_info;
		rec[count - i -1] = ((rec_t*)rec_list[i][0]);
	}
	func_list[i] = wipe_rec_ret;
	rec[i] = 0;
	sprintf(((char*)menu+20*i),"  [清空记录]");
	rt_free(menu);
	if(count == 0)
	{
		lcd_fill(0);
		lcd_puts(0,0,"无报警记录!",0);
		sleep(2000);
		lcd_fill(0);
		return 0;
	}
	if(show_menu_left(menu,func_list,rec,count+1,0) == -1)
	{
		goto redo;
	}
	return 0;
}