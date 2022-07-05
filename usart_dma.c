#include "usart_dma.h"
#include "hardware.h"

#include <string.h>
#include <stm32f4xx_dma.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "msg.h"
#include "utils.h"
#include "config.h"

volatile unsigned char usart1_receive_string[USART1_RX_BUFFER_LEN];
volatile unsigned char usart1_rx_dma_buffer[USART1_RX_BUFFER_LEN];
volatile unsigned int usart1_rx_dma_counter  = 0;
volatile unsigned int usart1_receive_string_len  = 0;
volatile struct RINGBUF usart1_tx_ringbuf[USART1_TX_NUM_OF_BUFS];
volatile int usart1_tx_free_buffer_idx = 0;
volatile int usart1_tx_transmitting_buffer_idx = 0;

volatile unsigned char usart2_receive_string[USART2_RX_BUFFER_LEN];
volatile unsigned char usart2_rx_dma_buffer[USART2_RX_BUFFER_LEN];
volatile unsigned int usart2_rx_dma_counter  = 0;
volatile unsigned int usart2_receive_string_len  = 0;
volatile struct RINGBUF usart2_tx_ringbuf[USART2_TX_NUM_OF_BUFS];
volatile int usart2_tx_free_buffer_idx = 0;
volatile int usart2_tx_transmitting_buffer_idx = 0;

volatile unsigned char usart3_receive_string[USART3_RX_BUFFER_LEN];
volatile unsigned char usart3_rx_dma_buffer[USART3_RX_BUFFER_LEN];
volatile unsigned int usart3_rx_dma_counter  = 0;
volatile unsigned int usart3_receive_string_len  = 0;
volatile struct RINGBUF usart3_tx_ringbuf[USART3_TX_NUM_OF_BUFS];
volatile int usart3_tx_free_buffer_idx = 0;
volatile int usart3_tx_transmitting_buffer_idx = 0;

volatile int _usart_tx_mode = USART_TX_ISR;

extern int DMA1_Stream5_used_by_DAC;
extern int play_buf_empty;
extern int play_dma_ht_counter;
extern int play_dma_tc_counter;
extern int play_buf_len;
extern char* play_buf;

extern uint8_t event_logging;

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))


static void USART1_dma_request(void)
{

        // Find next loaded buffer

        int loaded_buffer_idx, i;

        for(i = 0; i < USART1_TX_NUM_OF_BUFS; i++) {
                loaded_buffer_idx = (usart1_tx_transmitting_buffer_idx + i) % USART1_TX_NUM_OF_BUFS;
                if(usart1_tx_ringbuf[loaded_buffer_idx].cur_size != 0)
                        break;
        }
        if(i == USART1_TX_NUM_OF_BUFS) // No loaded buffers found
                return;

        usart1_tx_transmitting_buffer_idx = loaded_buffer_idx;

        DMA_MemoryTargetConfig(DMA2_Stream7, (uint32_t)usart1_tx_ringbuf[loaded_buffer_idx].buffer , DMA_Memory_0);
        DMA_SetCurrDataCounter(DMA2_Stream7, usart1_tx_ringbuf[loaded_buffer_idx].cur_size);

        DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF7 | DMA_FLAG_FEIF7 | DMA_FLAG_DMEIF7 | DMA_FLAG_TEIF7 | DMA_FLAG_HTIF7);

        DMA_Cmd(DMA2_Stream7, ENABLE);

}

#ifdef USART_EXT1
static void USART2_dma_request(void)
{

        // Find next loaded buffer

        int loaded_buffer_idx, i;

        for(i = 0; i < USART2_TX_NUM_OF_BUFS; i++) {
                loaded_buffer_idx = (usart2_tx_transmitting_buffer_idx + i) % USART2_TX_NUM_OF_BUFS;
                if(usart2_tx_ringbuf[loaded_buffer_idx].cur_size != 0)
                        break;
        }
        if(i == USART2_TX_NUM_OF_BUFS) // No loaded buffers found
                return;

        usart2_tx_transmitting_buffer_idx = loaded_buffer_idx;

        DMA_MemoryTargetConfig(DMA1_Stream6, (uint32_t)usart2_tx_ringbuf[loaded_buffer_idx].buffer , DMA_Memory_0);
        DMA_SetCurrDataCounter(DMA1_Stream6, usart2_tx_ringbuf[loaded_buffer_idx].cur_size);

	gpio_set(USART_EXT1_RS485_TX_ENL_GPIO_PORT, USART_EXT1_RS485_TX_ENL_GPIO_PIN, 1); // USART_EXT1_RS485_TX_ENL = 1

        DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6 | DMA_FLAG_FEIF6 | DMA_FLAG_DMEIF6 | DMA_FLAG_TEIF6 | DMA_FLAG_HTIF6);

        DMA_Cmd(DMA1_Stream6, ENABLE);

}
#endif

#ifdef USART_EXT2
static void USART3_dma_request(void)
{

        // Find next loaded buffer

        int loaded_buffer_idx, i;

        for(i = 0; i < USART3_TX_NUM_OF_BUFS; i++) {
                loaded_buffer_idx = (usart3_tx_transmitting_buffer_idx + i) % USART3_TX_NUM_OF_BUFS;
                if(usart3_tx_ringbuf[loaded_buffer_idx].cur_size != 0)
                        break;
        }
        if(i == USART3_TX_NUM_OF_BUFS) // No loaded buffers found
                return;

        usart3_tx_transmitting_buffer_idx = loaded_buffer_idx;

        DMA_MemoryTargetConfig(DMA1_Stream3, (uint32_t)usart3_tx_ringbuf[loaded_buffer_idx].buffer , DMA_Memory_0);
        DMA_SetCurrDataCounter(DMA1_Stream3, usart3_tx_ringbuf[loaded_buffer_idx].cur_size);

	gpio_set(USART_EXT2_RS485_TX_ENL_GPIO_PORT, USART_EXT2_RS485_TX_ENL_GPIO_PIN, 1); // USART_EXT2_RS485_TX_ENL = 1

        DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF3 | DMA_FLAG_FEIF3 | DMA_FLAG_DMEIF3 | DMA_FLAG_TEIF3 | DMA_FLAG_HTIF3);

        DMA_Cmd(DMA1_Stream3, ENABLE);

	//_print("USART3_dma_request()\r\n\r\n");

}
#endif

