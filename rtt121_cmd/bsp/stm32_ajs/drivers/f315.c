///////////////////////////////////////////////////////
//													 //
//			   @file   F315.C                        //
//													 //
//			   @date   2015.03.25					 //
//													 //
///////////////////////////////////////////////////////
#include "f315.h"
#include <rtthread.h>
uint8_t SyncFlag = 0;	// ͬ��������	0 δ��⵽ͬ���� 1��⵽ͬ����
uint16_t SyncHigh = 0, SyncLow = 0;	//ͬ����ߵ͵�ƽʱ���¼
uint8_t NewReceive[3],OldReceive[3];     //���뻺�� 3���ֽ�24λ��ÿ��λ��ʾһ����ƽ��00��ʾ0,11��ʾ1,01��ʾf��10������

char received_315 = 0;
static void time1_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);		//ʱ��ʹ��
	
	TIM_DeInit(TIM1);
	
	//TIM2 ��ؼĴ������� Tout= ((TIM_Period+1)*(TIM_Prescaler+1))/Tclk��
	TIM_TimeBaseInitStructure.TIM_Prescaler = 2-1;		//��ʱ�����ֵ ���ü��������С��ÿ��2�����Ͳ���һ�������¼�
	TIM_TimeBaseInitStructure.TIM_Period = 720-1;		//Ԥ��Ƶϵ��Ϊ720-1������������ʱ��Ϊ72MHz/720 = 100kHz
	TIM_TimeBaseInitStructure.TIM_ClockDivision =TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseInitStructure);		//20us�ж�һ��	
	
	//TIM2�ж�����
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;		//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_ClearFlag(TIM1, TIM_FLAG_Update);		//����жϱ�־
	TIM_ARRPreloadConfig(TIM1, DISABLE);		//��ֹԤװ������
	TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);	//��������ж�	
	
	TIM_Cmd(TIM1,ENABLE);	//��ʱ��ʹ��	
}

void f315_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);	//ʱ��ʹ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOA,&GPIO_InitStructure);	//��ʼ��GPIOD.12
	
	GPIO_SetBits(GPIOA,GPIO_Pin_11);	
	
	time1_init();
}

void TIM1_UP_IRQHandler(void)	//20us�ж�һ��
{	
	static uint16_t TimeCnt = 0,StartFlag = 0,RecvCnt = 0;
	static uint8_t NewPinStatus = 0, OldPinStatus = 0;
	rt_interrupt_enter();
	if(TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)	//����жϷ������
	{
		NewPinStatus = PAin(11);
		if(NewPinStatus != OldPinStatus)	//IO״̬�����ı�
		{
			if(NewPinStatus == 0)	//�½���
			{
				if(SyncFlag == 0 && StartFlag == 1)		//δ��⵽ͬ����ߵ�ƽ
				{
					SyncHigh = 	TimeCnt;
				}
			}
			else	//������
			{
				if(SyncFlag == 0)	//	δ��⵽ͬ����
				{
					if(StartFlag == 0)	//��δ��ʼ����
					{
						StartFlag = 1;
						TimeCnt = 0;
					}
					else
					{
						SyncLow = TimeCnt;
						if((SyncLow>SyncHigh*20)&&(SyncLow<SyncHigh*40)&&(SyncHigh>5))
						{
							SyncHigh = (SyncHigh + SyncLow)/32;		//������� 4a
							SyncFlag = 1; RecvCnt = 0;
						}
						else
						{
							SyncFlag = 0; RecvCnt = 0; TimeCnt = 0; StartFlag = 0;
						}
					}
				}
				else	//�Ѿ���⵽ͬ����
				{
					if((SyncHigh*0.5<TimeCnt)&&(TimeCnt<SyncHigh*2.1))	//խ���� 0
					{
						NewReceive[RecvCnt/8] <<= 1;
						NewReceive[RecvCnt/8] += 1;
						RecvCnt++;
					}
					else if((SyncHigh*2.1<TimeCnt)&&(TimeCnt<SyncHigh*4))	//������ 1
					{
						NewReceive[RecvCnt/8] <<= 1;
						RecvCnt++;
					}
					else
					{
						//printf("\r\n315 return\r\n");
						SyncFlag = 0; RecvCnt = 0; TimeCnt = 0; 
						TIM1->SR = (uint16_t)~TIM_FLAG_Update;//�������жϱ��
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
		TIM1->SR = (uint16_t)~TIM_FLAG_Update;//�������жϱ��
	}
	rt_interrupt_leave();
}
