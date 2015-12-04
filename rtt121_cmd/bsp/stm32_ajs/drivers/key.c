///////////////////////////////////////////////////////
//													 //
//			   @file   KEY.C                         //
//													 //
//			   @date   2015.05.28					 //
//													 //
///////////////////////////////////////////////////////
#include "key.h"
#include <rtthread.h>
#include "config.h"
#include "jlx12864.h"
#include "audio.h"
#include "gw.h"
//#include "delay.h"
//#include "gui.h"


/**
  * @brief  KEY初始化	
  * @param  None	
  * @retval None 
  */
void KEY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );//PORTB时钟使能 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //推挽输出 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	//初始化GPIOB
	GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7);  
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  //上拉输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);	//初始化GPIOB
	GPIO_SetBits(GPIOB,GPIO_Pin_5); 
}

/**
  * @brief  KEY初始化	
  * @param  None	
  * @retval None 
  */
uint8_t KEY_Board_Data(void)
{
	uint8_t i=0, j=0;
	uint8_t KEY_VAL = 0;
	uint8_t KEY_TEMP[3] = {0};
	
	KEY_74HC165_CPS = 0;		//选通165
	KEY_74HC165_CPS = 1;		//锁存数据
	
	for(i=0;i<3;i++)	//3个IC级联
	{
		for(j=8;j>=1;j--)
		{
			KEY_74HC165_CLK = 0;	//时钟
			if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5) == 0) break;	//引脚为低 按键按下 
			KEY_74HC165_CLK = 1;
		}
		
		KEY_TEMP[i] = j;		//无按键按下 值为0		
	}
	
	for(i=0;i<3;i++)
	{
		if(KEY_TEMP[i] == 0) 
			KEY_VAL = 0;
		else 	
		{
			KEY_VAL = KEY_TEMP[i]+i*8;
			break;
		}
	}
	
	return KEY_VAL;
	
}

//定义按键状态
//#define KEY_STATE_INIT       0  
//#define KEY_STATE_WOBBLE     1  
//#define KEY_STATE_PRESS      2  
//#define KEY_STATE_LONG       3  
//#define KEY_STATE_CONTINUE   4  
//#define KEY_STATE_RELEASE    5  

typedef enum 
{
	KEY_STATE_INIT = 0,	//按键基值
	KEY_STATE_WOBBLE,	//消抖
	KEY_STATE_PRESS,	//短按
	KEY_STATE_LONG,		//长按
	KEY_STATE_CONTINUE,	//持续
	KEY_STATE_RELEASE,	//释放

}KEY_Statedef;


uint8_t KEY_Value_Read(void)
{
	uint8_t KEY_VAL_TEMP = 0;
	uint8_t KEY_VAL = KEY_NULL;
	static uint8_t KEY_TIM_CNT = 0;	//按键计数
	static uint8_t KEY_STATE_NOW = 0;		//按键状态
	static uint8_t NOW_KEY = 0,LAST_KEY = 0;
	
	KEY_VAL_TEMP = KEY_Board_Data();	//获取按键数据
	
	switch(KEY_STATE_NOW)
	{
		case KEY_STATE_INIT:		//按键未按下
			if(KEY_VAL_TEMP != KEY_NULL)  
			{  
				KEY_STATE_NOW = KEY_STATE_WOBBLE ;	//消抖 
			} 
			else return 0;
		case KEY_STATE_WOBBLE:		//按键消抖
			sleep(80);
			if(KEY_VAL_TEMP != KEY_NULL)  
				KEY_STATE_NOW = KEY_STATE_PRESS ; //短按
			else 
			{	
				KEY_STATE_NOW = KEY_STATE_INIT;			//按键未按下
			}
			break;
		case KEY_STATE_PRESS:		//按键按下
			if(KEY_VAL_TEMP != KEY_NULL)  
			{
				NOW_KEY = KEY_VAL_TEMP;		//保存短按键
				KEY_STATE_NOW = KEY_STATE_LONG;	//长按
			//	D_printf("短按！\r\n");
				
			}
			else 
			{
				KEY_STATE_NOW = KEY_STATE_INIT;		//按键按键未按下
			}
			break;
		case KEY_STATE_LONG:		//按键长按
			if(KEY_VAL_TEMP != KEY_NULL)
			{
				if(++KEY_TIM_CNT>100)
				{
		//			D_printf("长按！\r\n");
					NOW_KEY = KEY_VAL_TEMP;		//保存长按键
					LAST_KEY = NOW_KEY;
					
				}
			}
			else
			{
				KEY_STATE_NOW = KEY_STATE_RELEASE;	//按键释放
			}
			break;
		case KEY_STATE_RELEASE:		//按键释放
			{
				
				LAST_KEY =	NOW_KEY;
				KEY_VAL = LAST_KEY;
				
				NOW_KEY = 0;
				LAST_KEY = 0;
				KEY_TIM_CNT = 0;
				
				KEY_STATE_NOW = KEY_STATE_INIT;
				
				
			}
			break;
	}
	if(KEY_VAL > 0)
	{
		alarm_str[0] &= 0x0f;
		back_led_on();
		wav_out(8000,wav_data[12],2400);
	}
	return 	KEY_VAL;
}


















