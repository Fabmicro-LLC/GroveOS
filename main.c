/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2017-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>

#include "main.h"
#include "utils.h"
#include "lcd-ft800.h"
#include "msg.h"
#include "hardware.h"
#include "work.h"
#include "adc.h"

#ifdef EXT_SPI
#include "ext_spi.h"
#endif

#ifdef DALI1_MODE
#include "dali1.h"
#endif

#ifdef DALI2_MODE
#include "dali2.h"
#endif

#include "config.h"
#include "logger.h"

#include <stm32f4xx.h>
#include <misc.h>	
#include <stm32f4xx_usart.h>
#include "stm32f4xx_it.h"
#include "utf8.h"
#include "tfs.h"

#ifdef OLED_DATA_GPIO_PORT 
#include "oled.h"
#endif

#include "softtimer.h"
#include "audio.h"
#include "ext_gpio.h"
#include "ext_irq.h"
#include "listener.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

uint32_t microsec_timestamp=0;
uint32_t millisec_timestamp=0;

static void tim3_init();
static void tim4_init();
static void tim5_init();
static void tim6_init();
static void (*work_process_message)(MSG* msg);

extern volatile unsigned int uwTimingDelay;
extern int exception_code;
extern void (*application_process_msg_addr)(MSG*);
extern uint32_t application_text_addr;

void canit(void); // zmodem cancel transfer fuction

// Below are define in logger.c and depend on hardware
extern const unsigned int FLASH_SECTOR2ADDR[];
extern const unsigned int FLASH_SECTOR2NUMBER[];