void DMA2_Stream7_IRQHandler(void) {

        __disable_irq();

        if(DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7)) {

                usart1_tx_ringbuf[usart1_tx_transmitting_buffer_idx].cur_size = 0; // mark this buffer as free

                DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);

                USART1_dma_request();
        }

        __enable_irq();
}

#ifdef USART_EXT1
void DMA1_Stream6_IRQHandler(void) {

        __disable_irq();

	//print("DMA1_Stream6_IRQHandler\r\n");


        if(DMA_GetITStatus(DMA1_Stream6, DMA_IT_TCIF6)) {

                usart2_tx_ringbuf[usart2_tx_transmitting_buffer_idx].cur_size = 0; // mark this buffer as free

                while( !(USART_EXT1->SR & 0x00000040) ); // wait for lst byte to transmit

		//print("USART_EXT1_RS485_TX_ENL = 0\r\n");
		gpio_set(USART_EXT1_RS485_TX_ENL_GPIO_PORT, USART_EXT1_RS485_TX_ENL_GPIO_PIN, 0); // RS485_TX_ENL = 0

                DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6);

                USART2_dma_request();
        }

        __enable_irq();
}
#endif

#ifdef USART_EXT2
void DMA1_Stream3_IRQHandler(void) {

        __disable_irq();

	//print("DMA1_Stream3_IRQHandler\r\n");


        if(DMA_GetITStatus(DMA1_Stream3, DMA_IT_TCIF3)) {

                usart3_tx_ringbuf[usart3_tx_transmitting_buffer_idx].cur_size = 0; // mark this buffer as free

                while( !(USART_EXT2->SR & 0x00000040) ); // wait for lst byte to transmit

		//print("USART_EXT2_RS485_TX_ENL = 0\r\n");
		gpio_set(USART_EXT2_RS485_TX_ENL_GPIO_PORT, USART_EXT2_RS485_TX_ENL_GPIO_PIN, 0); // RS485_TX_ENL = 0

                DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3);

                USART3_dma_request();
        }

        __enable_irq();
}
#endif


void DMA2_Stream2_IRQHandler(void) {
        if(DMA_GetITStatus(DMA2_Stream2, DMA_IT_TCIF2)) {
//                _print("DMA2_Stream2_IRQHandler: USART1 RX DMA buffer overlap\r\r\n");
                DMA_ClearITPendingBit(DMA2_Stream2, DMA_IT_TCIF2);
        }
}

void DMA1_Stream5_IRQHandler(void) {

        __disable_irq();

	//print("DMA1_Stream5_IRQHandler\r\n");

	if(DMA1_Stream5_used_by_DAC) {

		int ht = DMA_GetITStatus(DMA1_Stream5, DMA_IT_HTIF5);
		int tc = DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5);

		if(event_logging)
			_print("DMA1_Stream5_IRQHandler ht=%d, tc=%d, play_buf = %p, play_buf_len = %d\r\n", ht, tc, play_buf, play_buf_len);

		if(ht) {
               		DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_HTIF5);
			play_dma_ht_counter++;
			if(play_buf_empty) 
				PostMessageIRQ(AUDIO_PLAY_STOP, 1, 0, 0);
			else
                		PostMessageIRQ(AUDIO_PLAY_DMA_HT, 1, 0, 0);
		}

		if(tc) {
                	DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
			play_dma_tc_counter++;
			if(play_buf_empty) 
				PostMessageIRQ(AUDIO_PLAY_STOP, 1, 0, 0);
			else
				PostMessageIRQ(AUDIO_PLAY_DMA_TC, 1, 0, 0);
		}

	} else { 

	        if(DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5)) {
      	          	//_print("DMA1_Stream5_IRQHandler: USART2 RX DMA buffer overlap\r\r\n");

                	DMA_ClearITPendingBit(DMA1_Stream5, DMA_IT_TCIF5);
        	}

		if(event_logging)
                        _print("DMA1_Stream5_IRQHandler USART2 mode\r\n");
	}

        __enable_irq();
}


