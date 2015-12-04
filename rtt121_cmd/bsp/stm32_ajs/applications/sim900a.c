///////////////////////////////////////////////////////
//													 //
//			   @file   SIM900A.C                     //
//													 //
//			   @date   2015.05.26					 //
//													 //
///////////////////////////////////////////////////////
#include "sim900a.h"
#include "usr_uart.h"
#include "string.h"
#include <rtthread.h>
static int sim900a_mutex = 1;
void SIM900A_Gpio_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC, ENABLE);	//GPIOCÊ±ÖÓ
	
	// PC4/5 ÍÆÍìÊä³ö
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5| GPIO_Pin_7 ;	//GPIO
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOC,GPIO_Pin_5| GPIO_Pin_7);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;	//GPIO
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOC,GPIO_Pin_4); 

}

/**
  * @brief  SIM900A ¿ª¹Ø»ú
  * @param  None 	
  * @retval None 
  */
int SIM900A_Open_Close(void)
{
	uint8_t i = 0;
	while((SIM900_STA == 0)&&i<30)	//开机等待3s
	{
		SIM900_ON = 1;
		sleep(100);
		i++;
		if(i >= 30) 
		{return -1;}	//失败返回-1;
	}
	SIM900_ON = 0;
	return 1;
}
uint8_t CSQ_Strength_Flag = 0;		//3ÐÅºÅ×îºÃ ÒÀ´Î´ÎÖ® 0 ÎÞÐÅºÅ

void sim900a_init(void)
{
	SIM900A_Gpio_Init(); 
	usr_uart_init();
	usr_uart_dma_init();
	SIM900A_Open_Close();
	sim900a_send_cmd("AT",0,50);
	sim900a_send_cmd("ATE0",0,50);
	sim900a_send_cmd("ATE0",0,50);
	if(sim900a_send_cmd("AT+CSQ","OK",50)==GSM_NO_RESPONSE)
	{
		rt_kprintf("no module\r\n");
	} 	
	sleep(500);
	sim900a_send_cmd("AT+CLTS=1",0,50);

}
char* find_str(char* str_long,char* str_short)
{
	int short_len = strlen(str_short);
	int long_len = strlen(str_long);
	int i;
	if(short_len<=long_len)
	for(i = 0; i<(long_len - short_len);i++)
	{
		if(strncmp(str_long+i,str_short,short_len)==0)
		{
			return str_long+i;
		}
	}
	return 0;

}
int sim900a_send_cmd(u8 *cmd,u8 *ack,u16 time_out_10ms)
{
	u8 res=0; 
	char* buf;
	char send_buf[256];
	int len;

	while(sim900a_mutex<=0)
	{
		sleep(10);
	}
	sim900a_mutex--;

	uart2_read(&len);
	sprintf(send_buf,"%s\r\n",cmd);

	uart2_write(send_buf,strlen(send_buf));
	rt_kprintf("GSM>>%s",send_buf);
	while(time_out_10ms)	
	{
		sleep(10);
		buf = uart2_read(&len);
		if(len>0)
		{
			buf[len] = 0;
			rt_kprintf("GSM<<%s",buf);
			break;
		} 
		time_out_10ms--;
	}

	sim900a_mutex++;

	if(time_out_10ms == 0)
	{
		return GSM_NO_RESPONSE;
	}
	if(ack)
	{
		if(find_str(buf,ack))
		{
			return GSM_OK;
		}
		else
		{
			return GSM_ERROR;
		}
	}
	
} 
int sim900a_send_cmd_fast(u8 *cmd,u8 *ack,u16 time_out_10ms)
{
	u8 res=0; 
	char* buf;
	char send_buf[256];
	int len;

	while(sim900a_mutex<=0)
	{
		sleep(10);
	}
	sim900a_mutex--;

	uart2_read(&len);
	sprintf(send_buf,"%s\r\n",cmd);
	uart2_write(send_buf,strlen(send_buf));
	while(time_out_10ms)	
	{
		sleep(10);
		buf = uart2_read(&len);
		if(len>0)
		{
			buf[len] = 0;
			break;
		} 
		time_out_10ms--;
	}

	sim900a_mutex++;

	if(time_out_10ms == 0)
	{
		return GSM_NO_RESPONSE;
	}
	if(ack)
	{
		if(find_str(buf,ack))
		{
			return GSM_OK;
		}
		else
		{
			return GSM_ERROR;
		}
	}
} 
int sim900a_send_cmd_without_mutex(u8 *cmd,u8 *ack,u16 time_out_10ms)
{
	u8 res=0; 
	char* buf;
	char send_buf[256];
	int len;


	uart2_read(&len);
	sprintf(send_buf,"%s\r\n",cmd);
	uart2_write(send_buf,strlen(send_buf));
	while(time_out_10ms)	
	{
		sleep(10);
		buf = uart2_read(&len);
		if(len>0)
		{
			buf[len] = 0;
			break;
		} 
		time_out_10ms--;
	}

	if(time_out_10ms == 0)
	{
		return GSM_NO_RESPONSE;
	}
	if(ack)
	{
		if(find_str(buf,ack))
		{
			return GSM_OK;
		}
		else
		{
			return GSM_ERROR;
		}
	}
} 
int cstr_to_unicode(char* str,char* str_out, int len)
{
	int i;
	int j = 0;
	for(i = 0; i < len;i++)
	{
		str_out[j] = '0';
		str_out[j+1] = '0';
		sprintf(str_out+j+2,"%02x",str[i]);
		j+=4;
	}
	str_out[j] = 0;
	return j;
	//sprintf(str_out+j,"0000");
}

