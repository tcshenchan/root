///////////////////////////////////////////////////////
//													 //
//			   @file   F315.C                        //
//													 //
//			   @date   2015.03.25					 //
//													 //
///////////////////////////////////////////////////////
#include "f315.h"
#include <rtthread.h>
uint8_t SyncFlag = 0;	// 同步码检测标记	0 未检测到同步码 1检测到同步码
uint16_t SyncHigh = 0, SyncLow = 0;	//同步码高低电平时间记录
uint8_t NewReceive[3],OldReceive[3];     //解码缓冲 3个字节24位，每两位表示一个电平，00表示0,11表示1,01表示f，10不存在

char received_315 = 0;
static void time1_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);		//时钟使能
	
	TIM_DeInit(TIM1);
	
	//TIM2 相关寄存器配置 Tout= ((TIM_Period+1)*(TIM_Prescaler+1))/Tclk；
	TIM_TimeBaseInitStructure.TIM_Prescaler = 2-1;		//定时器溢出值 设置计数溢出大小，每计2个数就产生一个更新事件
	TIM_TimeBaseInitStructure.TIM_Period = 720-1;		//预分频系数为720-1，这样计数器时钟为72MHz/720 = 100kHz
	TIM_TimeBaseInitStructure.TIM_ClockDivision =TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStructure);		//20us中断一次	
	
	//TIM2中断配置
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;		//先占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//从优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_ClearFlag(TIM1, TIM_FLAG_Update);		//清除中断标志
	TIM_ARRPreloadConfig(TIM1, DISABLE);		//禁止预装缓存器
	TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);	//允许更新中断	
	
	TIM_Cmd(TIM1,ENABLE);	//定时器使能	
}

void f315_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	//时钟使能
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&GPIO_InitStructure);	//初始化GPIOD.12
	
	GPIO_SetBits(GPIOA,GPIO_Pin_11);	
	
	time1_init();
}

void TIM1_UP_IRQHandler(void)	//20us中断一次
{	
	static uint16_t TimeCnt = 0,StartFlag = 0,RecvCnt = 0;
	static uint8_t NewPinStatus = 0, OldPinStatus = 0;
	rt_interrupt_enter();
	if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)	//检测中断发生与否
	{
		NewPinStatus = PAin(11);
		if(NewPinStatus != OldPinStatus)	//IO状态发生改变
		{
			if(NewPinStatus == 0)	//下降沿
			{
				if(SyncFlag == 0 && StartFlag == 1)		//未检测到同步码高电平
				{
					SyncHigh = 	TimeCnt;
				}
			}
			else	//上升沿
			{
				if(SyncFlag == 0)	//	未检测到同步码
				{
					if(StartFlag == 0)	//还未开始计数
					{
						StartFlag = 1;
						TimeCnt = 0;
					}
					else
					{
						SyncLow = TimeCnt;
						if((SyncLow>SyncHigh*20)&&(SyncLow<SyncHigh*40)&&(SyncHigh>5))
						{
							SyncHigh = (SyncHigh + SyncLow)/32;		//引导码高 4a
							SyncFlag = 1; RecvCnt = 0;
						}
						else
						{
							SyncFlag = 0; RecvCnt = 0; TimeCnt = 0; StartFlag = 0;
						}
					}
				}
				else	//已经检测到同步码
				{
					if((SyncHigh*0.5<TimeCnt)&&(TimeCnt<SyncHigh*2.1))	//窄脉冲 0
					{
						NewReceive[RecvCnt/8] <<= 1;
						NewReceive[RecvCnt/8] += 1;
						RecvCnt++;
					}
					else if((SyncHigh*2.1<TimeCnt)&&(TimeCnt<SyncHigh*4))	//宽脉冲 1
					{
						NewReceive[RecvCnt/8] <<= 1;
						RecvCnt++;
					}
					else
					{
						//printf("\r\n315 return\r\n");
						SyncFlag = 0; RecvCnt = 0; TimeCnt = 0; 
						TIM1->SR = (uint16_t)~TIM_FLAG_Update;//清除溢出中断标记
						rt_interrupt_leave();
						return;
					}
					if(RecvCnt>23)
					{
						SyncFlag = 0; RecvCnt = 0; TimeCnt = 0; StartFlag = 0;
						SyncHigh = 0; SyncLow = 0;
						if((NewReceive[0] == OldReceive[0])&&(NewReceive[1] == OldReceive[1])&&(NewReceive[2] == OldReceive[2]))
						{
							//printf("\r\n 315 data receive success!  0x%X 0x%X 0x%X\r\n",NewReceive[0],NewReceive[1],NewReceive[2]);
							received_315 = 1;
						}
						else
						{
							OldReceive[0] = NewReceive[0];OldReceive[1] = NewReceive[1];OldReceive[2] = NewReceive[2];
						}
					}
				}
			}
			
			TimeCnt = 0;
			OldPinStatus = NewPinStatus;
		}
		else
		{
			TimeCnt++;
			if(TimeCnt>500) TimeCnt = 0;
		}
		TIM1->SR = (uint16_t)~TIM_FLAG_Update;//清除溢出中断标记
	}
	rt_interrupt_leave();
}
