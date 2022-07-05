#ifndef ___USART_DMA_H___
#define ___USART_DMA_H___


#include <stm32f4xx_usart.h>

#define USART1_RX_BUFFER_LEN	(2*1024)
#define USART1_TX_NUM_OF_BUFS	24
#define USART1_TX_TIMEOUT     	100	// in ms units 

#define USART2_RX_BUFFER_LEN	512
#define USART2_TX_NUM_OF_BUFS	4
#define USART2_TX_TIMEOUT     	100	// in ms units 

#define USART3_RX_BUFFER_LEN	512
#define USART3_TX_NUM_OF_BUFS	4
#define USART3_TX_TIMEOUT     	100	// in ms units 

#define USART_TX_BUF_SIZE     	256 

#define	USART_TX_ISR		0
#define	USART_TX_DIRECT		1


extern volatile int _usart_tx_mode;

void USART1_init();
void USART1_rx_dma_check(void);
void USART2_init();
void USART2_rx_dma_check(void);
void USART3_init();
void USART3_rx_dma_check(void);


int USART_puts(USART_TypeDef* USARTx, char *s, int len);
void USART_direct_puts(USART_TypeDef* USARTx, char *s, int len);

struct RINGBUF {
        int     cur_size;
        char    buffer[USART_TX_BUF_SIZE];
};

int _print(const char *format, ...);
int _read(USART_TypeDef* USARTx, char *buf, int len, int timeout);
int _maxread(USART_TypeDef* USARTx);
void _purge(USART_TypeDef* USARTx);
void _flush(USART_TypeDef* USARTx);


#endif
