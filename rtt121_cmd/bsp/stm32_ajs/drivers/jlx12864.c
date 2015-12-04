///////////////////////////////////////////////////////
//													 //
//			   @file   12864.C                       //
//													 //
//			   @date   2015.05.22					 //
//													 //
///////////////////////////////////////////////////////
#include "jlx12864.h"
#include "usart.h"
#include "font_a.H"

/**
  * @brief  LCDÑÓÊ±
  * @param  dat 	
  * @retval None 
  */
 int back_led_life = 0;
static void Delayms(uint16_t ms)
{
    uint8_t j;
    while(ms--)
        for(j=0; j<=20; j++);
}


/**
  * @brief  SPI1³õÊ¼»¯	
  * @param  None	
  * @retval None 
  */
static void Gpio_Spi1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOE, ENABLE);	//GPIOA GPIOEÊ±ÖÓ
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_SPI1, ENABLE);	//SPI1Ê±ÖÓ
	
	// PE13/14/15 ÍÆÍìÊä³ö
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15 ;	//GPIO
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitStructure);
	
	GPIO_SetBits(GPIOE,GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15);
	
	// PA5/7¸´ÓÃÍÆÍìÊä³ö  Ó²¼þSPI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5| GPIO_Pin_7 ;	// GPIO_Pin_6  SPI
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	/*12864 back_LED*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	GPIO_SetBits(GPIOA,GPIO_Pin_4);
	
	GPIO_SetBits(GPIOA,GPIO_Pin_5| GPIO_Pin_7);
	//SPI init
	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;	//SPIµ¥Ïò·¢ËÍ	
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA =  SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 2;
	SPI_Init(SPI1,&SPI_InitStructure);
	
	SPI_Cmd(SPI1, ENABLE);	//Ê¹ÄÜSPI1ÍâÉè
}

/**
  * @brief  SPI1ËÙ¶ÈÉèÖÃ	
  * @param  None	
  * @retval None 
  */
void Spi1_SetSpeed(uint8_t SPI_BaudRatePrescaler)
{
	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	
	SPI1->CR1 &= 0xFFC7;
	SPI1->CR1 |= SPI_BaudRatePrescaler;
	
	SPI_Cmd(SPI1, ENABLE);	//Ê¹ÄÜSPI1ÍâÉè
}


/**
  * @brief  LCDÐ´Ö¸Áî
  * @param  Data	
  * @retval None 
  */
static void LCD_Send_Command(uint8_t Data)   
{
	uint8_t i = 0;
	
    LCD_RS_Low();

	SPI1->DR = Data;

	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE) == RESET)		//µÈ´ý·¢ËÍ»º´æÎª¿Õ
	{
		i++;
		if(i>200) return;
	}
}

/**
  * @brief  LCDÐ´Êý¾Ý
  * @param  Data	
  * @retval None 
  */
static void LCD_Send_Data(uint8_t Data)
{
	uint8_t i = 0;
	
    LCD_RS_High();

	SPI1->DR = Data;

	while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE) == RESET)		//µÈ´ý·¢ËÍ»º´æÎª¿Õ
	{
		i++;
		if(i>200) return;
	}
}


/**
  * @brief  LCDÈ«ÆÁÌî³ä
  * @param  Data	0x00 £¨ÇåÆÁ²»ÏÔÊ¾£© 0xff£¨Ìî³äÈ«ÏÔÊ¾£©
  * @retval None 
  */
void GUI_Fill_Screen(uint8_t Data)
{
	uint8_t i,j;
	
 	LCD_CS_Low();
 	for(i=0;i<8;i++)
	{
		LCD_Send_Command((uint8_t)0xb0+i);
		LCD_Send_Command(0x10);
		LCD_Send_Command(0x04);
		for(j=0;j<128;j++)
		{
		  	LCD_Send_Data(Data);
		}
	}
 	LCD_CS_High();
}

