#include <board.h>
#include <rtthread.h>
#include <finsh.h>
#include <stdio.h>
#include "led.h"

#include "jlx12864.h"
#include "key.h"
#include "f315.h"
#include "gw.h"
#include "audio.h"
#include "sim900a.h"

extern room_t *alarm_room;
void alarm_thread_entry(void *para)
{
	int i;
	while(1)
	{

		
		sleep(100);
	}
}
void alarm_thread_run(void)
{
			rt_thread_t thread;
			thread = rt_thread_create("alarm",
                                   alarm_thread_entry, RT_NULL,
                                   1024*4, 24, 20);
			rt_thread_startup(thread);
}