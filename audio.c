/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2017-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include "utils.h"
#include "audio.h"
#include "msg.h"
#include "g711.h"
#include "tfs.h"

int PCMU_DECODE = 1;

char* play_buf = NULL;
int play_buf_len = 0;
int play_buf_offset = 0;
int play_buf_empty = 0;

char dac_buf[DAC_BUF_SIZE];

int tim6_counter;
int play_dma_ht_counter;
int play_dma_tc_counter;
int error_play_buf_underrun;

int DMA1_Stream5_used_by_DAC = 0;

#ifdef USART_EXT1
extern void USART2_init(void);
#endif

void enable_audio_amp(int enable) {
	if(enable) {
		GPIO_ResetBits(GPIOB, GPIO_Pin_7); // Pull Low to enable audio amplifier
	} else {
		GPIO_SetBits(GPIOB, GPIO_Pin_7); // Pull Up to disable audio amplifier
	}
}

void play_stop() {

	print("play_stop(), buffer = %p, len = %d\r\n", play_buf, play_buf_len);

	enable_audio_amp (0);

	DMA_Cmd(DMA1_Stream5, DISABLE);
	DMA_DeInit(DMA1_Stream5);
	DAC_DeInit();

	DMA1_Stream5_used_by_DAC = 0;

	play_buf_len = 0;
	play_buf_empty = 1;

}

//TFS_HEADER* audio_tfs;

void play_start(unsigned char *Buffer, int Len) {

	print("play_start(), buffer = %p, len = %d\n", Buffer, Len);

	play_buf = Buffer;
	play_buf_len = Len;
	play_buf_offset = 0;
	play_buf_empty = 0;


	if(play_buf_len > DAC_BUF_SIZE/2) {
		//memmove(dac_buf, play_buf, DAC_BUF_SIZE);
		pcmu_decode(dac_buf, play_buf, DAC_BUF_SIZE/2);
//tfs_write_block(audio_tfs, dac_buf, DAC_BUF_SIZE/2*2, play_buf_offset*2);
		play_buf_len-= DAC_BUF_SIZE/2;
		play_buf_offset+= DAC_BUF_SIZE/2;
	} else if(play_buf_len > 0) {
		//memset16((unsigned short*)(dac_buf+0), 0x0800, DAC_BUF_SIZE/2);
		memset16((unsigned short*)(dac_buf+0), 0x8000, DAC_BUF_SIZE/2);
		//memmove(dac_buf, play_buf, play_buf_len);
		pcmu_decode(dac_buf, play_buf, play_buf_len);
//tfs_write_block(audio_tfs, dac_buf, play_buf_len*2, play_buf_offset*2);
		play_buf_offset+= play_buf_len;
		play_buf_len = 0;
	}

//	audio_tfs = tfs_create_file("audio.raw", 10, Len*2);

	play_init(); //Init timer, dac and DMA, start DMA transfer


	enable_audio_amp(1);
}

void play_ht(void)
{

                        if(play_buf_len>= FRAME_SIZE_SAMPLES) {
                                //memmove(dac_buf+0, play_buf+play_buf_offset, FRAME_SIZE_BYTES);
                                pcmu_decode(dac_buf+0, play_buf+play_buf_offset, FRAME_SIZE_SAMPLES);
//tfs_write_block(audio_tfs, dac_buf+0, FRAME_SIZE_SAMPLES*2, play_buf_offset*2);
                                play_buf_len-= FRAME_SIZE_SAMPLES;
                                play_buf_offset+= FRAME_SIZE_SAMPLES;

                        } else if(play_buf_len > 0) {
                                //memset16((unsigned short*)(dac_buf+0), 0x0800, FRAME_SIZE_SAMPLES);
                                memset16((unsigned short*)(dac_buf+0), 0x8000, FRAME_SIZE_SAMPLES);
                                //memmove(dac_buf+0, play_buf+play_buf_offset, play_buf_len);
                                pcmu_decode(dac_buf+0, play_buf+play_buf_offset, play_buf_len);
//tfs_write_block(audio_tfs, dac_buf+0, play_buf_len*2, play_buf_offset*2);
                                play_buf_offset+= play_buf_len;
                                play_buf_len = 0;
                        } else {
//tfs_close(audio_tfs);
                                //memset16((unsigned short*)(dac_buf+0), 0x0800, FRAME_SIZE_SAMPLES);
                                memset16((unsigned short*)(dac_buf+0), 0x8000, FRAME_SIZE_SAMPLES);
                                play_buf_empty = 1;
                                play_stop();
	
				#ifdef USART_EXT1
                                USART2_init(); // Init RS485 which is sharing same DMA stream
				#endif
                        }

}