void USART1_init ()
{

	usart1_rx_dma_counter  = 0;
	usart1_receive_string_len  = 0;
        usart1_tx_free_buffer_idx = 0;
        usart1_tx_transmitting_buffer_idx = 0;


        GPIO_InitTypeDef GPIO_InitStructure; // this is for the GPIO pins used as TX and RX
        USART_InitTypeDef USART_InitStructure; // this is for the USART1 initilization
        NVIC_InitTypeDef   NVIC_InitStructure;

        //enable APB2 peripheral clock for USART1
        RCC_APB2PeriphClockCmd(USART_DEBUG_USART_PERIPH, ENABLE);

        /* enable the peripheral clock for the pins used by * USART1, PB6 for TX and PB7 for RX */
        RCC_AHB1PeriphClockCmd(USART_DEBUG_GPIO_PERIPH, ENABLE);

        /* This sequence sets up the TX and RX pins * so they work correctly with the USART1 peripheral */
        GPIO_InitStructure.GPIO_Pin = USART_DEBUG_GPIO_TX_PIN | USART_DEBUG_GPIO_RX_PIN; //
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // the pins are configured as alternate function so the USART peripheral has access to them
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       // this defines the IO speed and has nothing to do with the baudrate!
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  // this defines the output type as push pull mode (as opposed to open drain)
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;    // this activates the pullup resistors on the IO pins
        GPIO_Init(USART_DEBUG_GPIO_PORT, &GPIO_InitStructure);          // now all the values are passed to the GPIO_Init() function which sets the GPIO registers

        /* The RX and TX pins are now connected to their AF * so that the USART1 can take over control of the * pins */
        GPIO_PinAFConfig(USART_DEBUG_GPIO_PORT, USART_DEBUG_GPIO_TX_SRC_PIN, USART_DEBUG_GPIO_AF); //
        GPIO_PinAFConfig(USART_DEBUG_GPIO_PORT, USART_DEBUG_GPIO_RX_SRC_PIN, USART_DEBUG_GPIO_AF);

        /* Now the USART_InitStructure is used to define the * properties of USART1 */
        USART_InitStructure.USART_BaudRate = USART_DEBUG_BAUD;                            // the baudrate is set to the value we passed into this init function
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;// we want the data frame size to be 8 bits (standard)
        USART_InitStructure.USART_StopBits = USART_StopBits_1;          // we want 1 stop bit (standard)
        USART_InitStructure.USART_Parity = USART_Parity_No;             // we don't want a parity bit (standard)
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // we don't want flow control (standard)
        USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // we want to enable the transmitter and the receiver
        USART_Init(USART_DEBUG, &USART_InitStructure);// again all the properties are passed to the USART_Init function which takes care of all the bit setting

        // finally this enables the complete USART1 peripheral
        USART_Cmd(USART_DEBUG, ENABLE);

        //////////////// DMA ////////////////////////////

        // Init DMA
        DMA_InitTypeDef  DMA_InitStructure;

        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
        DMA_ClearFlag(DMA2_Stream7, DMA_FLAG_TCIF7 | DMA_FLAG_FEIF7 | DMA_FLAG_DMEIF7 | DMA_FLAG_TEIF7 | DMA_FLAG_HTIF7);
        DMA_Cmd(DMA2_Stream7, DISABLE);
        DMA_DeInit(DMA2_Stream7);
        DMA_InitStructure.DMA_Channel = DMA_Channel_4;
        DMA_InitStructure.DMA_PeripheralBaseAddr = (unsigned int)&(USART1->DR);
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
        DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
        DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
        DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
        DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
        DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
        DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
        DMA_Init(DMA2_Stream7, &(DMA_InitStructure));

        USART_DMACmd(USART_DEBUG, USART_DMAReq_Tx, ENABLE);


        // Configure and enable DMA IRQ
        DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);
        NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        // USART1 RX DMA (1 bytes circular mode)
        //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
        DMA_ClearFlag(DMA2_Stream2, DMA_FLAG_TCIF2 | DMA_FLAG_FEIF2 | DMA_FLAG_DMEIF2 | DMA_FLAG_TEIF2 | DMA_FLAG_HTIF2);
        DMA_Cmd(DMA2_Stream2, DISABLE);
        DMA_DeInit(DMA2_Stream2);
        DMA_InitStructure.DMA_Channel = DMA_Channel_4;
        DMA_InitStructure.DMA_PeripheralBaseAddr = (unsigned int)&(USART1->DR);
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&usart1_rx_dma_buffer;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
        DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
        DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; //DMA_FIFOMode_Enable;
        DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
        DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
        DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
        DMA_InitStructure.DMA_BufferSize = USART1_RX_BUFFER_LEN;
        DMA_Init(DMA2_Stream2, &(DMA_InitStructure));

        USART_DMACmd(USART_DEBUG, USART_DMAReq_Rx, ENABLE);

        DMA_ClearFlag(DMA2_Stream2, DMA_FLAG_TCIF2 | DMA_FLAG_FEIF2 | DMA_FLAG_DMEIF2 | DMA_FLAG_TEIF2 | DMA_FLAG_HTIF2);
        DMA_Cmd(DMA2_Stream2, ENABLE);

	//print("USART1_init() usart1_receive_string_len = %d, bytes in DMA: %d\r\n", usart1_receive_string_len, USART1_RX_BUFFER_LEN - DMA_GetCurrDataCounter(DMA2_Stream2));
}