/**
  * @brief  LCD³õÊ¼»¯
  * @param  None	
  * @retval None 
  */
void JLX12864_LCD_Init(void)
{
	Gpio_Spi1_Init();	/*SPI1 ³õÊ¼»¯*/
	
	LCD_CS_Low();
	LCD_RST_Low();
    Delayms(20);
	LCD_RST_High();
    Delayms(20);
   	
    LCD_Send_Command(0xe2); /*Èí¸´Î»*/
	Delayms(5);
    LCD_Send_Command(0x2c); /*ÉýÑ¹²½¾Û 1*/
	Delayms(5);
    LCD_Send_Command(0x2e); /*ÉýÑ¹²½¾Û 2*/
	Delayms(5);
    LCD_Send_Command(0x2f); /*ÉýÑ¹²½¾Û 3*/
	Delayms(5);
	LCD_Send_Command(0x24);  /*´Öµ÷¶Ô±È¶È£¬¿ÉÉèÖÃ·¶Î§0x20¡«0x27*/
	LCD_Send_Command(0x81);  /*Î¢µ÷¶Ô±È¶È*/
	LCD_Send_Command(0x1e);  /*0x1a,Î¢µ÷¶Ô±È¶ÈµÄÖµ£¬¿ÉÉèÖÃ·¶Î§0x00¡«0x3f*/
	LCD_Send_Command(0xa2);  /*1/9Æ«Ñ¹±È£¨bias£©*/
	LCD_Send_Command(0xc0);  /*ÐÐÉ¨ÃèË³Ðò£º´ÓÉÏµ½ÏÂ*/
	LCD_Send_Command(0xa1);  /*ÁÐÉ¨ÃèË³Ðò£º´Ó×óµ½ÓÒ*/
//	LCD_Send_Command(0x40);  /*ÆðÊ¼ÐÐ£ºµÚÒ»ÐÐ¿ªÊ¼*/
//   LCD_Send_Command(0xa7); /*·´ÏÔ*/
	LCD_Send_Command(0xaf);  /*¿ªÏÔÊ¾*/
	LCD_CS_High();
	
	GUI_Fill_Screen(0x00);	//ÇåÆÁ
	D_printf("12864³õÊ¼»¯Íê³É£¡\r\n");
}

/**
  * @brief  LCDÖ¸¶¨Î»ÖÃÏÔÊ¾Ö¸¶¨Êý¾Ý
  * @param  page 0-7Ò³ 	column 0-127ÁÐ DataÏÔÊ¾Êý¾Ý
  * @retval return 0 ÏÔÊ¾Ê§°Ü 1ÏÔÊ¾³É¹¦ 
  */
static uint8_t LCD_Specify_Display(uint8_t page, uint8_t column, uint8_t Data)
{
	if(page>7 || column>127) return false;	//³¬³öÏÔÊ¾·¶Î§
	
	column += 4;
	LCD_CS_Low();	
	
	LCD_Send_Command(0xb0+page);   /*ÉèÖÃÒ³µØÖ·*/
	LCD_Send_Command(0x10+(column>>4&0x0f));	/*ÉèÖÃÁÐµØÖ·µÄ¸ß4Î»*/
	LCD_Send_Command(column&0x0f);	/*ÉèÖÃÁÐµØÖ·µÄµÍ4Î»*/
	LCD_Send_Data(Data);
	
	LCD_CS_High();
	
	return true; 
}

/**
  * @brief  LCDÖ¸¶¨Î»ÖÃÏÔÊ¾Ò»¸ö5ÁÐ8ÐÐµÄÊý×ÖºÍ·ûºÅ
  * @param  page 0-7Ò³ 	column 0-127ÁÐ DataÏÔÊ¾Êý¾Ý ²»Ö§·´ÏÔ
  * @retval None
  */