void play_tc(void)
{

                        if(play_buf_len>=FRAME_SIZE_SAMPLES) {
                                //memmove(dac_buf+FRAME_SIZE_BYTES, play_buf+play_buf_offset, FRAME_SIZE_BYTES);
                                pcmu_decode(dac_buf+FRAME_SIZE_BYTES, play_buf+play_buf_offset, FRAME_SIZE_SAMPLES);
//tfs_write_block(audio_tfs, dac_buf+FRAME_SIZE_BYTES, FRAME_SIZE_SAMPLES*2, play_buf_offset*2);
                                play_buf_len-= FRAME_SIZE_SAMPLES;
                                play_buf_offset+= FRAME_SIZE_SAMPLES;
                        } else if(play_buf_len > 0) {
                                //memset16((unsigned short*)(dac_buf+FRAME_SIZE_BYTES), 0x0800, FRAME_SIZE_SAMPLES);
                                memset16((unsigned short*)(dac_buf+FRAME_SIZE_BYTES), 0x8000, FRAME_SIZE_SAMPLES);
                                //memmove(dac_buf+FRAME_SIZE_BYTES, play_buf+play_buf_offset, play_buf_len);
                                pcmu_decode(dac_buf+FRAME_SIZE_BYTES, play_buf+play_buf_offset, play_buf_len);
//tfs_write_block(audio_tfs, dac_buf+FRAME_SIZE_BYTES, play_buf_len*2, play_buf_offset*2);
                                play_buf_offset+= play_buf_len;
                                play_buf_len = 0;
                        } else {
//tfs_close(audio_tfs);
                                //memset16((unsigned short*)(dac_buf+FRAME_SIZE_BYTES), 0x0800, FRAME_SIZE_SAMPLES);
                                memset16((unsigned short*)(dac_buf+FRAME_SIZE_BYTES), 0x8000, FRAME_SIZE_SAMPLES);
                                play_buf_empty = 1;
                                play_stop();

				#ifdef USART_EXT1
                                USART2_init(); // Init RS485 which is sharing same DMA stream
				#endif
                        }

}



void play_init() {
	print("play_init()\r\n");

	DMA1_Stream5_used_by_DAC = 1;

	play_dma_ht_counter=0;
	play_dma_tc_counter=0;

	error_play_buf_underrun = 0;

	DAC_InitTypeDef  DAC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure; 
        
	DMA_Cmd(DMA1_Stream5, DISABLE);
	DMA_DeInit(DMA1_Stream5);
	DAC_DeInit();

	//audio ampl. enable
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// DAC channel 1 (DAC_OUT1 = PA.4)configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);



	// DAC channel1 Configuration 
	//RCC_AHB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE); 
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);



	DMA_InitStructure.DMA_Channel = DMA_Channel_7; ///DMA_Channel_7;
	//DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&DAC->DHR12R1;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&DAC->DHR12L1;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)dac_buf;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = DAC_BUF_SIZE/2; //number of samples !!!
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

	DMA_Init(DMA1_Stream5, &DMA_InitStructure);

	DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_TCIF5 | DMA_FLAG_FEIF5 | DMA_FLAG_DMEIF5 | DMA_FLAG_TEIF5 | DMA_FLAG_HTIF5);
	//DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_TCIF5 | DMA_FLAG_HTIF5);

	DMA_ITConfig(DMA1_Stream5, DMA_IT_TC | DMA_IT_HT | DMA_IT_TE | DMA_IT_DME | DMA_IT_FE, ENABLE);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);


	// Moved to main.c
/*
	// TIM6 Periph clock enable 
	TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 84 -1;
	TIM_TimeBaseStructure.TIM_Period = 125 - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

	TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);
	TIM_Cmd(TIM6, ENABLE);
*/

	DAC_Cmd(DAC_Channel_1, ENABLE);
	DAC_DMACmd(DAC_Channel_1, ENABLE);
	DMA_Cmd(DMA1_Stream5, ENABLE);

}