#ifdef USART_EXT1
void USART2_init ()
{

	print("USART2_init\r\n");

	usart2_rx_dma_counter  = 0;
	usart2_receive_string_len  = 0;
	usart2_tx_free_buffer_idx = 0;
	usart2_tx_transmitting_buffer_idx = 0;

	gpio_init(USART_EXT1_RS485_TX_ENL_PERIPH, GPIO_SPEED, USART_EXT1_RS485_TX_ENL_GPIO_PORT, USART_EXT1_RS485_TX_ENL_GPIO_PIN, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_NOPULL); // EXT1 RS485_TX_ENL


	USART_ClearFlag(USART2, USART_FLAG_CTS | USART_FLAG_LBD | USART_FLAG_TXE | USART_FLAG_TC | USART_FLAG_RXNE | USART_FLAG_IDLE | USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE);

        GPIO_InitTypeDef GPIO_InitStructure; // this is for the GPIO pins used as TX and RX
        USART_InitTypeDef USART_InitStructure; // this is for the USART1 initilization
        NVIC_InitTypeDef   NVIC_InitStructure;

        //enable APB1 peripheral clock for USART2
        RCC_APB1PeriphClockCmd(USART_EXT1_USART_PERIPH, ENABLE);

        /* enable the peripheral clock for the pins used by * USART1, PB6 for TX and PB7 for RX */
        RCC_AHB1PeriphClockCmd(USART_EXT1_GPIO_PERIPH, ENABLE);

        /* This sequence sets up the TX and RX pins * so they work correctly with the USART1 peripheral */
        GPIO_InitStructure.GPIO_Pin = USART_EXT1_GPIO_TX_PIN | USART_EXT1_GPIO_RX_PIN; //
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // the pins are configured as alternate function so the USART peripheral has access to them
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       // this defines the IO speed and has nothing to do with the baudrate!
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  // this defines the output type as push pull mode (as opposed to open drain)
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;    // this activates the pullup resistors on the IO pins
        GPIO_Init(USART_EXT1_GPIO_PORT, &GPIO_InitStructure);          // now all the values are passed to the GPIO_Init() function which sets the GPIO registers

        /* The RX and TX pins are now connected to their AF * so that the USART1 can take over control of the * pins */
        GPIO_PinAFConfig(USART_EXT1_GPIO_PORT, USART_EXT1_GPIO_TX_SRC_PIN, USART_EXT1_GPIO_AF); //
        GPIO_PinAFConfig(USART_EXT1_GPIO_PORT, USART_EXT1_GPIO_RX_SRC_PIN, USART_EXT1_GPIO_AF);

//        USART_InitStructure.USART_BaudRate = 9600;                            // the baudrate is set to the value we passed into this init function
//        USART_InitStructure.USART_WordLength = USART_WordLength_8b;// we want the data frame size to be 8 bits (standard)
//        USART_InitStructure.USART_StopBits = USART_StopBits_1;          // we want 1 stop bit (standard)
//        USART_InitStructure.USART_Parity = USART_Parity_No;             // we don't want a parity bit (standard)

        /* Now the USART_InitStructure is used to define the * properties of USART1 */
        USART_InitStructure.USART_BaudRate = baud_rates[config_active.ext1_port_baud_rate];
        USART_InitStructure.USART_StopBits = (config_active.ext1_port_stop_bits << 12);
        USART_InitStructure.USART_Parity = (config_active.ext1_port_parity << 10); 
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // we don't want flow control (standard)
        USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // we want to enable the transmitter and the receiver
        USART_Init(USART_EXT1, &USART_InitStructure);// again all the properties are passed to the USART_Init function which takes care of all the bit setting

        // finally this enables the complete USART2 peripheral
        USART_Cmd(USART_EXT1, ENABLE);

        //////////////// DMA ////////////////////////////

        // Init DMA
        DMA_InitTypeDef  DMA_InitStructure;

        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
        DMA_ClearFlag(DMA1_Stream6, DMA_FLAG_TCIF6 | DMA_FLAG_FEIF6 | DMA_FLAG_DMEIF6 | DMA_FLAG_TEIF6 | DMA_FLAG_HTIF6);
        DMA_Cmd(DMA1_Stream6, DISABLE);
        DMA_DeInit(DMA1_Stream6);
        DMA_InitStructure.DMA_Channel = DMA_Channel_4;
        DMA_InitStructure.DMA_PeripheralBaseAddr = (unsigned int)&(USART_EXT1->DR);
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
        DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
        DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
        DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
        DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
        DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
        DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
        DMA_Init(DMA1_Stream6, &(DMA_InitStructure));

        USART_DMACmd(USART_EXT1, USART_DMAReq_Tx, ENABLE);


        // Configure and enable DMA IRQ
        DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE);
        NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream6_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        // USART2 RX DMA (1 bytes circular mode)
        //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	USART_DMACmd(USART_EXT1, USART_DMAReq_Rx, DISABLE);
        DMA_Cmd(DMA1_Stream5, DISABLE);
        DMA_DeInit(DMA1_Stream5);
        DMA_InitStructure.DMA_Channel = DMA_Channel_4;
        DMA_InitStructure.DMA_PeripheralBaseAddr = (unsigned int)&(USART_EXT1->DR);
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&usart2_rx_dma_buffer;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
        DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
        DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; //DMA_FIFOMode_Enable;
        DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
        DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
        DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
        DMA_InitStructure.DMA_BufferSize = USART2_RX_BUFFER_LEN;
        DMA_Init(DMA1_Stream5, &(DMA_InitStructure));

        USART_DMACmd(USART_EXT1, USART_DMAReq_Rx, ENABLE);
        DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_TCIF5 | DMA_FLAG_FEIF5 | DMA_FLAG_DMEIF5 | DMA_FLAG_TEIF5 | DMA_FLAG_HTIF5);
        DMA_Cmd(DMA1_Stream5, ENABLE);

	//print("USART2_init() usart2_receive_string_len = %d, bytes in DMA: %d\r\n", usart2_receive_string_len, USART2_RX_BUFFER_LEN - DMA_GetCurrDataCounter(DMA1_Stream5));
}
#endif

