///////////////////////////////////////////////////////
//													 //
//			   @file   KEY.H                   	     //
//													 //
//			   @date   2015.05.28					 //
//													 //
///////////////////////////////////////////////////////

/*
配置定义
*/

#ifndef __KEY_H
#define __KEY_H

#include "config.h"

#define KEY_74HC165_DATA PBin(5)  //数据
#define KEY_74HC165_CLK  PBout(6)  //时钟
#define KEY_74HC165_CPS  PBout(7)  //锁存

typedef enum 	//按键基值
{
	KEY_NULL = 0,	//键值0 
	KEY_NUM1,     //1
	KEY_NUM4,			//2
	KEY_NUM7,			//3
	KEY_NUM0,			//4		0
	KEY_NUM2,			//5
	KEY_NUM5,			//6
	KEY_NUM8,			//7			
	KEY_XING,			//8   *
	KEY_NUM3,			//9
	KEY_NUM6,			//10
	KEY_NUM9,			//11
	KEY_JIA,			//12	+
	KEY_MENU,			//13  
	KEY_LEFT,			//14
	KEY_SEARCH,		//15
	KEY_BACK,			//16
	KEY_RIGHT,		//17
	KEY_REMOVE,		//18
	KEY_ENTER,		//19
}KEY_Typedef;

typedef enum 	//按键数字
{
	Num0 = 0,	//0-9
	Num1,
	Num2,	
	Num3,
	Num4,
	Num5,
	Num6,
	Num7,
	Num8,
	Num9,

}KEY_Numdef;

extern uint8_t KEY_Down_Flag;
extern KEY_Numdef KEY_Out_Num;	//数字

void KEY_Init(void);
uint8_t KEY_Value_Read(void);
void KEY_Board_Out_Num(void);


#define key_board_init KEY_Init
#define key_value_read KEY_Value_Read
#endif







