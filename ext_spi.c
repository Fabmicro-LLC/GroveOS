
#include "hardware.h"

#ifdef EXT_SPI

#include "ext_spi.h"

#include <string.h>
#include <stm32f4xx_dma.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "msg.h"
#include "utils.h"
#include "config.h"


extern uint8_t event_logging;

uint8_t ext_spi_buf[4];


int ext_spi_dma_request(char* tx_buf, char* rx_buf, int xfer_len)
{

	if(! tx_buf && !rx_buf && xfer_len < 1) 
		return -1;

	if(!config_active.spi.enabled)
		return -2;

	if(tx_buf) {

		DMA_InitTypeDef DMA_InitStructure;

		DMA_DeInit(EXT_SPI_DMA_TX_STREAM); // TX

		/* Configure DMA Initialization Structure */
		DMA_InitStructure.DMA_BufferSize = xfer_len ;
		DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
		DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull ;
		DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		if(config_active.spi.mode == EXT_SPI_MODE_CONTINUOUS) {
			DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
		} else if(config_active.spi.mode == EXT_SPI_MODE_SINGLE) {
			DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		}


		DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t) (&(EXT_SPI->DR)) ;
		DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;

                /* Configure TX DMA */
                DMA_InitStructure.DMA_Channel = EXT_SPI_DMA_CHANNEL ;
                DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral ;
                DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)tx_buf ;
                DMA_Init(EXT_SPI_DMA_TX_STREAM, &DMA_InitStructure);

        	DMA_ClearFlag(EXT_SPI_DMA_TX_STREAM, EXT_SPI_DMA_TX_TCFLAG);


		// Configure and enable DMA IRQ
		NVIC_InitTypeDef   NVIC_InitStructure;
		DMA_ITConfig(EXT_SPI_DMA_TX_STREAM, DMA_IT_TC, ENABLE);
		NVIC_InitStructure.NVIC_IRQChannel = EXT_SPI_DMA_TX_STREAM_IRQ;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);


		// Configure SPI RXNE IRQ
		NVIC_InitStructure.NVIC_IRQChannel = EXT_SPI_IRQ;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);


		gpio_set(EXT_SPI_CS_GPIO_PORT, EXT_SPI_CS_GPIO_PIN, 0); // CS = Low - Begin of transaction 

		DMA_Cmd(EXT_SPI_DMA_TX_STREAM, ENABLE);

		SPI_I2S_DMACmd(EXT_SPI, SPI_I2S_DMAReq_Tx, ENABLE);

		SPI_Cmd(EXT_SPI, ENABLE);

		if(event_logging)
			_print("ext_spi_dma_request() TX started, tx_buf = %p, xfer_len = %d\r\n", tx_buf, xfer_len);


	}

/*
	if(rx_buf) {
        	DMA_MemoryTargetConfig(EXT_SPI_DMA_RX_STREAM, (uint32_t)rx_buf , DMA_Memory_0);
        	DMA_SetCurrDataCounter(EXT_SPI_DMA_RX_STREAM, xfer_len);

        	DMA_ClearFlag(EXT_SPI_DMA_RX_STREAM, DMA_FLAG_TCIF3 | DMA_FLAG_FEIF3 | DMA_FLAG_DMEIF3 | DMA_FLAG_TEIF3 | DMA_FLAG_HTIF3);

        	DMA_Cmd(EXT_SPI_DMA_RX_STREAM, ENABLE);
	}
*/

	return 0;
}

void EXT_SPI_DMA_TX_STREAM_IRQ_HANDLER(void) {

        __disable_irq();

        if(DMA_GetITStatus(EXT_SPI_DMA_TX_STREAM, EXT_SPI_DMA_IT)) {

/*
		// Poll TX Complete flag

		for(int i = 0; i < 99999; i++)
			if(SPI_GetFlagStatus(EXT_SPI, SPI_FLAG_BSY) == 0)
				break;
		DelayLoopMicro(10);

		gpio_set(EXT_SPI_CS_GPIO_PORT, EXT_SPI_CS_GPIO_PIN, 1); // CS = High - end of transaction

               	PostMessageIRQ(SPI_DMA_TX_TC, 1, 0, 0);
*/
		SPI_I2S_ITConfig(EXT_SPI, SPI_IT_RXNE,ENABLE);

                DMA_ClearITPendingBit(EXT_SPI_DMA_TX_STREAM, EXT_SPI_DMA_IT);

		if(event_logging)
			_print("EXT_DMA_TX_STREAM_IRQ_HANDLER: TX complete!\r\n");

        }

        __enable_irq();
}


// Hack: this is the only way to fetect and of transmisstion of last bit over SPI on STM32F4.
void EXT_SPI_IRQ_HANDLER(void)
{

	__disable_irq();

	if (SPI_GetITStatus(EXT_SPI, SPI_IT_RXNE)) {
		SPI_I2S_ITConfig(EXT_SPI, SPI_IT_RXNE, DISABLE);
		SPI_ReceiveData(EXT_SPI); // Reading data registere clears RXNE flag
		PostMessageIRQ(SPI_DMA_TX_TC, 1, 0, 0);
		gpio_set(EXT_SPI_CS_GPIO_PORT, EXT_SPI_CS_GPIO_PIN, 1); // CS = High - end of transaction
	}

        __enable_irq();
}