int sim900a_send_msg(char* phone, char* text,int len)
{
	int ret = 0;
	char send_buf[256];
	while(sim900a_mutex<=0)
	{
		sleep(10);
	}
	sim900a_mutex--;
	ret |= sim900a_send_cmd_without_mutex("AT+CMGF=1","OK",50);
	ret |= sim900a_send_cmd_without_mutex("AT+CSCS=\"GSM\"","OK",50);
	sprintf(send_buf,"AT+CMGS=\"%s\"",phone);
	sim900a_send_cmd_without_mutex(send_buf,0,50);
	sprintf(send_buf,"%s",text);
	send_buf[len] = 26;
	send_buf[len+1] = 0;
	ret = sim900a_send_cmd_without_mutex(send_buf,"OK",2000);
	sim900a_mutex ++;
	return ret;
}
int sim900a_send_unicode_msg(char* phone, char* text_unic,int len)
{
	int ret = 0;
	char send_buf[256];
	char phone_unic[128];
	cstr_to_unicode(phone,phone_unic,11);
	while(sim900a_mutex<=0)
	{
		sleep(10);
	}
	sim900a_mutex--;
	ret |= sim900a_send_cmd_without_mutex("AT+CMGF=1","OK",50);
	ret |= sim900a_send_cmd_without_mutex("AT+CSMP=17,167,2,25","OK",50);
	ret |= sim900a_send_cmd_without_mutex("AT+CSCS=\"UCS2\"","OK",50);

	sprintf(send_buf,"AT+CMGS=\"%s\"",phone_unic);
	sim900a_send_cmd_without_mutex(send_buf,0,50);
	sprintf(send_buf,"%s",text_unic);
	send_buf[len] = 26;
	send_buf[len+1] = 0;
	ret = sim900a_send_cmd_without_mutex(send_buf,"OK",2000);
	sim900a_mutex ++;
	return ret;
}
int sim900a_call(char* phone, int time_out_10ms)
{
	int ret = 0;
	char send_buf[128];
	char *buf;;
	int len = 0;
	while(sim900a_mutex<=0)
	{
		sleep(10);
	}
	sim900a_mutex--;
	sim900a_send_cmd_without_mutex("AT+COLP=1","OK",50);
	sprintf(send_buf,"ATD%s;",phone);
	sim900a_send_cmd_without_mutex(send_buf,"OK",50);
	while(time_out_10ms)	
	{
		sleep(10);
		buf = uart2_read(&len);
		if(len>0)
		{
			buf[len] = 0;
			rt_kprintf("%s\r\n", buf);
			if(find_str(buf,"+COLP:"))
			{
				ret = GSM_OK;
				break;
			}
			if(find_str(buf,"NO CARRIER"))
			{
				ret = GSM_NO_RESPONSE;
				break;
			}
			if(find_str(buf,"NO ANSWER"))
			{
				ret = GSM_NO_RESPONSE;
				break;
			}
			if(find_str(buf,"ERROR"))
			{
				ret = GSM_ERROR;
				break;
			}
		} 

		time_out_10ms--;
	}
	sim900a_mutex ++;
	return ret;	
}
int sim900a_hang(void)
{
	int ret = 0;
	char send_buf[128];
	int len = 0;
	while(sim900a_mutex<=0)
	{
		sleep(10);
	}
	sim900a_mutex--;

	sprintf(send_buf,"ATH");
	ret = sim900a_send_cmd_without_mutex(send_buf,"OK",50);
	sim900a_mutex ++;
	return ret;	
}
int sim900a_get_rssi(void)
{
	char* buf;
	char *send_buf[128];
	int len,time_out_10ms = 50;

	while(sim900a_mutex<=0)
	{
		sleep(10);
	}

	uart2_read(&len);
	sprintf(send_buf,"AT+CSQ\r\n");
	uart2_write(send_buf,strlen(send_buf));
	//rt_kprintf("GSM>>%s",send_buf);
	while(time_out_10ms)	
	{
		sleep(10);
		buf = uart2_read(&len);
		if(len>0)
		{
			buf[len] = 0;
			//rt_kprintf("GSM<<%s",buf);
			break;
		} 
		time_out_10ms--;
	}

	sim900a_mutex++;

	if(time_out_10ms == 0)
	{
		return 0;
	}
	{
		int i;
		char str_rssi[5];
		int rssi_len = 0;
		for(i = 0; i<len;i++)
		{
			if(('0' <= buf[i])&&(buf[i]<='9'))
			{
				str_rssi[rssi_len] = buf[i];
				rssi_len++;
				if(rssi_len>=2)
				{
					str_rssi[rssi_len] = 0;
					i =  atoi(str_rssi);
					return i;
				}
			}
		}
	}
	return 0;
}
int sim900a_get_time(char* str)
{
	char* buf;
	char *send_buf[128];
	int len,time_out_10ms = 50;

	while(sim900a_mutex<=0)
	{
		sleep(10);
	}

	uart2_read(&len);
	sprintf(send_buf,"AT+CCLK?\r\n");
	uart2_write(send_buf,strlen(send_buf));
	//rt_kprintf("GSM>>%s",send_buf);
	while(time_out_10ms)	
	{
		sleep(10);
		buf = uart2_read(&len);
		if(len>0)
		{
			buf[len] = 0;
			//rt_kprintf("GSM<<%s",buf);
			break;
		} 
		time_out_10ms--;
	}

	sim900a_mutex++;

	if(time_out_10ms == 0)
	{
		return 0;
	}
	{
		char *t_str = find_str(buf,"CCLK: \"");
		if(t_str > 0)
		{
			memcpy (str, t_str+7,14);
			str[14] = 0;
			return GSM_OK;
		}
	}
	return 0;
}

static void at(int argc, char* argv[])
{ 
	int len;
	if(argc < 2)
	{
		rt_kprintf("/*error! eg:exec_at cmd/\r\n");
	}
	else if(argc == 3)
	{
		len = strlen(argv[1]);
		argv[1][len] = 26;
		argv[1][len+1] = 0;
	}
	sim900a_send_cmd(argv[1],0,300);
}
#include <finsh.h>
MSH_CMD_EXPORT(at, exec AT_CMD.)