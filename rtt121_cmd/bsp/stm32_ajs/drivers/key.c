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
  * @brief  KEY��ʼ��	
  * @param  None	
  * @retval None 
  */
void KEY_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );//PORTBʱ��ʹ�� 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  //������� 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	//��ʼ��GPIOB
	GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7);  
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  //��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);	//��ʼ��GPIOB
	GPIO_SetBits(GPIOB,GPIO_Pin_5); 
}

/**
  * @brief  KEY��ʼ��	
  * @param  None	
  * @retval None 
  */
uint8_t KEY_Board_Data(void)
{
	uint8_t i=0, j=0;
	uint8_t KEY_VAL = 0;
	uint8_t KEY_TEMP[3] = {0};
	
	KEY_74HC165_CPS = 0;		//ѡͨ165
	KEY_74HC165_CPS = 1;		//��������
	
	for(i=0;i<3;i++)	//3��IC����
	{
		for(j=8;j>=1;j--)
		{
			KEY_74HC165_CLK = 0;	//ʱ��
			if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_5) == 0) break;	//����Ϊ�� �������� 
			KEY_74HC165_CLK = 1;
		}
		
		KEY_TEMP[i] = j;		//�ް������� ֵΪ0		
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

//���尴��״̬
//#define KEY_STATE_INIT       0  
//#define KEY_STATE_WOBBLE     1  
//#define KEY_STATE_PRESS      2  
//#define KEY_STATE_LONG       3  
//#define KEY_STATE_CONTINUE   4  
//#define KEY_STATE_RELEASE    5  

typedef enum 
{
	KEY_STATE_INIT = 0,	//������ֵ
	KEY_STATE_WOBBLE,	//����
	KEY_STATE_PRESS,	//�̰�
	KEY_STATE_LONG,		//����
	KEY_STATE_CONTINUE,	//����
	KEY_STATE_RELEASE,	//�ͷ�

}KEY_Statedef;


uint8_t KEY_Value_Read(void)
{
	uint8_t KEY_VAL_TEMP = 0;
	uint8_t KEY_VAL = KEY_NULL;
	static uint8_t KEY_TIM_CNT = 0;	//��������
	static uint8_t KEY_STATE_NOW = 0;		//����״̬
	static uint8_t NOW_KEY = 0,LAST_KEY = 0;
	
	KEY_VAL_TEMP = KEY_Board_Data();	//��ȡ��������
	
	switch(KEY_STATE_NOW)
	{
		case KEY_STATE_INIT:		//����δ����
			if(KEY_VAL_TEMP != KEY_NULL)  
			{  
				KEY_STATE_NOW = KEY_STATE_WOBBLE ;	//���� 
			} 
			else return 0;
		case KEY_STATE_WOBBLE:		//��������
			sleep(80);
			if(KEY_VAL_TEMP != KEY_NULL)  
				KEY_STATE_NOW = KEY_STATE_PRESS ; //�̰�
			else 
			{	
				KEY_STATE_NOW = KEY_STATE_INIT;			//����δ����
			}
			break;
		case KEY_STATE_PRESS:		//��������
			if(KEY_VAL_TEMP != KEY_NULL)  
			{
				NOW_KEY = KEY_VAL_TEMP;		//����̰���
				KEY_STATE_NOW = KEY_STATE_LONG;	//����
			//	D_printf("�̰���\r\n");
				
			}
			else 
			{
				KEY_STATE_NOW = KEY_STATE_INIT;		//��������δ����
			}
			break;
		case KEY_STATE_LONG:		//��������
			if(KEY_VAL_TEMP != KEY_NULL)
			{
				if(++KEY_TIM_CNT>100)
				{
		//			D_printf("������\r\n");
					NOW_KEY = KEY_VAL_TEMP;		//���泤����
					LAST_KEY = NOW_KEY;
					
				}
			}
			else
			{
				KEY_STATE_NOW = KEY_STATE_RELEASE;	//�����ͷ�
			}
			break;
		case KEY_STATE_RELEASE:		//�����ͷ�
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


