int main(void) {
        __disable_irq();


        USART1_init(); // Debug/console


        RCC_ClocksTypeDef    RCC_Clocks;
        RCC_GetClocksFreq(&RCC_Clocks);
        SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

	print("\r\n\r\nGroveOS ver %04d, product: %s. Copyright (C) Fabmicro, LLC. Tyumen, Russia, 2017-2021.\r\n\r\n", _DEVICE_NAME_, BUILD_NUMBER, OS_PRODUCT_NAME);
	print("RCC_Clocks.HCLK_Frequency = %d, SystemCoreClock = %d, RCC_Clocks.PCLK1_Frequency = %d \r\n", RCC_Clocks.HCLK_Frequency, SystemCoreClock,RCC_Clocks.PCLK1_Frequency);
	print("UTF8=°йцукен\r\n");


	tim5_init(); // system ms timer
	tim3_init(); // PWM for AC 
	tim4_init(); // PWM for DC 
	tim6_init(); // DAC timer

	ext_irq_init();
	listener_init();

	#ifdef OLED_DATA_GPIO_PORT 
	oled_init();
	char str[17];
	snprintf(str, 17, "%s ver %04d", _DEVICE_NAME_, BUILD_NUMBER); 
        OLED_WEG010032_PRINT(16, str, 0, 0);
        OLED_WEG010032_PRINT(10, "Loading...", 18, 1);
	#endif

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);

        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

        __enable_irq();


	if(load_config() == 0) {
		TFS_HEADER *tfs = tfs_find(SRAM_BACKUP_FILENAME);
		if(tfs) {

			int bytes_to_write = MIN(tfs->size, 4096);

			print("SRAM Backup Memory file found, uploading %d bytes...\r\n");

			if(write_to_backup_sram((char*)tfs + sizeof(TFS_HEADER), bytes_to_write, 0) != bytes_to_write) {
				print("Failed to write to backup memory!\r\n");
			} else {
				print("SRAM Backup Memory restored OK\r\n");
				tfs_delete(tfs);
			}
		}
	}

	if(config_active.use_rtc) 
		RTC_init();

	if(config_active.logger_enabled) 
		logger_init();

	#ifdef GPIO_CHANNELS
	if(config_active.gpios_enabled) 
		ext_gpio_init();
	#endif

	#ifdef USART_EXT1
	if(config_active.ext1_port_modbus_mode) 
        	USART2_init(); // EXT1 RS485/Modbus
	#endif

	#ifdef USART_EXT2
	if(config_active.ext2_port_modbus_mode)
        	USART3_init(); // EXT2 RS485/Modbus
	#endif

	#ifdef DALI1_MODE
	if(config_active.dali1.mode)
		dali1_init();
	#endif

	#ifdef DALI2_MODE
	if(config_active.dali2.mode)
		dali2_init();
	#endif

	if(config_active.adc.enabled) 
		adc_init();

	#ifdef LCD_SPI
	if(config_active.lcd.enabled) 
		lcd_init();
	#endif

	#ifdef EXT_SPI
	if(config_active.spi.enabled) 
		ext_spi_init();
	#endif

	gpio_init(LED_PERIPH, GPIO_SPEED, LED_GPIO_PORT, LED_GPIO_PIN, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_NOPULL); // Green info LED

	// Keys
	#ifdef KEY_UP_GPIO_PORT
	gpio_init(KEY_UP_PERIPH, GPIO_SPEED, KEY_UP_GPIO_PORT, KEY_UP_GPIO_PIN, GPIO_Mode_IN, GPIO_OType_PP, GPIO_PuPd_UP); // Key UP 
        gpio_irq(KEY_UP_IRQ_PORT_SRC, KEY_UP_IRQ_PIN_SRC, KEY_UP_IRQ_LINE, KEY_UP_IRQ_CHAN, KEY_UP_IRQ_RAISFALL); // IRQ from UP key
	#endif

	#ifdef KEY_DOWN_GPIO_PORT
	gpio_init(KEY_DOWN_PERIPH, GPIO_SPEED, KEY_DOWN_GPIO_PORT, KEY_DOWN_GPIO_PIN, GPIO_Mode_IN, GPIO_OType_PP, GPIO_PuPd_UP); // Key DOWN 
        gpio_irq(KEY_DOWN_IRQ_PORT_SRC, KEY_DOWN_IRQ_PIN_SRC, KEY_DOWN_IRQ_LINE, KEY_DOWN_IRQ_CHAN, KEY_DOWN_IRQ_RAISFALL); // IRQ from DOWN key
	#endif

	#ifdef KEY_LEFT_GPIO_PORT
	gpio_init(KEY_LEFT_PERIPH, GPIO_SPEED, KEY_LEFT_GPIO_PORT, KEY_LEFT_GPIO_PIN, GPIO_Mode_IN, GPIO_OType_PP, GPIO_PuPd_UP); // Key LEFT 
        gpio_irq(KEY_LEFT_IRQ_PORT_SRC, KEY_LEFT_IRQ_PIN_SRC, KEY_LEFT_IRQ_LINE, KEY_LEFT_IRQ_CHAN, KEY_LEFT_IRQ_RAISFALL); // IRQ from LEFT key
	#endif

	#ifdef KEY_RIGHT_GPIO_PORT
	gpio_init(KEY_RIGHT_PERIPH, GPIO_SPEED, KEY_RIGHT_GPIO_PORT, KEY_RIGHT_GPIO_PIN, GPIO_Mode_IN, GPIO_OType_PP, GPIO_PuPd_UP); // Key RIGHT 
        gpio_irq(KEY_RIGHT_IRQ_PORT_SRC, KEY_RIGHT_IRQ_PIN_SRC, KEY_RIGHT_IRQ_LINE, KEY_RIGHT_IRQ_CHAN, KEY_RIGHT_IRQ_RAISFALL); // IRQ from RIGHT key
	#endif


	// AC
	#ifdef PWM1_GPIO_PORT
	gpio_init(PWM1_PERIPH, GPIO_SPEED, PWM1_GPIO_PORT, PWM1_GPIO_PIN, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP); // AC1 
	gpio_pinconfig(PWM1_GPIO_PORT, PWM1_PIN_SRC, PWM1_PIN_AFCONFIG);
	#endif

	#ifdef PWM2_GPIO_PORT
	gpio_init(PWM2_PERIPH, GPIO_SPEED, PWM2_GPIO_PORT, PWM2_GPIO_PIN, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP); // AC2 
	gpio_pinconfig(PWM2_GPIO_PORT, PWM2_PIN_SRC, PWM2_PIN_AFCONFIG);
	#endif

	#ifdef PWM3_GPIO_PORT
	gpio_init(PWM3_PERIPH, GPIO_SPEED, PWM3_GPIO_PORT, PWM3_GPIO_PIN, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP); // AC3 
	gpio_pinconfig(PWM3_GPIO_PORT, PWM3_PIN_SRC, PWM3_PIN_AFCONFIG);
	#endif

	#ifdef PWM4_GPIO_PORT
	gpio_init(PWM4_PERIPH, GPIO_SPEED, PWM4_GPIO_PORT, PWM4_GPIO_PIN, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP); // AC4 
	gpio_pinconfig(PWM4_GPIO_PORT, PWM4_PIN_SRC, PWM4_PIN_AFCONFIG);
	#endif

	// DC
	#ifdef PWM5_GPIO_PORT
	gpio_init(PWM5_PERIPH, GPIO_SPEED, PWM5_GPIO_PORT, PWM5_GPIO_PIN, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP); // DC1 
	gpio_pinconfig(PWM5_GPIO_PORT, PWM5_PIN_SRC, PWM5_PIN_AFCONFIG);
	#endif

	#ifdef PWM6_GPIO_PORT
	gpio_init(PWM6_PERIPH, GPIO_SPEED, PWM6_GPIO_PORT, PWM6_GPIO_PIN, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP); // DC2 
	gpio_pinconfig(PWM6_GPIO_PORT, PWM6_PIN_SRC, PWM6_PIN_AFCONFIG);
	#endif

	#ifdef PWM7_GPIO_PORT
	gpio_init(PWM7_PERIPH, GPIO_SPEED, PWM7_GPIO_PORT, PWM7_GPIO_PIN, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP); // DC3 
	gpio_pinconfig(PWM7_GPIO_PORT, PWM7_PIN_SRC, PWM7_PIN_AFCONFIG);
	#endif

	#ifdef PWM8_GPIO_PORT
	gpio_init(PWM8_PERIPH, GPIO_SPEED, PWM8_GPIO_PORT, PWM8_GPIO_PIN, GPIO_Mode_AF, GPIO_OType_PP, GPIO_PuPd_UP); // DC4 
	gpio_pinconfig(PWM8_GPIO_PORT, PWM8_PIN_SRC, PWM8_PIN_AFCONFIG);
	#endif

	work_process_message = &process_message;


	//Delay(100);

	main_queue_init();
        softtimer_init();
	init();
	

        MSG msg;
        int rc=0;

	main_loop:

	prompt();
	main_queue_enabled = 1;

	if(SaveCPUState(&saved_cpu_state)==0) {
	        while (1) {
        	        rc=GetMessage(&msg);

			if(rc == -2) {
        	                continue;
                	} else if(rc) {
				print("main() GetMessage() failed!\r\n");
				Delay(100);
				continue;
			}

			(*work_process_message)(&msg);
		}
	} else {
		//_purge();
		canit();
		canit();
        	debug_mode = DEBUG_TO_USART1_AND_SWD;
        	main_queue_enabled = 1;
		_print("\r\n");
		_print("main() Exception in main loop, restarting loop!\r\n");
		goto main_loop;
	}

}


