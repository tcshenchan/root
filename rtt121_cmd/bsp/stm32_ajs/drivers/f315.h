///////////////////////////////////////////////////////
//													 //
//			   @file   F315.h                        //
//													 //
//			   @date   2015.03.25					 //
//													 //
///////////////////////////////////////////////////////

#ifndef __F315__H
#define __F315__H

#include "config.h"

void f315_init(void);
extern uint8_t SyncFlag;	// ͬ��������	0 δ��⵽ͬ���� 1��⵽ͬ����
extern uint16_t SyncHigh, SyncLow;	//ͬ����ߵ͵�ƽʱ���¼
extern uint8_t NewReceive[3],OldReceive[3];     //���뻺�� 3���ֽ�24λ��ÿ��λ��ʾһ����ƽ��00��ʾ0,11��ʾ1,01��ʾf��10������
extern char received_315;
#endif
