/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2013-07-12     aozima       update for auto initial.
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <board.h>
#include <rtthread.h>
#include <finsh.h>
#include <stdio.h>
#include "led.h"
#include "audio.h"
#include "jlx12864.h"
#include "key.h"
#include "f315.h"
#include "sim900a.h"
extern void menu_thread_run(void);
void power_pin_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = (GPIO_Pin_0|GPIO_Pin_1);
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}
void rt_init_thread_entry(void* parameter)
{
		unsigned int i = 0;
		//shell终端初始化
		{
			extern void finsh_system_init();
			finsh_system_init();
			finsh_set_device(RT_CONSOLE_DEVICE_NAME);
		}
		//外设初始化
		{
			
			lcd_init();
			lcd_puts(1,2,"  2安吉斯网关ABC",0);
			key_board_init();
			f315_init();
			//lcd_puts(0,0,"为什么",0);

		}
		
		power_pin_init();
    //运行后台低优先级任务
		led_thread_run();	
		menu_thread_run();
		gsm_thread_run();	
		listen_thread_run();
		//alarm_thread_run();
		passwd_init();
		
		// while(1)
		// {
		// 	//wav_out(8000,wav_data[i],2600);
		// 	sleep(5000);
		// 	i++;
		// 	i%=11;
		// 	wav_out(8000,wav_data[11],13000);
		// }

}

int rt_application_init(void)
{
    rt_thread_t init_thread;
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   12048, 8, 20);
    rt_thread_startup(init_thread);
	return 0;
}

/*@}*/
