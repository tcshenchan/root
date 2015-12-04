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
extern uint8_t SyncFlag;	// 同步码检测标记	0 未检测到同步码 1检测到同步码
extern uint16_t SyncHigh, SyncLow;	//同步码高低电平时间记录
extern uint8_t NewReceive[3],OldReceive[3];     //解码缓冲 3个字节24位，每两位表示一个电平，00表示0,11表示1,01表示f，10不存在
extern char received_315;
#endif