void TIM5_IRQHandler(void) {
        //1-us resolution timer
        __disable_irq();

	if (TIM_GetITStatus(TIM5, TIM_IT_Update) == SET) {  
       		microsec_timestamp++;

		if(microsec_timestamp % (416 / TIM5_RESOLUTION) == 0) {
			// 0.104 MS (2400Hz)

			#ifdef DALI1_MODE
			if(config_active.dali1.mode)
				dali1_transmit_one_bit();
			#endif

			#ifdef DALI2_MODE
			if(config_active.dali2.mode)
				dali2_transmit_one_bit();
			#endif
		}


		if(microsec_timestamp % (1000 / TIM5_RESOLUTION) == 0) {
			// 1 MS (1000Hz)

       			millisec_timestamp++;

			if (uwTimingDelay != 0x00)
				uwTimingDelay--;

			softtimer_check();

			if(millisec_timestamp % 10 == 0) { 
				// 10 MS (100Hz)
				PostMessageIRQ(INFO_TIMER_INT, 0, millisec_timestamp, 0);
			}


			switch(millisec_timestamp & 0x03) { // Same as modulo 4
				case 0: USART1_rx_dma_check(); // Look into USART1 RX DMA buffer for input data
					break;
				case 1: USART2_rx_dma_check(); // Look into USART2 RX DMA buffer for input data
					break;
				case 2: USART3_rx_dma_check(); // Look into USART3 RX DMA buffer for input data
					break;
				default:
					break;
			}

		}


		// 1/8 MS (8000 Hz)
		if(config_active.microsec_timer_enabled) {
			PostMessageIRQ(MICRO_TIMER, 0, microsec_timestamp, 0);
		}


	        TIM_ClearITPendingBit(TIM5, TIM_IT_Update); // clear interrupt flag
	}

        __enable_irq();
}