#ifdef USART_EXT2
void USART3_init ()
{

	print("USART3_init\r\n");

	usart3_rx_dma_counter  = 0;
	usart3_receive_string_len  = 0;
	usart3_tx_free_buffer_idx = 0;
	usart3_tx_transmitting_buffer_idx = 0;

	gpio_init(USART_EXT2_RS485_TX_ENL_PERIPH, GPIO_SPEED, USART_EXT2_RS485_TX_ENL_GPIO_PORT, USART_EXT2_RS485_TX_ENL_GPIO_PIN, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_NOPULL); // EXT2 RS485_TX_ENL


	USART_ClearFlag(USART3, USART_FLAG_CTS | USART_FLAG_LBD | USART_FLAG_TXE | USART_FLAG_TC | USART_FLAG_RXNE | USART_FLAG_IDLE | USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE);

        GPIO_InitTypeDef GPIO_InitStructure; // this is for the GPIO pins used as TX and RX
        USART_InitTypeDef USART_InitStructure; // this is for the USART1 initilization
        NVIC_InitTypeDef   NVIC_InitStructure;

        //enable APB1 peripheral clock for USART2
        RCC_APB1PeriphClockCmd(USART_EXT2_USART_PERIPH, ENABLE);

        /* enable the peripheral clock for the pins used by * USART1, PB6 for TX and PB7 for RX */
        RCC_AHB1PeriphClockCmd(USART_EXT2_GPIO_PERIPH, ENABLE);

        /* This sequence sets up the TX and RX pins * so they work correctly with the USART1 peripheral */
        GPIO_InitStructure.GPIO_Pin = USART_EXT2_GPIO_TX_PIN | USART_EXT2_GPIO_RX_PIN; //
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // the pins are configured as alternate function so the USART peripheral has access to them
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       // this defines the IO speed and has nothing to do with the baudrate!
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  // this defines the output type as push pull mode (as opposed to open drain)
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;    // this activates the pullup resistors on the IO pins
        GPIO_Init(USART_EXT2_GPIO_PORT, &GPIO_InitStructure);          // now all the values are passed to the GPIO_Init() function which sets the GPIO registers

        /* The RX and TX pins are now connected to their AF * so that the USART1 can take over control of the * pins */
        GPIO_PinAFConfig(USART_EXT2_GPIO_PORT, USART_EXT2_GPIO_TX_SRC_PIN, USART_EXT2_GPIO_AF); //
        GPIO_PinAFConfig(USART_EXT2_GPIO_PORT, USART_EXT2_GPIO_RX_SRC_PIN, USART_EXT2_GPIO_AF);

//        USART_InitStructure.USART_BaudRate = 9600;                            // the baudrate is set to the value we passed into this init function
//        USART_InitStructure.USART_WordLength = USART_WordLength_8b;// we want the data frame size to be 8 bits (standard)
//        USART_InitStructure.USART_StopBits = USART_StopBits_1;          // we want 1 stop bit (standard)
//        USART_InitStructure.USART_Parity = USART_Parity_No;             // we don't want a parity bit (standard)

        /* Now the USART_InitStructure is used to define the * properties of USART1 */
        USART_InitStructure.USART_BaudRate = baud_rates[config_active.ext2_port_baud_rate];
        USART_InitStructure.USART_StopBits = (config_active.ext2_port_stop_bits << 12);
        USART_InitStructure.USART_Parity = (config_active.ext2_port_parity << 10); 
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // we don't want flow control (standard)
        USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // we want to enable the transmitter and the receiver
        USART_Init(USART_EXT2, &USART_InitStructure);// again all the properties are passed to the USART_Init function which takes care of all the bit setting

        // finally this enables the complete USART2 peripheral
        USART_Cmd(USART_EXT2, ENABLE);

        //////////////// DMA ////////////////////////////

        // Init DMA
        DMA_InitTypeDef  DMA_InitStructure;

        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
        DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF3 | DMA_FLAG_FEIF3 | DMA_FLAG_DMEIF3 | DMA_FLAG_TEIF3 | DMA_FLAG_HTIF3);
        DMA_Cmd(DMA1_Stream3, DISABLE);
        DMA_DeInit(DMA1_Stream3);
        DMA_InitStructure.DMA_Channel = DMA_Channel_4;
        DMA_InitStructure.DMA_PeripheralBaseAddr = (unsigned int)&(USART_EXT2->DR);
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
        DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
        DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
        DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
        DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
        DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
        DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
        DMA_Init(DMA1_Stream3, &(DMA_InitStructure));

        USART_DMACmd(USART_EXT2, USART_DMAReq_Tx, ENABLE);


        // Configure and enable DMA IRQ
        DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);
        NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream3_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        // USART2 RX DMA (1 bytes circular mode)
        //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	USART_DMACmd(USART_EXT2, USART_DMAReq_Rx, DISABLE);
        DMA_Cmd(DMA1_Stream1, DISABLE);
        DMA_DeInit(DMA1_Stream1);
        DMA_InitStructure.DMA_Channel = DMA_Channel_4;
        DMA_InitStructure.DMA_PeripheralBaseAddr = (unsigned int)&(USART_EXT2->DR);
        DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
        DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
        DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
        DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&usart3_rx_dma_buffer;
        DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
        DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
        DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; //DMA_FIFOMode_Enable;
        DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
        DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
        DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
        DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
        DMA_InitStructure.DMA_BufferSize = USART3_RX_BUFFER_LEN;
        DMA_Init(DMA1_Stream1, &(DMA_InitStructure));

        USART_DMACmd(USART_EXT2, USART_DMAReq_Rx, ENABLE);
        DMA_ClearFlag(DMA1_Stream1, DMA_FLAG_TCIF1 | DMA_FLAG_FEIF1 | DMA_FLAG_DMEIF1 | DMA_FLAG_TEIF1 | DMA_FLAG_HTIF1);
        DMA_Cmd(DMA1_Stream1, ENABLE);

	//print("USART3_init() usart3_receive_string_len = %d, bytes in DMA: %d\r\n", usart3_receive_string_len, USART3_RX_BUFFER_LEN - DMA_GetCurrDataCounter(DMA1_Stream1));
}
#endif

void USART_direct_puts(USART_TypeDef* USARTx, char *s, int len){

	#ifdef USART_EXT1
	if(USARTx == USART_EXT1) 
		gpio_set(USART_EXT1_RS485_TX_ENL_GPIO_PORT, USART_EXT1_RS485_TX_ENL_GPIO_PIN, 1); // RS485_TX_ENL = 1
	#endif

	#ifdef USART_EXT2
	if(USARTx == USART_EXT2) 
		gpio_set(USART_EXT2_RS485_TX_ENL_GPIO_PORT, USART_EXT2_RS485_TX_ENL_GPIO_PIN, 1); // RS485_TX_ENL = 1
	#endif

	for(int i = 0; i < len; i++) {
                // wait until data register is empty
                while( !(USARTx->SR & 0x00000040) );
                USART_SendData(USARTx, *s);
                *s++;
        }

	#ifdef USART_EXT1
	if(USARTx == USART_EXT1) { 
                while( !(USARTx->SR & 0x00000040) );
		gpio_set(USART_EXT1_RS485_TX_ENL_GPIO_PORT, USART_EXT1_RS485_TX_ENL_GPIO_PIN, 0); // RS485_TX_ENL = 0
	}
	#endif

	#ifdef USART_EXT2
	if(USARTx == USART_EXT2) { 
                while( !(USARTx->SR & 0x00000040) );
		gpio_set(USART_EXT2_RS485_TX_ENL_GPIO_PORT, USART_EXT2_RS485_TX_ENL_GPIO_PIN, 0); // RS485_TX_ENL = 0
	}
	#endif
}


