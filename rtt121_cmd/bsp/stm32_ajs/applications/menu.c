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
char key_table[20] = {0,'1','4','7','0','2','5','8','*','3','6','9','+','S','L','F','Q','R','C','E'};
char focus_on_home_ui;
rt_thread_t main_ui_thread;
int str_to_unicode(char* str,char* str_out, int len, int big_endian)
{
	int i;
	int j = 0;
	for(i = 0; i < len;i++)
	{
		if(str[i] <= 0x80)
		{
			if(big_endian)
			{
				sprintf(str_out+j,"%02x",str[i]);
				str_out[j+2] = '0';
				str_out[j+3] = '0';
			}
			else
			{
				str_out[j] = '0';
				str_out[j+1] = '0';
				sprintf(str_out+j+2,"%02x",str[i]);
			}
			j+=4;
		}
		else
		{
			if(big_endian)
			{
				sprintf(str_out+j,"%02x",str[i]);
				sprintf(str_out+j+2,"%02x",str[i+1]);
			}
			else
			{
				sprintf(str_out+j,"%02x",str[i+1]);
				sprintf(str_out+j+2,"%02x",str[i]);
			}
			i++;
			j+=4;
		}
	}
	str_out[j] = 0;
	return j;
}
int modif_input_panel(const char* title, char* str, int len, int  fix)
{
	int offset = 0;
	int ui_update = 1;
	lcd_fill(0);
	lcd_puts(0,0,title,0);
	if (fix)
	{
		offset = strlen(str);
	}
	else
	{
		offset = 0;
	}
	str[offset] = 0;
	while(1)
	{
		char key = KEY_VALUE;
		if((key >= '0')&&(key<='9'))
		{
			wav_out(8000,wav_data[key-48],2600);
		}
		if(((key >= '0')&&(key<='9'))||(key == '*'))
		{
			str[offset] = key;
			if(offset < len)
			offset++;
			str[offset] = 0;
			ui_update = 1;
		}
		if(key == 'C')
		{
			lcd_puts(8*offset,2," ",0);
			if(offset > 0)
			offset--;
			str[offset] = 0;
			ui_update = 1;
		}
		if(key == 'E')
		{
			lcd_fill(0);
			return offset;
		}
		if(key == 'Q')
		{
			lcd_fill(0);
			return -1;
		}
		if (ui_update)
		{
			lcd_puts(0,2,str,0);
			if(offset<len)
			lcd_puts(8*offset,2,"_",0);
			ui_update = 0;
		}
		sleep(10);
	}
}
int show_menu_left(const char menu[][20],int (*func_list[3])(void*), void* para[], int all_row_count,int x)
{
	unsigned char select = 0;
	unsigned char select_old = 1;
	int row_count;
	int roll_range = 0;
	int select_offset = 0;
	lcd_fill(0);
	
	if(all_row_count > 4)
	{
		row_count = 4;
		roll_range = all_row_count - 4;
	}
	else
	{
		row_count = all_row_count;
	}
	while(1)
	{
		sleep(10);
		char key = KEY_VALUE;
		if(key == 'L')
		{
			if((roll_range == 0)||(select>0))
			{
				select--;if(select == 255)select = row_count - 1;
			}
			else
			{
				select_offset--;
				if(select_offset < 0)
				{
					select_offset = roll_range;
					select = 3;
				}
			}
			select_old = select - 1;
		}
		if(key == 'R')
		{
			if((roll_range == 0)||(select<row_count-1))
			{
				select++;
				select %= row_count;
			}
			else
			{
				select_offset++;
				if(select_offset > roll_range) 
				{
					select_offset = 0;
					select = 0;
				}
			}
			select_old = select - 1;
		}
		
		if((key == 'E')&&(func_list != 0)&&(func_list[select+select_offset]))
		{
			lcd_fill(0);
			if(para!=0)
			{
				int ret = func_list[select+select_offset](para[select+select_offset]);
				if( ret <= -1)
				{
					lcd_fill(0);
					return (ret*(select+select_offset)-1);
				}
			}
			else
			{
				int ret = func_list[select+select_offset](0);
				if( ret <= -1)
				{
					lcd_fill(0);
					return (ret*(select+select_offset)-1);
				}
			}
			select_old = select+1;
		}
		if(key == 'Q')
		{
			lcd_fill(0);
			break;
		}
		if(select != select_old)
		{
			int i;
			lcd_fill(0);
			for(i = 0; i<row_count; i++)
			{
				lcd_puts(x,i*2,menu[i+select_offset],0);
			}
			lcd_puts(x,select*2,menu[select+select_offset],1);
			select_old = select;
		}
	}	
	return 0;
}
int show_menu_full_func(const char menu[][20],int (*func_list[3])(void*), void* para[], int all_row_count,int x,int row)
{
	unsigned char select = 0;
	unsigned char select_old = 1;
	int row_count;
	int roll_range = 0;
	int select_offset = 0;
	row = row%row_count;
	lcd_fill(0);
	
	if(all_row_count > 4)
	{
		row_count = 4;
		roll_range = all_row_count - 4;
	}
	else
	{
		row_count = all_row_count;
	}

	while(row>0)//模拟下翻按键按row次
	{
		if((roll_range == 0)||(select<row_count-1))
		{
			select++;
			select %= row_count;
		}
		else
		{
			select_offset++;
			if(select_offset > roll_range) 
			{
				select_offset = 0;
				select = 0;
			}
		}
		select_old = select - 1;
		row--;
	}
	while(1)
	{
		sleep(10);
		char key = KEY_VALUE;
		if(key == 'L')
		{
			if((roll_range == 0)||(select>0))
			{
				select--;if(select == 255)select = row_count - 1;
			}
			else
			{
				select_offset--;
				if(select_offset < 0)
				{
					select_offset = roll_range;
					select = 3;
				}
			}
			select_old = select - 1;
		}
		if(key == 'R')
		{
			if((roll_range == 0)||(select<row_count-1))
			{
				select++;
				select %= row_count;
			}
			else
			{
				select_offset++;
				if(select_offset > roll_range) 
				{
					select_offset = 0;
					select = 0;
				}
			}
			select_old = select - 1;
		}
		
		if((key == 'E')&&(func_list != 0)&&(func_list[select+select_offset]))
		{
			lcd_fill(0);
			if(para!=0)
			{
				int ret = func_list[select+select_offset](para[select+select_offset]);
				if( ret <= -1)
				{
					lcd_fill(0);
					return (ret*(select+select_offset)-1);
				}
			}
			else
			{
				int ret = func_list[select+select_offset](0);
				if( ret <= -1)
				{
					lcd_fill(0);
					return (ret*(select+select_offset)-1);
				}
			}
			select_old = select+1;
		}
		if(key == 'Q')
		{
			lcd_fill(0);
			break;
		}
		if(select != select_old)
		{
			int i;
			lcd_fill(0);
			for(i = 0; i<row_count; i++)
			{
				lcd_puts(x,i*2,menu[i+select_offset],0);
			}
			lcd_puts(x,select*2,menu[select+select_offset],1);
			select_old = select;
		}
	}	
	return 0;
}
int str_separation(char* src, int dst[],int max_count)
{
	char *str = src;
	int offset = 0;
	int count = 0;

	while(*str!=0)
	{
		if(*src == '*')
		{
			*src = 0;
			dst[count] = atoi(str);
			count ++;
			if(count == max_count)
			{
				break;
			}
			else
			{
				str = src+1;
			}
		}
		else if(*src == 0)
		{
			dst[count] = atoi(str);
			count++;
			break;
		}
		src++;
	}
	lcd_fill(0);
	return count;

}
int auth(void)
{
	char str[20];
	int count,passwd;
	count = input_panel("密码:",str,8);
	if(count == -1)
	{
		lcd_fill(0);
		return 0;
	}
	str[count] = 0;
	passwd = atoi(str);
	if(*(unsigned int*)(PASSWD_ADDR+4) != passwd)
	{
		lcd_puts(0,0,"密码错误!",0);
		sleep(1000);
		lcd_fill(0);
		return 0;
	}
	lcd_fill(0);
	return 1;
	
}
void show_home_ui(void)
{
	int ui_update = 100;

	while(1)
	{
		char key = KEY_VALUE;
		sleep(10);
		
		if(key == 'Q')
		{
			if((alarm_str[0]!=0)&&(strncmp("警告:电源断开!!",alarm_str,8)!=0))
			{
				lcd_fill(0);
				lcd_puts(36,1,"清除报警?",0);
				while(1)
				{
					char key_inside = KEY_VALUE;
					sleep(10);
					if(key_inside == 'E')
					{
						if(auth())
						{
							alarm_led(0);
							alarm_str[0] = 0;
							ext_control(0);
						}
						break;
					}
					else if(key_inside == 'Q')
					{
						break;
					}
				}
			}

		}
		if(key == 'S')
		{
			
			lcd_fill(0);
			break;
		}
		if(key == 'F')
		{
			
			// if(auth()==0)
			// {
			// 	continue;
			// }
			scan_recording(0);
			lcd_fill(0);
			ui_update = 100;
		}
		if(ui_update == 100)
		{
			lcd_fill(0);
			lcd_rssi(0,0,rssi/7);
			if(bat_detecte())
			{
				lcd_bat_level(112,0,5);
			}
			if(dc_detecte())
			{
				lcd_show_charge(106,0);
			}
			if(net_sta_in == 1)
			{
				lcd_puts(36,1,"中国移动",0);
			}
			else if(net_sta_in == 2)
			{
				lcd_puts(36,1,"中国联通",0);
			}
			else
			{
				lcd_puts(36,1,"无网络",0);
			}

			lcd_puts(16,4,"消防网关V0.1",0);
			if(time_str_display)
			{
				lcd_puts(8,6,time_str_display,0);
			}
			if(alarm_str[0])
			{
				
				lcd_puts(0,4,alarm_str+1,0);
				
				if((voice_running == 0)&&(alarm_str[0]&0xf0))
				{
					if((alarm_str[0]&0x0f) == 2)
					{
						wav_out(8000,wav_data[11]+6000,7000);
						alarm_led(1);
						ext_control(1);
					}
					else 
					{
						wav_out(16000,wav_data[12],2000);
					}
				}
			}
			ui_update = 0;
		}
		ui_update++;

	}
}
void menu_thread_entry(void* parameter)
{

	 const unsigned char menu[][20] = {
				 "1.房间管理  ",
				 "2.探测器管理",
				 "3.报警记录  ",
				 "4.设置      "
				};
	void (*func_list[4])(void* para)  = {
	room_manage,
	detector_manage,
	scan_recording,
	settings
	};
	while(1)
	{
		redo:
		focus_on_home_ui = 1;
		show_home_ui();
		focus_on_home_ui = 0;
		if(auth()!=1)
		{
			goto redo;
		}
		show_menu(menu, func_list,0,4);
	}
}
void menu_thread_run(void)
{
			
			main_ui_thread = rt_thread_create("menu",
                                   menu_thread_entry, RT_NULL,
                                   1024*10, 16, 20);
			rt_thread_startup(main_ui_thread);
}