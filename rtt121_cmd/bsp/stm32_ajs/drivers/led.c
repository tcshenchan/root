/*
 * File      : led.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2009, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */
#include <rtthread.h>
#include <stm32f10x.h>
#include "config.h"
#include "jlx12864.h"
#include "key.h"
#include "gw.h"
int display_flag = 0;//显示记录状况标志，为1时，每隔1s会显示一次记录大小和CPU占用率。

void rt_hw_led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOE,ENABLE);
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = (GPIO_Pin_9|GPIO_Pin_8|GPIO_Pin_7);
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = (GPIO_Pin_12);
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    pulse_led(0) ;
	alarm_led(0) ;
	breakdown_led(0) ;
	ext_control(0);
}
int cpu_use_time = 0;
rt_timer_t s_timer;
float percent;
static void delay1ms(void)
{
	volatile uint32_t t, t2;
	
	for(t2 = 0; t2 < 1; t2++)
	for(t = 0; t < 0x1800; t++)
	{
		;
	}
}
#include <stdio.h>
void rt_timer_out(void* parameter)
{
	percent = cpu_use_time;
	cpu_use_time = 0;
	rt_timer_start(s_timer);

}
void cpu_counter(void)
{
	cpu_use_time += 1;
	delay1ms();	
}

void led_thread_entry(void* parameter)
{

	rt_hw_led_init();
	rt_thread_idle_sethook(cpu_counter);
	s_timer = rt_timer_create("s_timer", rt_timer_out, 0, 1000, 0);
	rt_timer_start(s_timer);
    while (1)
    {
		char str[100];
		sleep(500);
		pulse_led(0)	;
		sleep(500);
		pulse_led(1)	;
		if(display_flag)
		{
			
			percent = (1000-percent)/10;
			sprintf(str, "CPU:%2.1f%%\r\n", percent);	
			rt_kprintf(str);	
		}
		{

			if(back_led_life>-300)
			{
				back_led_life--;
			}
			if(back_led_life <= 0)
			{
				back_led_off();
			}
			if((back_led_life  <= -200)&&(focus_on_home_ui==0))
			{
				back_led_life = 0;
				rt_thread_delete(main_ui_thread);
				sleep(300);
				//rt_thread_startup(main_ui_thread);
				menu_thread_run();
			}

		}
			
    }
}
void led_thread_run(void)
{
			rt_thread_t led_thread;
			led_thread = rt_thread_create("led",
                                   led_thread_entry, RT_NULL,
                                   2048, 30, 20);
			rt_thread_startup(led_thread);
}
#include <finsh.h>
void kill(int argc, char *argv[])
{
					rt_thread_t thread;
					if(argc != 2)
					{
						rt_kprintf("error argc!\r\n");
						return;
					}
					rt_kprintf("to kill %s\r\n", argv[1]);
					thread = rt_thread_find(argv[1]);
					if(thread != NULL)
					{
						rt_thread_delete(thread);
					}
					else
					{
						rt_kprintf("thread '%s' not exist!\r\n", argv[1]);
					}
}
MSH_CMD_EXPORT(kill, kill the thread with name.)



void look(void){ display_flag = !display_flag; }

MSH_CMD_EXPORT(look,  look cpu and data size.)


#if defined(__CC_ARM)
/*keil*/
 
__asm__  void reset(void)
{
  MOV R0, #1          
  MSR FAULTMASK, R0   
  LDR R0, =0xE000ED0C  
  LDR R1, =0x05FA0004   
  STR R1, [R0]           
 
deadloop
    B deadloop       
}

#else

/* gcc */
void reset(void)
{
	__asm__ __volatile__(
	"MOV R0, #1;"
	"MSR FAULTMASK, R0 ;"
	"LDR R0, =0xE000ED0C  ;"
  "LDR R1, =0x05FA0004  ;" 
  "STR R1, [R0] ;"          
	"deadloop:;"
  "B deadloop ;" 
	);
}
#endif
MSH_CMD_EXPORT(reset, soft reset.)