int USART_puts(USART_TypeDef* USARTx, char *s, int len){

	int attempts;
	int free_buffer_idx, i;

        if(USARTx == USART1) {

		for(attempts = 0; attempts < USART1_TX_TIMEOUT; attempts++) {

	                __disable_irq();

      		         // Find next free buffer

	                for(i = 0; i < USART1_TX_NUM_OF_BUFS; i++) {
            	            free_buffer_idx = (usart1_tx_transmitting_buffer_idx + i) % USART1_TX_NUM_OF_BUFS;
                	        if(usart1_tx_ringbuf[free_buffer_idx].cur_size == 0)
                        	        break;
              	  	}

                	if(i == USART1_TX_NUM_OF_BUFS) { // No free buffers found
                        	__enable_irq();
				DelayLoop(1);
				//Delay(10); // this hangs because print() is used in Exception_Handler where IRQs are blocked
				continue;
			}

			break;
                }

		if(attempts == USART1_TX_TIMEOUT) {
                       	__enable_irq();
			return -1;
		} 

                len = MIN(len,USART_TX_BUF_SIZE);
                memcpy((void*)usart1_tx_ringbuf[free_buffer_idx].buffer, s, len);
                usart1_tx_ringbuf[free_buffer_idx].cur_size = len;

                __enable_irq();

                // If DMA not busy, engage it

                if(DMA_GetCmdStatus(DMA2_Stream7) != ENABLE) {
                        USART1_dma_request();
		}


        } 

	#ifdef USART_EXT1
	else if(USARTx == USART2) {

		for(attempts = 0; attempts < USART2_TX_TIMEOUT; attempts++) {

	                __disable_irq();

      		         // Find next free buffer

	                for(i = 0; i < USART2_TX_NUM_OF_BUFS; i++) {
            	            free_buffer_idx = (usart2_tx_transmitting_buffer_idx + i) % USART2_TX_NUM_OF_BUFS;
                	        if(usart2_tx_ringbuf[free_buffer_idx].cur_size == 0)
                        	        break;
              	  	}

                	if(i == USART2_TX_NUM_OF_BUFS) { // No free buffers found
                        	__enable_irq();
				DelayLoop(1);
				//Delay(10); // this hangs because print() is used in Exception_Handler where IRQs are blocked
				continue;
			}

			break;
                }

		if(attempts == USART2_TX_TIMEOUT) {
                       	__enable_irq();
			return -1;
		} 

                len = MIN(len,USART_TX_BUF_SIZE);
                memcpy((void*)usart2_tx_ringbuf[free_buffer_idx].buffer, s, len);
                usart2_tx_ringbuf[free_buffer_idx].cur_size = len;

                __enable_irq();

                // If DMA not busy, engage it

                if(DMA_GetCmdStatus(DMA1_Stream6) != ENABLE) {
                        USART2_dma_request();
		}


        }
	#endif


	#ifdef USART_EXT2
	else if(USARTx == USART3) {

		for(attempts = 0; attempts < USART3_TX_TIMEOUT; attempts++) {

	                __disable_irq();

      		         // Find next free buffer

	                for(i = 0; i < USART3_TX_NUM_OF_BUFS; i++) {
            	            free_buffer_idx = (usart3_tx_transmitting_buffer_idx + i) % USART3_TX_NUM_OF_BUFS;
                	        if(usart3_tx_ringbuf[free_buffer_idx].cur_size == 0)
                        	        break;
              	  	}

                	if(i == USART3_TX_NUM_OF_BUFS) { // No free buffers found
                        	__enable_irq();
				DelayLoop(1);
				//Delay(10); // this hangs because print() is used in Exception_Handler where IRQs are blocked
				continue;
			}

			break;
                }

		if(attempts == USART3_TX_TIMEOUT) {
                       	__enable_irq();
			return -1;
		} 

                len = MIN(len,USART_TX_BUF_SIZE);
                memcpy((void*)usart3_tx_ringbuf[free_buffer_idx].buffer, s, len);
                usart3_tx_ringbuf[free_buffer_idx].cur_size = len;

                __enable_irq();

                // If DMA not busy, engage it

                if(DMA_GetCmdStatus(DMA1_Stream3) != ENABLE) {
                        USART3_dma_request();
		}
        }
	#endif

	return len;
}


void USART1_rx_dma_check(void)
{

        int i;
        int bytes_in_dma_buffer = USART1_RX_BUFFER_LEN - DMA_GetCurrDataCounter(DMA2_Stream2); // How much data currently in DMA buffer

        if(bytes_in_dma_buffer > usart1_rx_dma_counter) {
                int bytes_newly_read = bytes_in_dma_buffer - usart1_rx_dma_counter;
                for(i = 0; i < bytes_newly_read; i++ ) {
                        usart1_receive_string[usart1_receive_string_len++] = usart1_rx_dma_buffer[usart1_rx_dma_counter + i];
                        if(usart1_receive_string_len >= USART1_RX_BUFFER_LEN) {
                                _print("USART1_rx_dma_check()::1 usart1_receive_string overflow, too long command, bidb = %d, urdc = %d, urs_len = %d\r\n", bytes_in_dma_buffer, usart2_rx_dma_counter, usart2_receive_string_len);
                                usart1_receive_string_len = 0;
				continue;
                        }
                        //if(usart1_receive_string[usart1_receive_string_len-1] == '\n' || usart1_receive_string[usart1_receive_string_len-1] == '\r') {
                        if(usart1_receive_string_len) {
                                // Process command
				PostMessageIRQ(USART1_RX_DATA, 1, (int)usart1_receive_string, usart1_receive_string_len);
                                //usart1_receive_string_len = 0;
                        }
                }
                usart1_rx_dma_counter += bytes_newly_read;

        } else if(bytes_in_dma_buffer < usart1_rx_dma_counter) {
                // Overlap occured
                int bytes_newly_read = USART1_RX_BUFFER_LEN - usart1_rx_dma_counter;

                // First part
                for(i = 0; i < bytes_newly_read; i++ ) {
                        usart1_receive_string[usart1_receive_string_len++] = usart1_rx_dma_buffer[usart1_rx_dma_counter + i];
                        if(usart1_receive_string_len >= USART1_RX_BUFFER_LEN) {
                                _print("USART1_rx_dma_check()::2 usart1_receive_string overflow, too long command, bidb = %d, urdc = %d, urs_len = %d\r\n", bytes_in_dma_buffer, usart2_rx_dma_counter, usart2_receive_string_len);
                                usart1_receive_string_len = 0;
				continue;
                        }
                        if(usart1_receive_string_len >= USART1_RX_BUFFER_LEN) {
                                usart1_receive_string_len = 0;
				continue;
                        }
                        //if(usart1_receive_string[usart1_receive_string_len-1] == '\n' || usart1_receive_string[usart1_receive_string_len-1] == '\r') {
                        if(usart1_receive_string_len) {
                                // Process command
				PostMessageIRQ(USART1_RX_DATA, 1, (int)usart1_receive_string, usart1_receive_string_len);
                                //usart1_receive_string_len = 0;
                        }
                }

                // Second part
                for(i = 0; i < bytes_in_dma_buffer; i++ ) {
                        usart1_receive_string[usart1_receive_string_len++] = usart1_rx_dma_buffer[i];
                        if(usart1_receive_string_len >= USART1_RX_BUFFER_LEN) {
                                _print("USART1_rx_dma_check()::3 usart1_receive_string overflow, too long command, bidb = %d, urdc = %d, urs_len = %d\r\n", bytes_in_dma_buffer, usart2_rx_dma_counter, usart2_receive_string_len);
                                usart1_receive_string_len = 0;
				continue;
                        }
                        //if(usart1_receive_string[usart1_receive_string_len-1] == '\n' || usart1_receive_string[usart1_receive_string_len-1] == '\r') {
                        if(usart1_receive_string_len) {
                                // Process command
				PostMessageIRQ(USART1_RX_DATA, 1, (int)usart1_receive_string, usart1_receive_string_len);
                                //usart1_receive_string_len = 0;
                        }
                }
                usart1_rx_dma_counter = bytes_in_dma_buffer;
        }
}