void tim3_init() { // AC ports PWM counter
	TIM_OCInitTypeDef TIM_OCStruct;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
        TIM_TimeBaseInitTypeDef    timer;
        TIM_TimeBaseStructInit(&timer);
        timer.TIM_Prescaler = 1 -1;
        timer.TIM_Period = 840-1; // 100kHz sec PWM timer
        timer.TIM_ClockDivision = TIM_CKD_DIV1;
	timer.TIM_RepetitionCounter = 0;
        timer.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_TimeBaseInit(TIM3, &timer);
	TIM_Cmd(TIM3, ENABLE);

	/* PWM mode 2 = Clear on compare match */
	/* PWM mode 1 = Set on compare match */
	TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
    
/*
    To get proper duty cycle, you have simple equation
    
    pulse_length = ((TIM_Period + 1) * DutyCycle) / 100 - 1
    
    where DutyCycle is in percent, between 0 and 100%
    
    25% duty cycle:     pulse_length = ((8399 + 1) * 25) / 100 - 1 = 2099
    50% duty cycle:     pulse_length = ((8399 + 1) * 50) / 100 - 1 = 4199
    75% duty cycle:     pulse_length = ((8399 + 1) * 75) / 100 - 1 = 6299
    100% duty cycle:    pulse_length = ((8399 + 1) * 100) / 100 - 1 = 8399
    
    Remember: if pulse_length is larger than TIM_Period, you will have output HIGH all the time
*/

	TIM_OCStruct.TIM_Pulse = 0;
	TIM_OC1Init(TIM3, &TIM_OCStruct);
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    
	TIM_OCStruct.TIM_Pulse = 0;
	TIM_OC2Init(TIM3, &TIM_OCStruct);
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
    
	TIM_OCStruct.TIM_Pulse = 0;
	TIM_OC3Init(TIM3, &TIM_OCStruct);
	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
    
	TIM_OCStruct.TIM_Pulse = 0;
	TIM_OC4Init(TIM3, &TIM_OCStruct);
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
}