void LCD_Show_Asc_5x8(uint8_t page, uint8_t column, uint8_t Data)
{
	uint8_t i,TempData;
	
	for(i=0;i<5;i++)
	{
		TempData = chardot_5x8[i+(Data-0x20)*5];
		
		LCD_Specify_Display(page,column,TempData);
		column++;
	}
}

/**
  * @brief  LCDÖ¸¶¨Î»ÖÃÏÔÊ¾Ò»¸ö8ÁÐ16ÐÐµÄÊý×ÖºÍ·ûºÅ
  * @param  page 0-7Ò³ 	column 0-127ÁÐ DataÏÔÊ¾Êý¾Ý color 0 Õý³£ÏÔÊ¾ 1 ·´ÏÔ
  * @retval None
  */
void LCD_Show_Asc_8x16(uint8_t page, uint8_t column, uint8_t Data, uint8_t color)
{
	uint8_t i,TempData;
	
	for(i=0;i<8;i++)
	{
		if(!color)	
			TempData = chardot_8x16[i+(Data-0x20)*16];
		else
			TempData = ~chardot_8x16[i+(Data-0x20)*16];
		LCD_Specify_Display(page,column,TempData);
		column++;
	}
	column -= 8;
	for(i=8;i<16;i++)
	{
		if(!color)	
			TempData = chardot_8x16[i+(Data-0x20)*16];
		else
			TempData = (~chardot_8x16[i+(Data-0x20)*16])&0x1f;
		
		LCD_Specify_Display(page+1,column,TempData);
		column++;
	}
}
/*
*****************************************************************************
* Display_ASCII_UdLine - ÏÔÊ¾Ò»¸ö´øÓÐÏÂ»®ÏßµÄASCII
* DESCRIPTION: -
*
* @Param X:X 0-127ÁÐ 
* @Param Y:Y 0-7Ò³ 
* @Param Char:ASCIIÂë
* @Return :
* ----
*****************************************************************************
*/
void LCD_Show_Asc_8x16_UdLine(uint8_t Y, uint8_t X,uint8_t Char)
{
	
  	uint8_t  i, ImgData;
  	uint8_t  HZ_X = X;
	
        for(i=0;i<8;i++)
		{
            ImgData = chardot_8x16[i+(Char-0x20)*16];     
            LCD_Specify_Display( Y,HZ_X,ImgData);
            HZ_X++;
        }
        HZ_X=X;
        for(i=8;i<16;i++)
		{
            ImgData = chardot_8x16[i+(Char-0x20)*16] | 0x80;
            LCD_Specify_Display(Y + 1,HZ_X,ImgData);
            HZ_X++;
        }
}

void GUI_Dispaly_String_UdLine(uint8_t page, uint8_t column,uint8_t *str)
{
	uint8_t  i=0;
	
	while(*(str+i)!='\0')
	{
		if(*(str+i)<0x80)	//ASCII
		{
			if(str[i] == '\n')
			 {
					column = 0;
					page += 2;
			 } 
			 else 
			 {
				if(column > 128-1)	//³¬³öÏÔÊ¾ »»ÐÐ
				{
					column = 0;
					page += 2;
				}
		
				LCD_Show_Asc_8x16_UdLine(page,column,str[i]);
				column +=8;

			}
		}
		else return;
		
	str++;
	}
}

/**
  * @brief  LCDÖ¸¶¨Î»ÖÃÏÔÊ¾Ò»¸ö12ÁÐ12ÐÐµÄºº×Ö
  * @param  page 0-7Ò³ 	column 0-127ÁÐ Buff ºº×ÖÊý×é color 0 Õý³£ÏÔÊ¾ 1 ·´ÏÔ
  * @retval return 0 ÏÔÊ¾Ê§°Ü 1ÏÔÊ¾³É¹¦ 
  */
