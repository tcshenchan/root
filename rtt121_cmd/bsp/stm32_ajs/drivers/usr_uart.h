#ifndef __usr_uart_h
#define __usr_uart_h
#include "board.h"
#define UART2_DMA_BUF_LEN 2048
extern uint8_t uart2_dma_buf[2][UART2_DMA_BUF_LEN];


void usr_uart_init(void);
void usr_uart_dma_init(void);

void uart2_write(uint8_t *buf, int len);
uint8_t* uart2_read(uint32_t *len);
uint8_t* uart2_read_no_clean(uint32_t *len);
#endif
