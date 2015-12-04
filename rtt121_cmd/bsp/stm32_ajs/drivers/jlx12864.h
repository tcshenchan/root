///////////////////////////////////////////////////////
//													 //
//			   @file   12864.H                       //
//													 //
//			   @date   2015.05.22					 //
//													 //
///////////////////////////////////////////////////////


/*
ÅäÖÃ¶¨Òå
*/

#ifndef __JLX12864_H
#define __JLX12864_H

#include "config.h"

#define LCD_RS_High() GPIO_SetBits(GPIOE,GPIO_Pin_13)
#define LCD_RS_Low()  GPIO_ResetBits(GPIOE,GPIO_Pin_13)

#define LCD_CS_High() GPIO_SetBits(GPIOE,GPIO_Pin_14)
#define LCD_CS_Low()  GPIO_ResetBits(GPIOE,GPIO_Pin_14)

#define LCD_RST_High() GPIO_SetBits(GPIOE,GPIO_Pin_15)
#define LCD_RST_Low()  GPIO_ResetBits(GPIOE,GPIO_Pin_15)



//¶ÔÆë·½Ê½
#define GUI_TA_LEFT              (0<<4)		//×ó
#define GUI_TA_HCENTER           (1<<4)		//ÖÐ
#define GUI_TA_RIGHT             (2<<4)		//ÓÒ
#define GUI_JUST_TEXT            (1<<7)

void JLX12864_LCD_Init(void);
void GUI_Fill_Screen(uint8_t Data);
void LCD_Show_Asc_5x8(uint8_t page, uint8_t column, uint8_t Data);
void LCD_Show_Asc_8x16(uint8_t page, uint8_t column, uint8_t Data, uint8_t color);
//uint8_t LCD_Show_CH_12x12(uint8_t page, uint8_t column, uint8_t *Buff, uint8_t color);
void GUI_Dispaly_String(uint8_t page, uint8_t column,uint8_t *str,uint8_t color,uint8_t flag);
//uint8_t LCD_Show_CH_16x16(uint8_t page, uint8_t column, uint16_t UniCode, uint8_t color);
//void GUI_Dispaly_String(uint8_t page, uint8_t column, uint8_t *str, uint8_t color,uint8_t flag);
void LCD_Show_Custom_chr(uint8_t page, uint8_t column, uint8_t mat,uint8_t size);
void DrawInputBox(uint8_t x0,uint8_t y0,uint8_t x1);
void GUI_Dispaly_String_UdLine(uint8_t page, uint8_t column,uint8_t *str);
void Pop_Windows(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1);

void GUI_DispStringAtBar(uint8_t *s,uint8_t x0, uint8_t y0, uint8_t x1,uint8_t Mode,uint8_t color);
void Bar(u8 Item_,u8 ItemNum_);
void GUI_DispStringAtBar1(uint8_t *s,uint8_t x0, uint8_t y0, uint8_t x1,uint8_t Mode,uint8_t color);
#define lcd_init JLX12864_LCD_Init
#define lcd_puts(page,column,str,color) GUI_Dispaly_String(page,column,(uint8_t*)str,color,0)
#define lcd_fill(Data) GUI_Fill_Screen(Data)
void lcd_rssi(int x,int y,int rssi);
void lcd_bat_level(int x,int y,int power_level);
void lcd_show_charge(int x,int y);
#endif





