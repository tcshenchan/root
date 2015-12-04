#include "board.h"
#include "usr_uart.h"
uint8_t uart2_dma_buf[2][UART2_DMA_BUF_LEN];
static DMA_Channel_TypeDef uart2_dma_regs;
static uint8_t uart2_buf_no = 0;

void usr_uart_init(void)
{
	  GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
	    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
    /* Enable UART clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
		
		USART_Init(USART2, &USART_InitStructure); 
		USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE);	
		USART_Cmd(USART2, ENABLE);	
	
}
//串口DMA初始化
void usr_uart_dma_init(void)
{
  DMA_InitTypeDef uart2_dma_cfg, uart3_dma_cfg, uart4_dma_cfg;
	
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 | RCC_AHBPeriph_DMA2, ENABLE);
	
	DMA_DeInit(DMA1_Channel6);
	uart2_dma_cfg.DMA_PeripheralBaseAddr = (u32)&USART2->DR;
	uart2_dma_cfg.DMA_MemoryBaseAddr = (u32)uart2_dma_buf[uart2_buf_no];
	uart2_dma_cfg.DMA_DIR = DMA_DIR_PeripheralSRC;//外设作为源地址
	uart2_dma_cfg.DMA_BufferSize = UART2_DMA_BUF_LEN;//单位为后面设置的数据宽度
	uart2_dma_cfg.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设地址不增长
	uart2_dma_cfg.DMA_MemoryInc = DMA_MemoryInc_Enable; //内存地址增长
	uart2_dma_cfg.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据宽度8bit
	uart2_dma_cfg.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte; //内存数据宽度8bit
	uart2_dma_cfg.DMA_Mode = DMA_Mode_Circular; //工作在循环模式
	uart2_dma_cfg.DMA_Priority = DMA_Priority_High; //设置该通道为高优先级
	uart2_dma_cfg.DMA_M2M = DMA_M2M_Disable; //失能内存到内存传输
	DMA_Init(DMA1_Channel6, &uart2_dma_cfg); //
	uart2_dma_regs = *DMA1_Channel6;
	DMA_Cmd(DMA1_Channel6, ENABLE);
	
}

#define DMA1_Channel3_IT_Mask    ((uint32_t)(DMA_ISR_GIF3 | DMA_ISR_TCIF3 | DMA_ISR_HTIF3 | DMA_ISR_TEIF3))
#define DMA1_Channel6_IT_Mask    ((uint32_t)(DMA_ISR_GIF6 | DMA_ISR_TCIF6 | DMA_ISR_HTIF6 | DMA_ISR_TEIF6))
#define DMA2_Channel3_IT_Mask    ((uint32_t)(DMA_ISR_GIF3 | DMA_ISR_TCIF3 | DMA_ISR_HTIF3 | DMA_ISR_TEIF3))
static void delay(void)
{
	volatile int i = 100;
	while(i--);
}
void uart2_write(uint8_t *buf, int len)
{
	int i;
	for(i = 0; i < len; i++)
	{
		USART_SendData(USART2, buf[i]);
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == 0);
	}
}
uint8_t* uart2_read(uint32_t *len)
{
	uint32_t ret = 0;
	if(USART2->SR & USART_FLAG_IDLE)
	{
			*len = UART2_DMA_BUF_LEN - DMA1_Channel6->CNDTR;
			USART2->SR = (uint16_t)~USART_FLAG_IDLE;
			if(*len == 0)
			{
				return 0;
			}
			DMA1->IFCR |= DMA1_Channel6_IT_Mask;
			DMA1_Channel6->CCR &= ~(DMA_CCR1_EN);
			*DMA1_Channel6 = uart2_dma_regs;
			ret = (u32)uart2_dma_buf[uart2_buf_no];
			uart2_buf_no++;
			uart2_buf_no %= 2;
			DMA1_Channel6->CMAR = (u32)uart2_dma_buf[uart2_buf_no];
			DMA1_Channel6->CCR |= DMA_CCR1_EN;
			return (uint8_t*)ret;
			
	}
	return 0;
}
uint8_t* uart2_read_no_clean(uint32_t *len)
{
	uint32_t ret = 0;
	if(USART2->SR & USART_FLAG_IDLE)
	{
			*len = UART2_DMA_BUF_LEN - DMA1_Channel6->CNDTR;
			if(*len == 0)
			{
				return 0;
			}
			ret = (u32)uart2_dma_buf[uart2_buf_no];
			return (uint8_t*)ret;		
	}
	return 0;
}