void ext_spi_init(void)
{
	print("EXT SPI initializing...\r\n");


        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

        RCC_APB2PeriphClockCmd(EXT_SPI_PERIPH, ENABLE);
        RCC_AHB1PeriphClockCmd(EXT_SPI_DMA_PERIPH, ENABLE);

        gpio_init(EXT_SPI_CS_GPIO_PERIPH, EXT_SPI_GPIO_SPEED, EXT_SPI_CS_GPIO_PORT, EXT_SPI_CS_GPIO_PIN, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_UP);
        gpio_init(EXT_SPI_SCK_GPIO_PERIPH, EXT_SPI_GPIO_SPEED, EXT_SPI_SCK_GPIO_PORT, EXT_SPI_SCK_GPIO_PIN, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP);
        gpio_init(EXT_SPI_MISO_GPIO_PERIPH, EXT_SPI_GPIO_SPEED, EXT_SPI_MISO_GPIO_PORT, EXT_SPI_MISO_GPIO_PIN, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP);
        gpio_init(EXT_SPI_MOSI_GPIO_PERIPH, EXT_SPI_GPIO_SPEED, EXT_SPI_MOSI_GPIO_PORT, EXT_SPI_MOSI_GPIO_PIN, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP);

        // Connect SPI pins to AF5
        //GPIO_PinAFConfig(EXT_SPI_CS_GPIO_PORT, EXT_SPI_CS_GPIO_PIN_SRC, EXT_SPI_AF);//CS
        GPIO_PinAFConfig(EXT_SPI_SCK_GPIO_PORT, EXT_SPI_SCK_GPIO_PIN_SRC, EXT_SPI_AF);//SCK
        GPIO_PinAFConfig(EXT_SPI_MISO_GPIO_PORT, EXT_SPI_MISO_GPIO_PIN_SRC, EXT_SPI_AF);//MISO
        GPIO_PinAFConfig(EXT_SPI_MOSI_GPIO_PORT, EXT_SPI_MOSI_GPIO_PIN_SRC, EXT_SPI_AF);//MOSI

        /* SPI configuration -------------------------------------------------------*/
	SPI_InitTypeDef  SPI_InitStructure;
        SPI_DeInit(EXT_SPI);
        SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
        SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
        SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;

	if(config_active.spi.flags & 0x02) {
        	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	} else {
        	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	}

	if(config_active.spi.flags & 0x04) {
        	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	} else {
        	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	}

        SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
        SPI_InitStructure.SPI_BaudRatePrescaler = config_active.spi.prescaler;
       	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; // this does not take effect because of a bug in the library (wrong CR1_CLEAR_MASK)


        SPI_InitStructure.SPI_CRCPolynomial = 7;
        SPI_Init(EXT_SPI, &SPI_InitStructure);

	//SPI_SSOutputCmd(EXT_SPI, ENABLE);


	// In standard library CR1_CLEAR_MASK is defind in such a way that it does not clear LSB bit, any changes to SPI_FirstBit does not take effect.  
	// Hence, we have to set LSB bit manually as below:

	if(config_active.spi.flags & 0x01) {
		EXT_SPI->CR1 &= ~0x80;
		print("EXT SPI firstbit: MSB\r\n");
	} else {
		EXT_SPI->CR1 |= 0x80;
		print("EXT SPI firstbit: LSB\r\n");
	}

	print("EXT SPI initialization done!\r\n");

}

void ext_spi_stop(void)
{
	DMA_Cmd(EXT_SPI_DMA_TX_STREAM, DISABLE);
}


void ext_spi_deinit(void)
{
        // Configure and enable DMA IRQ
        NVIC_InitTypeDef   NVIC_InitStructure;
        DMA_ITConfig(EXT_SPI_DMA_TX_STREAM, DMA_IT_TC, ENABLE);
        NVIC_InitStructure.NVIC_IRQChannel = EXT_SPI_DMA_TX_STREAM_IRQ;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
        NVIC_Init(&NVIC_InitStructure);

	DMA_Cmd(EXT_SPI_DMA_TX_STREAM, DISABLE);
        SPI_DeInit(EXT_SPI);
}


void ext_spi_send8(uint32_t val)
{
	ext_spi_buf[0] = val;
	ext_spi_dma_request(ext_spi_buf, NULL, 1);	
}


void ext_spi_send16(uint32_t data)
{
	ext_spi_buf[0] = (data >> 8) & 0xff;
	ext_spi_buf[1] = (data >> 0) & 0xff;
	ext_spi_dma_request(ext_spi_buf, NULL, 2);	
}

void ext_spi_send24(uint32_t data)
{
	ext_spi_buf[0] = (data >> 16) & 0xff;
	ext_spi_buf[1] = (data >> 8) & 0xff;
	ext_spi_buf[2] = (data >> 0) & 0xff;
	ext_spi_dma_request(ext_spi_buf, NULL, 3);	
}

void ext_spi_send32(uint32_t data)
{
	ext_spi_buf[0] = (data >> 24) & 0xff;
	ext_spi_buf[1] = (data >> 16) & 0xff;
	ext_spi_buf[2] = (data >> 8) & 0xff;
	ext_spi_buf[3] = (data >> 0) & 0xff;
	ext_spi_dma_request(ext_spi_buf, NULL, 4);	
}

#endif