uint8_t LCD_Show_CH_12x12(uint8_t page, uint8_t column, uint8_t *Buff, uint8_t color) 
{
	uint8_t i,TempData;
		
	for(i=0;i<12;i++)
	{
		if(!color)	
			TempData = Buff[i]; 
		else
			TempData = ~Buff[i];
		LCD_Specify_Display(page,column,TempData);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
		column++;
	}
	column -= 12;
	for(i=12;i<24;i++)
	{
		if(!color)	
			TempData = Buff[i]; 
		else
			TempData = ~Buff[i];
		LCD_Specify_Display(page+1,column,TempData&0x1f);     //0x1f ½â¾ö·´ÏÔÊ±ºº×ÖÊúÏòÁ¬½Ó                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
		column++;
	}
	
	return true;
}

/**
  * @brief  LCDÖ¸¶¨Î»ÖÃÏÔÊ¾×Ö·û´®
  * @param  page 0-7Ò³ 	column 0-127ÁÐ str ÏÔÊ¾×Ö·û´® color 0 Õý³£ÏÔÊ¾ 1 ·´ÏÔ flag 0ÏÔ Ê¾8x16ASC 1 ÏÔÊ¾5x8ASC
  * @retval None
  */

void GUI_Dispaly_String(uint8_t column, uint8_t page, uint8_t *str,uint8_t color,uint8_t flag)
{
	uint8_t i=0;
	uint8_t area_code,bit_code;
	uint32_t hanzi_addr;
	
	while(*(str+i)!='\0')
	{
		if(*(str+i)<0x80)	//ASCII
		{
			if(str[i] == '\n')
			 {
					column = 0;
					page += 2;
			 } 
			 else 
			 {
				if(column > 128-1)	//³¬³öÏÔÊ¾ »»ÐÐ
				{
					column = 0;
					page += 2;
				}
				if(flag==0)
				{
					LCD_Show_Asc_8x16(page,column,str[i],color);
					column += 8;
				}
				else
				{
					LCD_Show_Asc_5x8(page,column,str[i]);
					column += 5;
				}
			}
		}
		else	//ºº×Ö
		{
			extern const unsigned char hz12_font[];
			area_code=*(str+i*2);
			bit_code =*(str+i*2+1);
			hanzi_addr=((area_code-0xB0)*94+(bit_code-0xA1))*24;	//¼ÆËãºº×ÖµØÖ·		
			LCD_Show_CH_12x12(page,column,(uint8_t*)(&hz12_font[hanzi_addr]),color);
			str++;column+=12;
		}
		
		str++;
	}
}




/**
  * @brief  LCDÖ¸¶¨Î»ÖÃÏÔÊ¾Ò»¸ö×Ô¶¨Òå8x8 »ò 16x16×Ö·û
  * @param  page 0-7Ò³ 	column 0-127ÁÐ DataÏÔÊ¾Êý¾Ý ²»Ö§·´ÏÔ
  * @retval None
  */
void LCD_Show_Custom_chr(uint8_t page, uint8_t column, uint8_t mat,uint8_t size)
{
	uint8_t i,TempData;
	
	if(size == 0)
	{
		for(i=0;i<8;i++)
		{
			TempData = chardot_8x8[mat][i];
			
			LCD_Specify_Display(page,column,TempData);
			column++;
		}
	}
	else
	{
//		for(i=0;i<16;i++)
//		{
//			TempData = chardot_8x8[mat][i];
//			
//			LCD_Specify_Display(page,column,TempData);
//			column++;
//		}
//		for(i=16;i<32;i++)
//		{
//			TempData = chardot_8x8[mat][i];
//			
//			LCD_Specify_Display(page,column,TempData);
//			column++;
//		}
	}
		
}