void tim4_init() { // DC ports PWM counter
        TIM_TimeBaseInitTypeDef    timer;
	TIM_OCInitTypeDef TIM_OCStruct;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
        TIM_TimeBaseStructInit(&timer);
        timer.TIM_Prescaler = 1 -1;
        timer.TIM_Period = 840-1; // 100kHz sec PWM timer
        timer.TIM_ClockDivision = TIM_CKD_DIV1;
	timer.TIM_RepetitionCounter = 0;
        timer.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_TimeBaseInit(TIM4, &timer);
	TIM_Cmd(TIM4, ENABLE);
    
	/* PWM mode 2 = Clear on compare match */
	/* PWM mode 1 = Set on compare match */
	TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
    
/*
    To get proper duty cycle, you have simple equation
    
    pulse_length = ((TIM_Period + 1) * DutyCycle) / 100 - 1
    
    where DutyCycle is in percent, between 0 and 100%
    
    25% duty cycle:     pulse_length = ((8399 + 1) * 25) / 100 - 1 = 2099
    50% duty cycle:     pulse_length = ((8399 + 1) * 50) / 100 - 1 = 4199
    75% duty cycle:     pulse_length = ((8399 + 1) * 75) / 100 - 1 = 6299
    100% duty cycle:    pulse_length = ((8399 + 1) * 100) / 100 - 1 = 8399
    
    Remember: if pulse_length is larger than TIM_Period, you will have output HIGH all the time
*/
	TIM_OCStruct.TIM_Pulse = 0;
	TIM_OC1Init(TIM4, &TIM_OCStruct);
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
    
	TIM_OCStruct.TIM_Pulse = 0;
	TIM_OC2Init(TIM4, &TIM_OCStruct);
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
    
	TIM_OCStruct.TIM_Pulse = 0;
	TIM_OC3Init(TIM4, &TIM_OCStruct);
	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
    
	TIM_OCStruct.TIM_Pulse = 0;
	TIM_OC4Init(TIM4, &TIM_OCStruct);
	TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
}


void tim5_init() {
	NVIC_InitTypeDef nvicStructure;
	nvicStructure.NVIC_IRQChannel = TIM5_IRQn;
	nvicStructure.NVIC_IRQChannelPreemptionPriority = 0;
	nvicStructure.NVIC_IRQChannelSubPriority = 1;
	nvicStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvicStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
        TIM_TimeBaseInitTypeDef    timer;
        TIM_TimeBaseStructInit(&timer);
        timer.TIM_Prescaler = 84 -1;
        timer.TIM_Period = (TIM5_RESOLUTION) - 1;	// 125 usec timer (8000 Hz)
        timer.TIM_ClockDivision = TIM_CKD_DIV1;
        timer.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_TimeBaseInit(TIM5, &timer);

//	TIM_ARRPreloadConfig(TIM5, ENABLE);
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM5, ENABLE);
	NVIC_EnableIRQ(TIM5_IRQn);
}


void tim6_init() {
       // TIM6 is for DAC1: 8000 Hz 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
        TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
        TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
        TIM_TimeBaseStructure.TIM_Prescaler = 84 -1;
        TIM_TimeBaseStructure.TIM_Period = 125 - 1;
        TIM_TimeBaseStructure.TIM_ClockDivision = 0;
        TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);
        TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);
        TIM_Cmd(TIM6, ENABLE);
}


void RTC_Alarm_IRQHandler(void)
{
        __disable_irq();

	if(RTC_GetITStatus(RTC_IT_ALRA) != RESET) {
		RTC_ClearITPendingBit(RTC_IT_ALRA);
		EXTI_ClearITPendingBit(EXTI_Line17);
		PostMessageIRQ(RTC_ALARM, 1, 0, 0);
	}

        __enable_irq();
}



void General_Exception_Handler_main(unsigned int * args)
{

	_usart_tx_mode = USART_TX_DIRECT;

        _print("\r\n");
        _print("============================= G E N E R A L   E X C E P T I O N   T Y P E: %d =============================\r\n", exception_code);
        _print("R0: 0x%08X, R1: 0x%08X, R2: 0x%08X, R3: 0x%08X, R12: 0x%08X, R14/LR: 0x%08X, R15/PC: 0x%08X, xPSR: 0x%08X\r\n", args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7]);
	_print("BFAR = 0x%08X, CFSR = 0x%08X, HFSR = 0x%08X, DFSR = 0x%08X, AFSR = 0x%08X, SCB_SHCSR = 0x%08X, TEXT = %p\r\n", 
		(*((volatile unsigned long *)(0xE000ED38))),
		(*((volatile unsigned long *)(0xE000ED28))),
		(*((volatile unsigned long *)(0xE000ED2C))),
		(*((volatile unsigned long *)(0xE000ED30))),
		(*((volatile unsigned long *)(0xE000ED3C))),
		SCB->SHCSR,
		application_text_addr
		);

	if(args[6] >= FLASH_SECTOR2ADDR[TFS_FIRST_SECTOR] && args[6] < FLASH_SECTOR2ADDR[TFS_LAST_SECTOR+1]) {
		_print("Exception inside applciation at: offset 0x%08X called from: offset 0x%08X\r\nApplication execution terminated!\r\n\r\n", args[6] - application_text_addr, args[5] - application_text_addr);
		application_process_msg_addr = NULL;
		PostMessage(EXCEPTION_APPLICATION, 1, exception_code, args[6]);
	} else {
		PostMessage(EXCEPTION_SUPERVISOR, 1, exception_code, args[6]);
	}

        _print("\r\n");

	_usart_tx_mode = USART_TX_ISR;

	DelayLoop(100);
}