void USART2_rx_dma_check(void)
{

	if(DMA1_Stream5_used_by_DAC)
		return; // DMA1_Stream5 is currently occuied by DAC

        int i;
        int bytes_in_dma_buffer = USART2_RX_BUFFER_LEN - DMA_GetCurrDataCounter(DMA1_Stream5); // How much data currently in DMA buffer

        if(bytes_in_dma_buffer > usart2_rx_dma_counter) {
                int bytes_newly_read = bytes_in_dma_buffer - usart2_rx_dma_counter;
                for(i = 0; i < bytes_newly_read; i++ ) {
                        usart2_receive_string[usart2_receive_string_len++] = usart2_rx_dma_buffer[usart2_rx_dma_counter + i];
                        if(usart2_receive_string_len >= USART2_RX_BUFFER_LEN) {
                                _print("USART2_rx_dma_check()::1 usart2_receive_string overflow, too long command, bidb = %d, urdc = %d, urs_len = %d\r\n", bytes_in_dma_buffer, usart2_rx_dma_counter, usart2_receive_string_len);
                                usart2_receive_string_len = 0;
				continue;
                        }
                }
		if(usart2_receive_string_len) {
			PostMessageIRQ(USART2_RX_DATA, 1, (int)usart2_receive_string, usart2_receive_string_len);
			//usart2_receive_string_len = 0;
		}
                usart2_rx_dma_counter += bytes_newly_read;

        } else if(bytes_in_dma_buffer < usart2_rx_dma_counter) {
                // Overlap occured
                int bytes_newly_read = USART2_RX_BUFFER_LEN - usart2_rx_dma_counter;

                // First part
                for(i = 0; i < bytes_newly_read; i++ ) {
                        usart2_receive_string[usart2_receive_string_len++] = usart2_rx_dma_buffer[usart2_rx_dma_counter + i];
                        if(usart2_receive_string_len >= USART2_RX_BUFFER_LEN) {
                                _print("USART2_rx_dma_check()::2 usart2_receive_string overflow, too long command, bidb = %d, urdc = %d, urs_len = %d\r\n", bytes_in_dma_buffer, usart2_rx_dma_counter, usart2_receive_string_len);
                                usart2_receive_string_len = 0;
				continue;
                        }
                }
		if(usart2_receive_string_len) {
			PostMessageIRQ(USART2_RX_DATA, 1, (int)usart2_receive_string, usart2_receive_string_len);
			//usart2_receive_string_len = 0;
		}

                // Second part
                for(i = 0; i < bytes_in_dma_buffer; i++ ) {
                        usart2_receive_string[usart2_receive_string_len++] = usart2_rx_dma_buffer[i];
                        if(usart2_receive_string_len >= USART2_RX_BUFFER_LEN) {
                                _print("USART2_rx_dma_check()::3 usart2_receive_string overflow, too long command, bidb = %d, urdc = %d, urs_len = %d\r\n", bytes_in_dma_buffer, usart2_rx_dma_counter, usart2_receive_string_len);
                                usart2_receive_string_len = 0;
				continue;
                        }
                }
		if(usart2_receive_string_len) {
			PostMessageIRQ(USART2_RX_DATA, 1, (int)usart2_receive_string, usart2_receive_string_len);
			//usart2_receive_string_len = 0;
		}
                usart2_rx_dma_counter = bytes_in_dma_buffer;
        }
}