/*
*****************************************************************************
* Pop_Windows - µ¯³ö´°¿Ú
* DESCRIPTION: -
* @Param x0:µ¯´°ÆðÊ¼XÖá×ø±ê
* @Param x1:µ¯´°½áÊøµãµÄXÖá×ø±ê
* @Param y0:µ¯´°µÄYÖá×ø±ê
* @Param y1:µ¯´°½áÊøµãµÄYÖá×ø±ê
* @Return :
* ----  Pop_Windows(8,1,120,6);
*****************************************************************************
*/
void Pop_Windows(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1)
{
	uint8_t i,j;

	/*********XÖá»æÖÆÏß¶Î*********/
	for(i=x0+6;i<=x1-3;i++) 	LCD_Specify_Display(y0,i,0xc0);
	for(i=x0+6;i<=x1-3;i++) 	LCD_Specify_Display(y1,i,0x1f);
	
	for(i=x0+8;i<=x1-8;i++)
		for(j=y0+1;j<=y1-1;j++) 	
			LCD_Specify_Display(j,i,0x00);

	/*********YÖá»æÖÆÏß¶Î*********/
	for(i=x0;i<x0+6;i++)
		for(j=y0;j<=y1;j++) 	
			LCD_Specify_Display(j,i,0x00);
	for(i=x0+6;i<x0+8;i++)
		for(j=y0+1;j<=y1-1;j++) 	
			LCD_Specify_Display(j,i,0xff);
	for(i=x1-8;i<=x1-3;i++)
		for(j=y0+1;j<=y1-1;j++) 	
			LCD_Specify_Display(j,i,0xff);
	for(i=x1-3;i<=x1;i++)
		for(j=y0;j<=y1;j++) 	
			LCD_Specify_Display(j,i,0x00);
}

/*
*****************************************************************************
* GUI_DispStringAtBar - ÏÔÊ¾Ò»¸ö×Ö·û´®ÔÚÒ»¸öBARÖÐ
* DESCRIPTION: -
* ¿ÉÒÔÉè¶¨¶ÔÆë·½Ê½
* @Param s:×Ö·û´®
* @Param x0:BARÆðÊ¼XÖá×ø±ê
* @Param x1:BAR½áÊøµãµÄXÖá×ø±ê
* @Param y0:BARµÄYÖá×ø±ê
* @Param Mode:¶ÔÆë·½Ê½ ¾ÓÖÐÓÚBAR ×ó¶ÔÆëBAR ÓÒ¶ÔÆëBAR
* @Param color: 0 Õý³£ÏÔÊ¾ 1 ·´ÏÔÊ¾
* @Return :
* ----
*****************************************************************************
*/