void EXTI15_10_IRQHandler(void) {

        __disable_irq();

        if(EXTI_GetITStatus(EXTI_Line10) != RESET) {
		ext_irq_call_proc(EXTI_Line10);
                EXTI_ClearITPendingBit(EXTI_Line10);
        }


        if(EXTI_GetITStatus(EXTI_Line11) != RESET) {
		ext_irq_call_proc(EXTI_Line11);
                EXTI_ClearITPendingBit(EXTI_Line11);
        }


        if(EXTI_GetITStatus(EXTI_Line12) != RESET) {
		ext_irq_call_proc(EXTI_Line12);
                EXTI_ClearITPendingBit(EXTI_Line12);
        }


        if(EXTI_GetITStatus(EXTI_Line13) != RESET) {
		ext_irq_call_proc(EXTI_Line13);
                EXTI_ClearITPendingBit(EXTI_Line13);
        }


        if(EXTI_GetITStatus(EXTI_Line14) != RESET) {
		ext_irq_call_proc(EXTI_Line14);
                EXTI_ClearITPendingBit(EXTI_Line14);
        }


        if(EXTI_GetITStatus(EXTI_Line15) != RESET) {
		ext_irq_call_proc(EXTI_Line15);
                EXTI_ClearITPendingBit(EXTI_Line15);
        }

        __enable_irq();
}


void EXTI9_5_IRQHandler(void) {

        __disable_irq();

	#ifdef KEY_RIGHT_GPIO_PORT
	if(EXTI_GetITStatus(KEY_RIGHT_IRQ_LINE) != RESET) {
		PostMessageIRQ(KEY_RIGHT_IRQ, 1, GPIO_ReadInputDataBit(KEY_RIGHT_GPIO_PORT, KEY_RIGHT_GPIO_PIN), 0);
		DelayLoop(50); // Debouncing
	}
	#endif


        if(EXTI_GetITStatus(EXTI_Line5) != RESET) {
		ext_irq_call_proc(EXTI_Line5);
                EXTI_ClearITPendingBit(EXTI_Line5);
        }


        if(EXTI_GetITStatus(EXTI_Line6) != RESET) {
		ext_irq_call_proc(EXTI_Line6);
                EXTI_ClearITPendingBit(EXTI_Line6);
        }


        if(EXTI_GetITStatus(EXTI_Line7) != RESET) {
		//_print("EXTI_Line7\r\n");
		ext_irq_call_proc(EXTI_Line7);
                EXTI_ClearITPendingBit(EXTI_Line7);
        }


        if(EXTI_GetITStatus(EXTI_Line8) != RESET) {
		ext_irq_call_proc(EXTI_Line8);
                EXTI_ClearITPendingBit(EXTI_Line8);
        }


        if(EXTI_GetITStatus(EXTI_Line9) != RESET) {
		ext_irq_call_proc(EXTI_Line9);
                EXTI_ClearITPendingBit(EXTI_Line9);
        }

        __enable_irq();
}