void USART3_rx_dma_check(void)
{

        int i;
        int bytes_in_dma_buffer = USART3_RX_BUFFER_LEN - DMA_GetCurrDataCounter(DMA1_Stream1); // How much data currently in DMA buffer

        if(bytes_in_dma_buffer > usart3_rx_dma_counter) {
                int bytes_newly_read = bytes_in_dma_buffer - usart3_rx_dma_counter;
                for(i = 0; i < bytes_newly_read; i++ ) {
                        usart3_receive_string[usart3_receive_string_len++] = usart3_rx_dma_buffer[usart3_rx_dma_counter + i];
                        if(usart3_receive_string_len >= USART3_RX_BUFFER_LEN) {
                                _print("USART3_rx_dma_check()::1 usart3_receive_string overflow, too long command, bidb = %d, urdc = %d, urs_len = %d\r\n", bytes_in_dma_buffer, usart2_rx_dma_counter, usart2_receive_string_len);
                                usart2_receive_string_len = 0;
				continue;
                        }
                }
		if(usart3_receive_string_len) {
			PostMessageIRQ(USART3_RX_DATA, 1, (int)usart3_receive_string, usart3_receive_string_len);
			//usart3_receive_string_len = 0;
		}
                usart3_rx_dma_counter += bytes_newly_read;

        } else if(bytes_in_dma_buffer < usart3_rx_dma_counter) {
                // Overlap occured
                int bytes_newly_read = USART3_RX_BUFFER_LEN - usart3_rx_dma_counter;

                // First part
                for(i = 0; i < bytes_newly_read; i++ ) {
                        usart3_receive_string[usart3_receive_string_len++] = usart3_rx_dma_buffer[usart3_rx_dma_counter + i];
                        if(usart3_receive_string_len >= USART3_RX_BUFFER_LEN) {
                                _print("USART3_rx_dma_check()::2 usart3_receive_string overflow, too long command, bidb = %d, urdc = %d, urs_len = %d\r\n", bytes_in_dma_buffer, usart2_rx_dma_counter, usart2_receive_string_len);
                                usart3_receive_string_len = 0;
				continue;
                        }
                }
		if(usart3_receive_string_len) {
			PostMessageIRQ(USART3_RX_DATA, 1, (int)usart3_receive_string, usart3_receive_string_len);
			//usart3_receive_string_len = 0;
		}

                // Second part
                for(i = 0; i < bytes_in_dma_buffer; i++ ) {
                        usart3_receive_string[usart3_receive_string_len++] = usart3_rx_dma_buffer[i];
                        if(usart3_receive_string_len >= USART3_RX_BUFFER_LEN) {
                                _print("USART3_rx_dma_check()::3 usart3_receive_string overflow, too long command, bidb = %d, urdc = %d, urs_len = %d\r\n", bytes_in_dma_buffer, usart2_rx_dma_counter, usart2_receive_string_len);
                                usart3_receive_string_len = 0;
				continue;
                        }
                }
		if(usart3_receive_string_len) {
			PostMessageIRQ(USART3_RX_DATA, 1, (int)usart3_receive_string, usart3_receive_string_len);
			//usart3_receive_string_len = 0;
		}
                usart3_rx_dma_counter = bytes_in_dma_buffer;
        }
}




int _print(const char *format, ...) {

        va_list list;
        va_start(list, format);
        int len = vsnprintf(0, 0, format, list);
        char s[len+1];
        vsprintf(s, format, list);

	if(_usart_tx_mode == USART_TX_DIRECT)
		USART_direct_puts(USART1,s,len);
	else
		USART_puts(USART1,s,len);

        va_end(list);
        return len;
}


int _maxread(USART_TypeDef* USARTx)
{
	int len;

        __disable_irq();
	if(USARTx == USART1) 
		len = usart1_receive_string_len; 
	else if(USARTx == USART2)
		len = usart2_receive_string_len; 
	else if(USARTx == USART3)
		len = usart3_receive_string_len; 
	else
		len = 0;
	__enable_irq();

	return len;
}

void _purge(USART_TypeDef* USARTx)
{
        __disable_irq();
	if(USARTx == USART1) 
        	usart1_receive_string_len = 0;
	else if(USARTx == USART2)
        	usart2_receive_string_len = 0;
	else if(USARTx == USART3)
        	usart3_receive_string_len = 0;
	else {}

        __enable_irq();
}




int _read(USART_TypeDef* USARTx, char *buf, int len, int timeout)
{
	int bytes_read;

        int n;
	int read_flag = 0;

        for(n = 0; n < timeout; n++) {

		if(USARTx == USART1) {

			__disable_irq();
			USART1_rx_dma_check();
			__enable_irq();

			if(usart1_receive_string_len) {
       		         	// Retrieve small chunk of data from I/O buffer
				__disable_irq();
				bytes_read = MIN(usart1_receive_string_len, len);
				memmove(buf, (void*)usart1_receive_string, bytes_read);
				memmove((void*)usart1_receive_string, (void*)usart1_receive_string+bytes_read, usart1_receive_string_len - bytes_read);
				usart1_receive_string_len -= bytes_read;
				__enable_irq();
	
				return bytes_read;
			}

		} else if(USARTx == USART2) {

			__disable_irq();
			USART2_rx_dma_check();
			__enable_irq();

			if(usart2_receive_string_len) {
       		         	// Retrieve small chunk of data from I/O buffer
				__disable_irq();
				bytes_read = MIN(usart2_receive_string_len, len);
				memmove(buf, (void*)usart2_receive_string, bytes_read);
				memmove((void*)usart2_receive_string, (void*)usart2_receive_string+bytes_read, usart2_receive_string_len - bytes_read);
				usart2_receive_string_len -= bytes_read;
				__enable_irq();
	
				return bytes_read;
			}

		} else if(USARTx == USART3) {

			__disable_irq();
			USART3_rx_dma_check();
			__enable_irq();

			if(usart3_receive_string_len) {
       		         	// Retrieve small chunk of data from I/O buffer
				__disable_irq();
				bytes_read = MIN(usart3_receive_string_len, len);
				memmove(buf, (void*)usart3_receive_string, bytes_read);
				memmove((void*)usart3_receive_string, (void*)usart3_receive_string+bytes_read, usart3_receive_string_len - bytes_read);
				usart3_receive_string_len -= bytes_read;
				__enable_irq();
	
				return bytes_read;
			}

		} else {
			break;
		}

		Delay(1);
        }

	return 0;
}


void _flush(USART_TypeDef* USARTx)
{
        int i;

        __disable_irq();

	if(USARTx == USART1) {
        	for(i = 0; i < USART1_TX_NUM_OF_BUFS; i++) {
                	usart1_tx_ringbuf[i].cur_size == 0;
        	}
	} else if(USARTx == USART2) {
        	for(i = 0; i < USART2_TX_NUM_OF_BUFS; i++) {
                	usart2_tx_ringbuf[i].cur_size == 0;
        	}
	} else if(USARTx == USART3) {
        	for(i = 0; i < USART3_TX_NUM_OF_BUFS; i++) {
                	usart3_tx_ringbuf[i].cur_size == 0;
        	}
	}

        __enable_irq();

}