void GUI_DispStringAtBar(uint8_t *s,uint8_t x0, uint8_t y0, uint8_t x1,uint8_t Mode,uint8_t color)
{
	uint8_t ENCount = 0;	//Ó¢ÎÄÍ³¼Æ
	uint8_t CNCount = 0;	//ÖÐÎÄÍ³¼Æ

	uint8_t XLen = 0,XlenTmp = 0;
	uint8_t X = 0, Y = 0, i = 0;


	while(s[i]!='\0')	//Í³¼ÆÖÐÓ¢ÎÄ¸öÊý
	{
		if (s[i] > 0x80) 
		{
			CNCount++;
			i++;
		} 
		else 
		{
			ENCount++;
		}
		i++;
	}
	
	XLen = ENCount*8 + CNCount*12;		//¼ÆËã×Ö·û×Ü³¤¶È
	
	if(XLen > 127) XLen = 127;	//Ö»ÄÜÏÔÊ¾Ò»ÐÐ
	
	XlenTmp = XLen;
	
	switch(Mode & 0x30)	//¶ÔÆë·½Ê½
	{
		 case GUI_TA_LEFT: //×ó¶ÔÆë
            XLen = 0;
            break;
        case GUI_TA_HCENTER:  //¾ÓÖÐ
            XLen  = ((x1-x0)-XLen)/2;
            break;
        case GUI_TA_RIGHT:   //ÓÒ¶ÔÆë
            XLen  = (x1-x0)-XLen;
            break;
        default:
            break;
	}
//--------------------------------------------------//
//×ø±ê	
	X = x0 + XLen;		//»ñµÃ¶ÔÆëÏÔÊ¾ÆðÊ¼×ø±ê
	Y = y0;
//--------------------------------------------------//
//Çå¿ÕÏÔÊ¾Ç°¿Õ°×
	if(Mode & GUI_JUST_TEXT)	//Çå¿Õ×Ö·ûÇ°Ãæ¿Õ°×²¿·Ö
	{	}
	else
	{
		if(color)	//·´ÏÔ
		{
			for(i=0;i<X;i++)
			{
				LCD_Specify_Display(y0,i,0xFF);
				LCD_Specify_Display(y0+1,i,0xFF);
			}
		}
		else
		{
			for(i=0;i<X;i++)
			{
				LCD_Specify_Display(y0,i,0x00);
				LCD_Specify_Display(y0+1,i,0x00);
			}
		}
	
	}
	
//--------------------------------------------------//
//ÏÔÊ¾
	
	GUI_Dispaly_String(Y,X,s,color,0);	//ÏÔÊ¾×Ö·û´®
//--------------------------------------------------//
//Çå¿ÕÏÔÊ¾ºó¿Õ°×
	if(Mode&GUI_JUST_TEXT)
	{
//		GUI_SetEnFont(En_5x8);
		return;
	} 
	else
	{
		X += XlenTmp;
		if(color)	//·´ÏÔ XLen
		{
			for(i=X;i<x1;i++)
			{
				LCD_Specify_Display(y0,i,0xFF);
				LCD_Specify_Display(y0+1,i,0xFF);
			}
		}
		else
		{
			for(i=X; i<x1;i++)
			{
				LCD_Specify_Display(y0,i,0x00);
				LCD_Specify_Display(y0+1,i,0x00);
			}
		}
//	  GUI_SetEnFont(En_5x8);
		return;
	}
}


//--------------
/*
BarµÄËã·¨
    ___
 |   |
 |   |
| |<-|-----BarLen
 |   L
 |   |
 |   |
 |  _|_
BarµÄ»¬¶¯¾àÀëÊÇL-BarLen
ÎªÁËÃÀ¹Û,¿ÉÒÔÔÚ¿ªÊ¼ºÍ½áÎ²²¿·Ö¶àÁ÷³öÀ´Ò»Ð©µã,ÄÇÃ´»¬¶¯¾àÀëÒª¿Û³ýÕâÐ©µãµÄ³¤¶È,²¢ÔÚ¼ÆËã½á¹û
µÃµ½0µÄÊ±ºò,Ìí¼ÓÉÏÉÏ¶ËÒªÁô³öÀ´µÄµãBarRemainDot
2ÖÖÏÔÊ¾·½Ê½:
Ò»ÖÖÊÇBarLenÊÇ¶¨³¤µÄ,
Ò»ÖÖBarLenÊÇ¸ù¾ÝÏÔÊ¾×Ü¹²µÄÏîÊý¶¨ÏÂÀ´µÄ
*/


//--------------
//BarµÄ³¤¶È
//Ô¤Áô³öÀ´µÄµã
//#define BarRemainDot  3
//Êý×ÖÏÔÊ¾Î»ÖÃ
#define BarNumPosX 120
#define BarNumPosY 0
//BarµÄÏÔÊ¾¿ªÊ¼/½áÊøÎ»ÖÃ
#define BarBeginPosX 	(15*8)
#define BarEndPosX 		(16*8)
#define BarBeginPosY 	(1*8)
#define BarEndPosY 		(8*8)

const uint8_t BarCode[]={0x00,0x00,0x00,0xff,0xff,0x00,0x00,0x00,};
//const uint8_t BarRollCode[]={0x00,0x00,0xff,0x81,0x81,0xff,0x00,0x00,};
const uint8_t BarRollCode[]={0x00,0x00,0xff,0xff,0xff,0xff,0x00,0x00,};