void EXTI0_IRQHandler(void) {

        __disable_irq();

	#ifdef LCD_SPI_INT_IRQ_LINE
        if(EXTI_GetITStatus(LCD_SPI_INT_IRQ_LINE) != RESET) {
                EXTI_ClearITPendingBit(LCD_SPI_INT_IRQ_LINE);

                int val=GPIO_ReadInputDataBit(LCD_SPI_INT_GPIO_PORT, LCD_SPI_INT_GPIO_PIN);

                //print("EXTI0_IRQHandler: val = %d\r\n", val);

                if(val) {
                        // Pin released to VDD
                        //PostMessageIRQ(LCD_INT_RELEASED, 1, 0, 0);
                } else {
                        // Pin is grounded
                        //PostMessageIRQ(LCD_INT, 1, 0, 0);
                        lcd_irq_requested = 1;
                }

        }
	#else
        if(EXTI_GetITStatus(EXTI_Line0) != RESET) {
		ext_irq_call_proc(EXTI_Line0);
                EXTI_ClearITPendingBit(EXTI_Line0);
        }
	#endif

        __enable_irq();
}





void EXTI1_IRQHandler(void) {
	__disable_irq();

	#ifdef EXTI1_GPIO_PORT
	if(EXTI_GetITStatus(EXTI_Line1) != RESET) {
		PostMessageIRQ(EXTI1_IRQ, 1, GPIO_ReadInputDataBit(EXTI1_GPIO_PORT, EXTI1_GPIO_PIN), 0);
	}
	#endif

	if(EXTI_GetITStatus(EXTI_Line1) != RESET) {
		ext_irq_call_proc(EXTI_Line1);
		EXTI_ClearITPendingBit(EXTI_Line1);
	}


	__enable_irq();
}

void EXTI2_IRQHandler(void) {
	__disable_irq();
	
	#ifdef KEY_UP_GPIO_PORT	
	if(EXTI_GetITStatus(KEY_UP_IRQ_LINE) != RESET) {
		PostMessageIRQ(KEY_UP_IRQ, 1, GPIO_ReadInputDataBit(KEY_UP_GPIO_PORT, KEY_UP_GPIO_PIN), 0);
		DelayLoop(50); // Debouncing
		EXTI_ClearITPendingBit(KEY_UP_IRQ_LINE);
	}
	#endif

	if(EXTI_GetITStatus(EXTI_Line2) != RESET) {
		ext_irq_call_proc(EXTI_Line2);
		EXTI_ClearITPendingBit(EXTI_Line2);
	}

	__enable_irq();
}

void EXTI3_IRQHandler(void) {
	__disable_irq();
	
	#ifdef KEY_DOWN_GPIO_PORT
	if(EXTI_GetITStatus(KEY_DOWN_IRQ_LINE) != RESET) {
		PostMessageIRQ(KEY_DOWN_IRQ, 1, GPIO_ReadInputDataBit(KEY_DOWN_GPIO_PORT, KEY_DOWN_GPIO_PIN), 0);
		DelayLoop(50); // Debouncing
		EXTI_ClearITPendingBit(KEY_DOWN_IRQ_LINE);
	}
	#endif

	if(EXTI_GetITStatus(EXTI_Line3) != RESET) {
		ext_irq_call_proc(EXTI_Line3);
		EXTI_ClearITPendingBit(EXTI_Line3);
	}

	__enable_irq();
}

void EXTI4_IRQHandler(void) {
	__disable_irq();

	#ifdef KEY_LEFT_GPIO_PORT
	if(EXTI_GetITStatus(KEY_LEFT_IRQ_LINE) != RESET) {
		PostMessageIRQ(KEY_LEFT_IRQ, 1, GPIO_ReadInputDataBit(KEY_LEFT_GPIO_PORT, KEY_LEFT_GPIO_PIN), 0);
		DelayLoop(50); // Debouncing
		EXTI_ClearITPendingBit(KEY_LEFT_IRQ_LINE);
	}
	#endif

	if(EXTI_GetITStatus(EXTI_Line4) != RESET) {
		ext_irq_call_proc(EXTI_Line4);
		EXTI_ClearITPendingBit(EXTI_Line4);
	}

	__enable_irq();
}