void Bar(uint8_t Item_,uint8_t ItemNum_)//,u8 BarPosX,u8 BarNumPosX)
{
    uint8_t Y,i;
	uint8_t BarLen;
	uint8_t BarLen_Y1,Barlen_Q,Barlen_Z,Barlen_H;

	/*ÏÔÊ¾µ±Ç°Ñ¡ÖÐÊý*/
	LCD_Show_Asc_5x8(BarNumPosY,BarNumPosX+2,Item_+0x31);
	
	/*µÃµ½¹ö¶¯ÌõBarLen³¤¶È*/
	BarLen = (BarEndPosY-BarBeginPosY)/ItemNum_;


	/*ÏÔÊ¾»ù±¾ÊúÌõ*/
	for(Y=1;Y<8;Y++)
	{
		for(i=0;i<8;i++)
			LCD_Specify_Display(Y,BarBeginPosX+i,BarCode[i]);	
	}
	

	Y = ((BarEndPosY-BarBeginPosY) % BarLen);
	
	BarLen_Y1 = (Item_*BarLen)/8+1;		//´ÓµÚ¼¸Ò³¿ªÊ¼ÏÔÊ¾
	Barlen_Q = (8-((Item_*BarLen)%8))%8;		//×îÇ°ÃæÏÔÊ¾µÄÐÐÊý
	Barlen_Z = (BarLen-Barlen_Q)/8;			//ÖÐ¼äÐèÒªÏÔÊ¾µÄÒ³Êý
	Barlen_H = (BarLen-Barlen_Q)%8+Y;			//×îºó»¹ÐèÏÔÊ¾µÄÐÐÊý

//	D_printf("Item_ = %d\r\n",Item_);
//	D_printf("ItemNum_ = %d\r\n",ItemNum_);	
	
//	D_printf("BarLen_Y1 = %d\r\n",BarLen_Y1);
//	D_printf("Barlen_Q = %d\r\n",Barlen_Q);
//	D_printf("Barlen_Z = %d\r\n",Barlen_Z);
//	D_printf("Barlen_H = %d\r\n",Barlen_H);
	
	//Ç°ÏÔÊ¾ÐÐÊý
	if(Barlen_Q>0)
	{
		LCD_Specify_Display(BarLen_Y1,BarBeginPosX+2,0xff<<(8-Barlen_Q));	
		LCD_Specify_Display(BarLen_Y1,BarBeginPosX+5,0xff<<(8-Barlen_Q));
		BarLen_Y1 += 1;
	}
	//ÖÐ¼äÏÔÊ¾Ò³Êý
	while(Barlen_Z>=1)
	{
		LCD_Specify_Display(BarLen_Y1,BarBeginPosX+2,0xff);	
		LCD_Specify_Display(BarLen_Y1,BarBeginPosX+5,0xff);
		BarLen_Y1 += 1;
		Barlen_Z -= 1;
	}
	
	//ºóÏÔÊ¾ÐÐÊý
	if(Barlen_H>0)
	{
		LCD_Specify_Display(BarLen_Y1,BarBeginPosX+2,0xff>>(8-Barlen_H));	
		LCD_Specify_Display(BarLen_Y1,BarBeginPosX+5,0xff>>(8-Barlen_H));
	}
	
    Item_ += 1;
	
   	return;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//ÁÙÊ±Ìí¼Ó
/*
*****************************************************************************
* GUI_DispStringAtBar - ÏÔÊ¾Ò»¸ö×Ö·û´®ÔÚÒ»¸öBARÖÐ
* DESCRIPTION: -
* ¿ÉÒÔÉè¶¨¶ÔÆë·½Ê½
* @Param s:×Ö·û´®
* @Param x0:BARÆðÊ¼XÖá×ø±ê
* @Param x1:BAR½áÊøµãµÄXÖá×ø±ê
* @Param y0:BARµÄYÖá×ø±ê
* @Param Mode:¶ÔÆë·½Ê½ ¾ÓÖÐÓÚBAR ×ó¶ÔÆëBAR ÓÒ¶ÔÆëBAR
* @Param color: 0 Õý³£ÏÔÊ¾ 1 ·´ÏÔÊ¾
* @Return :
* ----
*****************************************************************************
*/

void GUI_DispStringAtBar1(uint8_t *s,uint8_t x0, uint8_t y0, uint8_t x1,uint8_t Mode,uint8_t color)
{
	uint8_t ENCount = 0;	//Ó¢ÎÄÍ³¼Æ
	uint8_t CNCount = 0;	//ÖÐÎÄÍ³¼Æ

	uint8_t XLen = 0;//XlenTmp = 0;
	uint8_t X = 0, Y = 0, i = 0;


	while(s[i]!='\0')	//Í³¼ÆÖÐÓ¢ÎÄ¸öÊý
	{
		if (s[i] > 0x80) 
		{
			CNCount++;
			i++;
		} 
		else 
		{
			ENCount++;
		}
		i++;
	}
	
	XLen = ENCount*8 + CNCount*12;		//¼ÆËã×Ö·û×Ü³¤¶È
	
	if(XLen > 127) XLen = 127;	//Ö»ÄÜÏÔÊ¾Ò»ÐÐ
	
	switch(Mode & 0x30)	//¶ÔÆë·½Ê½
	{
		 case GUI_TA_LEFT: //×ó¶ÔÆë
            XLen = 0;
            break;
        case GUI_TA_HCENTER:  //¾ÓÖÐ
            XLen  = ((x1-x0)-XLen)/2;
            break;
        case GUI_TA_RIGHT:   //ÓÒ¶ÔÆë
            XLen  = (x1-x0)-XLen;
            break;
        default:
            break;
	}
//--------------------------------------------------//
//×ø±ê	
	X = x0 + XLen;		//»ñµÃ¶ÔÆëÏÔÊ¾ÆðÊ¼×ø±ê
	Y = y0;
	
//--------------------------------------------------//
//ÏÔÊ¾
	
	GUI_Dispaly_String(Y,X,s,color,0);	//ÏÔÊ¾×Ö·û´®
}
#define lcd_show_8b(x,y,data) LCD_Specify_Display(y,x,data);
void lcd_rssi(int x,int y,int rssi)
{
	int i;
	const char ANT[] = {0x06,0x8A,0xFE,0x8A,0x06} ;
	for(i = 0;i < 5;i++)
	{
		lcd_show_8b(x+i,y,ANT[i]);
	}
	x = x+5;
	for (i = 0; i < 4; ++i)
	{
		/* code */
		char color;
		if(rssi) 
		{color= 0xff << (3-i)*2;}
		else
		{
			color = 0;
		}
		lcd_show_8b(x,y, color);
		lcd_show_8b(x+1,y,color);
		lcd_show_8b(x+2,y,0);
		if(rssi>0)
		{
			rssi --;
		}
		x += 3;
	}

}
void lcd_bat_level(int x,int y,int power_level)
{
	int i;
	lcd_show_8b(x,y,0xfe);x++;
	for (i = 0; i < 9; ++i)
	{
		/* code */
		char color;
		if(power_level) 
		{color= 0xfe;}
		else
		{
			color = 0x82;
		}
		lcd_show_8b(x,y, color);
		if(power_level>0)
		{
			power_level --;
		}
		x += 1;
	}
	lcd_show_8b(x,y,0x7c);x++;
	lcd_show_8b(x,y,0x38);
}
void lcd_show_charge(int x,int y)
{
	int i;
	const char logo[] = {0x08,0x4c,0x2a,0x19,0x08} ;
	for (i = 0; i < 5; ++i)
	{
		/* code */

		lcd_show_8b(x,y, logo[i]<<1);

		x += 1;
	}
}









