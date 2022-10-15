/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "svc.h"
#include "msg.h"
#include "tfs.h"
#include "oled.h"
#include "softtimer.h"
#include "config.h"
#include "work.h"
#include "modbus_common.h"
#include "modbus_ext1.h"
#include "modbus_ext2.h"
#include "dali_common.h"
#include "dali1.h"
#include "dali2.h"
#include "vault.h"
#include "audio.h"
#include "logger.h"
#include "wnd.h"
#include "adc.h"
#include "ext_gpio.h"
#include "crc16.h"
#include "svglib.h"
#include "lcd-ft800.h"
#include "ext_irq.h"
#include "ext_spi.h"
#include "ext_pwm.h"
#include "listener.h"


#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define ABS(X) ((X) >= 0 ? (X) : -(X))

extern TFS_HEADER* application_tfs;
extern void (*application_process_msg_addr)(MSG*);
extern uint32_t application_text_addr;
extern uint8_t* application_data_addr;
extern uint32_t application_memory[];
extern WND* SCREEN;
extern NSVGfromEdgesRasterizer* SVGRAST;
extern uint8_t event_logging;

extern WND* show_calibrate_touch(BOOL test);
extern WND* show_input_text(INPUT_TEXT_PARAM* p, char* text);
extern WND* show_input_num(INPUT_TEXT_PARAM* p);
extern WND* show_input_num_format(INPUT_TEXT_PARAM *param, char* format);
extern WND* show_select_list(INPUT_TEXT_PARAM *param, char** list, unsigned int list_len, unsigned int selected_index);
extern WND* show_alert(char* text);
extern WND* show_set_time();
extern WND* show_adc_setup();


extern void* input_ports[];
extern unsigned int input_pins[];

int exec(TFS_HEADER *tfs);

void SVC_Handler_main(unsigned int * svc_args)
{
        unsigned int svc_number = ((char *)svc_args[6])[-2];
	float *s0 = (float*)(svc_args + 26);
        /*    * Stack contains:
                * r0, r1, r2, r3, r12, r14, the return address and xPSR
                * First argument (r0) is svc_args[0]
        */


	//if(event_logging)
	//	print("SVC_Handler_main() SVC request number: %d, arg0: %p, arg1: %p\r\n", svc_number, svc_args[0], svc_args[1]);

	switch(svc_number) {

		case SVC_STOP:
			stop_application((void*)application_text_addr);
			svc_args[0] = 0x0;
			break;

		case SVC_DEBUG_PRINT:
			print("[%p] %s", svc_args[6], svc_args[0]);
			svc_args[0] = 0x0;
			break;

		case SVC_GET_TEXT:
			svc_args[0] = application_text_addr;
			break;

		case SVC_GET_DATA:
			svc_args[0] = (int)application_data_addr;
			break;

		case SVC_POST_MESSAGE:
			svc_args[0] = PostMessage(svc_args[0], svc_args[1], svc_args[2], svc_args[3]);
			break;

		case SVC_GET_OS_VERSION:
			svc_args[0] = BUILD_NUMBER;
			break;

		case SVC_EXEC: {
			TFS_HEADER *tfs;
			tfs = tfs_find((char*)svc_args[0]);
			if(tfs) {
				PostMessage(SVC_EXEC_PENDING, 0, (int)tfs, 0);
				svc_args[0] = 0; // OK 
			} else {
				svc_args[0] = -1; // error: file not found
			}
		} break;

		case SVC_TFS_APLAY: {
			TFS_HEADER *tfs;
			tfs = tfs_find((char*)svc_args[0]);
			if(tfs) {
				play_start((char*)tfs + sizeof(TFS_HEADER), tfs->size);
				svc_args[0] = 0; // OK 
			} else {
				svc_args[0] = -1; // error: file not found
			}
		} break;


		case SVC_TFS_CREATE:
			svc_args[0] = (int)tfs_create_file((char *) svc_args[0], (int) svc_args[1], (int) svc_args[2]);
			break;

		case SVC_TFS_WRITE:
			svc_args[0] = (int)tfs_write_block((TFS_HEADER*) svc_args[0], (char*) svc_args[1], svc_args[2], svc_args[3]);
			break;

		case SVC_TFS_CLOSE:
			svc_args[0] = (int)tfs_close((TFS_HEADER*) svc_args[0]);
			break;

		case SVC_TFS_FIND:
			svc_args[0] = (int)tfs_find((char*)svc_args[0]); 
			break;

		case SVC_TFS_FIND_NEXT:
			svc_args[0] = (int)tfs_find_next((TFS_HEADER*) svc_args[0]);
			break;

		case SVC_TFS_GET_FREE:
			svc_args[0] = (int) tfs_get_free_space();
			break;

		case SVC_TFS_GET_BEGIN:
			svc_args[0] = (int) tfs_get_begin();
			break;

		case SVC_TFS_FORMAT:
			PostMessage(SVC_FORMAT_PENDING, 1, 0, 0);
			svc_args[0] = 0; 
			break;

		#ifdef OLED_DATA_GPIO_PORT 
		case SVC_OLED_PRINT:
			OLED_WEG010032_PRINT((uint16_t) svc_args[0], (char *)svc_args[1], (int16_t) svc_args[2], (int16_t) svc_args[3]);
			svc_args[0] = 0x0;
			break;


		case SVC_OLED_BLIT:
			OLED_WEG010032_BLIT((uint16_t) svc_args[0], (char *)svc_args[1], (int16_t) svc_args[2], (int16_t) svc_args[3]);
			svc_args[0] = 0x0;
			break;

		case SVC_OLED_CLEAR:
			OLED_WEG010032_CLEAR();
			svc_args[0] = 0x0;
			break;
		#endif

		#ifdef GPIO_CHANNELS
		case SVC_READ_IN:
			if(svc_args[0] < GPIO_CHANNELS) 
				svc_args[0] =  GPIO_ReadInputDataBit(ext_gpios[svc_args[0]].port,  ext_gpios[svc_args[0]].pin); 
			else
				svc_args[0] = -1;	
			break;
		#endif

		//case SVC_READ_DC_PWM:
		case SVC_GET_PWM: {
			float duty = ext_pwm_get(svc_args[0]);
			svc_args[0] = *(int*)(&duty);
			
		}; break;

		case SVC_READ_AC_PWM: {
			float duty = ext_pwm_get(svc_args[0] + 4);
			svc_args[0] = *(int*)(&duty);
			
		}; break;

		//case SVC_SET_DC_PWM: 
		case SVC_SET_PWM: {
			float duty = *s0;
			svc_args[0] = ext_pwm_set(svc_args[0], duty);
		}; break;

		case SVC_SET_AC_PWM: {
			float duty = *s0;
			svc_args[0] = ext_pwm_set(svc_args[0] + 4, duty);
		}; break;


		//case SVC_SET_DC_CLOCK: 
		case SVC_SET_PWM_CLOCK: {
			svc_args[0] = ext_pwm_set_clock(svc_args[0], svc_args[1]);
			break;
		}

		case SVC_SET_AC_CLOCK: {
			svc_args[0] = ext_pwm_set_clock(svc_args[0] + 4, svc_args[1]);
			break;
		}


		case SVC_SOFTTIMER_RUN:
			svc_args[0] = softtimer_run_timer(svc_args[0], svc_args[1], svc_args[2], svc_args[3]);
			break;

		case SVC_SOFTTIMER_STOP:
			svc_args[0] = softtimer_stop_timer(svc_args[0]);
			break;

		case SVC_READ_CONFIG: {
			switch(svc_args[0]) {
				case CONFIG_EXT1_MODBUS_ADDR:
					*(int*)svc_args[1] = config_active.ext1_port_modbus_addr;
					svc_args[0] = 0;
					break;
				case CONFIG_EXT1_MODBUS_MODE:
					*(int*)svc_args[1] = config_active.ext1_port_modbus_mode;
					svc_args[0] = 0;
					break;
				case CONFIG_EXT1_BAUD_RATE:
					*(int*)svc_args[1] = config_active.ext1_port_baud_rate;
					svc_args[0] = 0;
					break;
				case CONFIG_EXT1_PARITY:
					*(int*)svc_args[1] = config_active.ext1_port_parity;
					svc_args[0] = 0;
					break;
				case CONFIG_EXT1_STOP_BITS:
					*(int*)svc_args[1] = config_active.ext1_port_stop_bits;
					svc_args[0] = 0;
					break;
				case CONFIG_EXT2_MODBUS_ADDR:
					*(int*)svc_args[1] = config_active.ext2_port_modbus_addr;
					svc_args[0] = 0;
					break;
				case CONFIG_EXT2_MODBUS_MODE:
					*(int*)svc_args[1] = config_active.ext2_port_modbus_mode;
					svc_args[0] = 0;
					break;
				case CONFIG_EXT2_BAUD_RATE:
					*(int*)svc_args[1] = config_active.ext2_port_baud_rate;
					svc_args[0] = 0;
					break;
				case CONFIG_EXT2_PARITY:
					*(int*)svc_args[1] = config_active.ext2_port_parity;
					svc_args[0] = 0;
					break;
				case CONFIG_EXT2_STOP_BITS:
					*(int*)svc_args[1] = config_active.ext2_port_stop_bits;
					svc_args[0] = 0;
					break;
				case CONFIG_DEFAULT_APP:
					memcpy((char*)svc_args[1], config_active.default_application_file_name, MIN(256, strlen((char*)config_active.default_application_file_name)+1)); 
					svc_args[0] = 0;
					break;
				case CONFIG_LOGGER_ENABLED:
					*(int*)svc_args[1] = config_active.logger_enabled;
                                        svc_args[0] = 0;
                                        break;
				case CONFIG_LCD_ENABLED:
					*(int*)svc_args[1] = config_active.lcd.enabled;
                                        svc_args[0] = 0;
                                        break;
				case CONFIG_ADC_ENABLED:
					*(int*)svc_args[1] = config_active.adc.enabled;
                                        svc_args[0] = 0;
                                        break;
				case CONFIG_USER_DATA:
					*(int*)svc_args[1] = (int)config_active.user_data;
                                        svc_args[0] = 0;
                                        break;
				case CONFIG_DEBUG_ENABLED:
					*(int*)svc_args[1] = event_logging;
					svc_args[0] = 0;
					break;
				default:
					print("SVC_Handler_main() Config variable %d is not suppored!\r\n", svc_args[0]);
					svc_args[0] = -1;
					break;
			}
			break;
		}



		case SVC_SET_CONFIG: {
			switch(svc_args[0]) {
			#ifdef USART_EXT1
				case CONFIG_EXT1_MODBUS_ADDR:
					config_active.ext1_port_modbus_addr = *(int*)svc_args[1];
					save_config();
					svc_args[0] = 0;
					break;
				case CONFIG_EXT1_MODBUS_MODE:
					config_active.ext1_port_modbus_mode = *(int*)svc_args[1];
					save_config();
					svc_args[0] = 0;
					break;
				case CONFIG_EXT1_BAUD_RATE:
					config_active.ext1_port_baud_rate = *(int*)svc_args[1];
					save_config();
					USART2_init();
					svc_args[0] = 0;
					break;
				case CONFIG_EXT1_PARITY:
					config_active.ext1_port_parity = *(int*)svc_args[1];
					save_config();
					USART2_init();
					svc_args[0] = 0;
					break;
				case CONFIG_EXT1_STOP_BITS:
					config_active.ext1_port_stop_bits = *(int*)svc_args[1];
					save_config();
					USART2_init();
					svc_args[0] = 0;
					break;
			#endif

			#ifdef USART_EXT2
				case CONFIG_EXT2_MODBUS_ADDR:
					config_active.ext2_port_modbus_addr = *(int*)svc_args[1];
					save_config();
					svc_args[0] = 0;
					break;
				case CONFIG_EXT2_MODBUS_MODE:
					config_active.ext2_port_modbus_mode = *(int*)svc_args[1];
					save_config();
					svc_args[0] = 0;
					break;
				case CONFIG_EXT2_BAUD_RATE:
					config_active.ext2_port_baud_rate = *(int*)svc_args[1];
					save_config();
					USART3_init();
					svc_args[0] = 0;
					break;
				case CONFIG_EXT2_PARITY:
					config_active.ext2_port_parity = *(int*)svc_args[1];
					save_config();
					USART3_init();
					svc_args[0] = 0;
					break;
				case CONFIG_EXT2_STOP_BITS:
					config_active.ext2_port_stop_bits = *(int*)svc_args[1];
					save_config();
					USART3_init();
					svc_args[0] = 0;
					break;
			#endif

				case CONFIG_DEFAULT_APP:
					memcpy(config_active.default_application_file_name, (char*)svc_args[1], MIN(256, strlen((char*)svc_args[1])+1)); 
					save_config();
					svc_args[0] = 0;
					break;
				case CONFIG_USER_DATA:
					//skip params, only save_config
                                        save_config();
                                        svc_args[0] = 0;
                                        break;
				default:
					print("SVC_Handler_main() Config variable %d is not suppored!\r\n", svc_args[0]);
					svc_args[0] = -1;
					break;
			}
			break;
		} break;

		case SVC_GET_ADC_COEFF: {
			int i = svc_args[0];
			if(i >= ADC_NUM_OF_CHANNELS) {
				svc_args[0] = -1;
			} else {
				*(float*)(svc_args[1]) = config_active.adc.coeff[i];
				svc_args[0] = 0;
			}
		} break;

		case SVC_GET_ADC_OFFSET: {
			int i = svc_args[0];
			if(i >= ADC_NUM_OF_CHANNELS) {
				svc_args[0] = -1;
			} else {
				*(float*)(svc_args[1]) = config_active.adc.offset[i];
				svc_args[0] = 0;
			}
		} break; 

		case SVC_SET_ADC_COEFF: {
			int i = svc_args[0];
			if(i >= ADC_NUM_OF_CHANNELS) {
				svc_args[0] = -1;
			} else {
				config_active.adc.coeff[i] = *s0;
				svc_args[0] = 0;
			}
		} break;

		case SVC_SET_ADC_OFFSET: {
			int i = svc_args[0];
			if(i >= ADC_NUM_OF_CHANNELS) {
				svc_args[0] = -1;
			} else {
				config_active.adc.offset[i] = *s0;
				svc_args[0] = 0;
			}
		} break;

		case SVC_READ_ADC: {
			int i = svc_args[0];
			if(i >= ADC_NUM_OF_CHANNELS) {
				svc_args[0] = -1;
			} else {
				*(float*)(svc_args[1]) = ADC_DATA.adc_buf[i];
				svc_args[0] = 0;
			}
		} break; 

		case SVC_READ_SENSOR_RMS: {
			int i = svc_args[0];

			if(ABS(i) >= ADC_NUM_OF_CHANNELS) {
				svc_args[0] = -1;
			} else {
				if(i < 0) { 
					i = ADC_NUM_OF_CHANNELS + i;
				}
				*(float*)(svc_args[1]) = ADC_DATA.adc_rms[i];
				svc_args[0] = 0;
			}
		} break; 

		case SVC_READ_SENSOR_AVG: {
			int i = svc_args[0];

			if(ABS(i) >= ADC_NUM_OF_CHANNELS) {
				svc_args[0] = -1;
			} else {
				if(i < 0) { 
					i = ADC_NUM_OF_CHANNELS + i;
				}
				*(float*)(svc_args[1]) = ADC_DATA.adc_avg[i];
				svc_args[0] = 0;
			}
		} break; 

		case SVC_GET_ALL_ADC_DATA: {
			svc_args[0] = (unsigned int) &ADC_DATA;
		} break;


		case SVC_MALLOC: {
			int i;
			for(i = 0; i < APPLICATION_MAX_MEM; i++) {
				if(application_memory[i] == 0)
					break;
			}

			if(i == APPLICATION_MAX_MEM) {
				print("SVC_Handler_main() Application max memory reached!\r\n");
				svc_args[0] = 0;
				break;
			}

			application_memory[i] = (uint32_t)malloc(svc_args[0]);
			if(!application_memory[i]) {
				print("SVC_Handler_main() failed to malloc(%d)\r\n", svc_args[0]);
				svc_args[0] = 0;
			} else {
				svc_args[0] = application_memory[i];
			}

		} break;

		
		case SVC_FREE: {
			int i;
			for(i = 0; i  < APPLICATION_MAX_MEM; i++) {
				if(application_memory[i] == svc_args[0])
					break;
			}
			if(i < APPLICATION_MAX_MEM) {
				free((void*)application_memory[i]);
				application_memory[i] = 0;
				svc_args[0] = 0;
			} else {
				svc_args[0] = -1;
			}
		} break;


		case SVC_DC_PWM_XFER: {

			if(svc_args[0] == DC_PORT_2 || svc_args[0] == DC_PORT_4) {
				print("SVC_Handler_main() Operation is not supported for this DC channel (only for DC_PORT_1 and DC_PORT_3)\r\n");
				svc_args[0] = -1;
				break;
			}

			//print("SVC_Handler_main() initializing DMA transfer, size = %d\r\n", svc_args[2]);

			if(svc_args[0] == DC_PORT_1) {
				DMA_DeInit(DMA1_Stream0);
			} else if(svc_args[0] == DC_PORT_3) {
				DMA_DeInit(DMA1_Stream7);
			}

			DMA_InitTypeDef DMA_InitStructure;
			DMA_InitStructure.DMA_BufferSize = svc_args[2] ;
			DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable ;
			//DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable ;
			DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull ;
			DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single ;
			DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
			DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
			if(svc_args[3] == 0)
				DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
			else
				DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;

			if(svc_args[0] == DC_PORT_1) {
				DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t) (&(TIM4->CCR1)); 
			} else if(svc_args[0] == DC_PORT_3) {
				DMA_InitStructure.DMA_PeripheralBaseAddr =(uint32_t) (&(TIM4->CCR3));
			}
				
			DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
			DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
			DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
			DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
			DMA_InitStructure.DMA_Channel = DMA_Channel_2 ;
			DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral ;
			DMA_InitStructure.DMA_Memory0BaseAddr =(uint32_t)svc_args[1] ;

			if(svc_args[0] == DC_PORT_1) {
				DMA_Init(DMA1_Stream0, &DMA_InitStructure);
				DMA_ITConfig(DMA1_Stream0, DMA_IT_TC , ENABLE);
			} else if(svc_args[0] == DC_PORT_3) {
				DMA_Init(DMA1_Stream7, &DMA_InitStructure);
				DMA_ITConfig(DMA1_Stream7, DMA_IT_TC , ENABLE);
			}


			NVIC_InitTypeDef NVIC_InitStructure;
			if(svc_args[0] == DC_PORT_1)
				NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream0_IRQn;
			else if(svc_args[0] == DC_PORT_3)
				NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream7_IRQn;
			
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

			NVIC_Init(&NVIC_InitStructure);

			if(svc_args[0] == DC_PORT_1) {
        			DMA_ClearFlag(DMA1_Stream0,DMA_FLAG_TCIF0);
				DMA_Cmd(DMA1_Stream0, ENABLE);
			} else if(svc_args[0] == DC_PORT_3) {
        			DMA_ClearFlag(DMA1_Stream7,DMA_FLAG_TCIF7);
				DMA_Cmd(DMA1_Stream7, ENABLE);
			}

			TIM4->CNT = 0; // reset counter to avoid spurious effect	

			//print("SVC_Handler_main() PWM DMA xfer initialized\r\n");

		} break;


		#ifdef USART_EXT1
		case SVC_MODBUS1_ENQUEUE_REQUEST: {
			svc_args[0] = modbus_enqueue_request_ext1((MODBUS_REQUEST*) svc_args[0]);
		} break;

		case SVC_MODBUS1_REGISTER_RESPONDER: {
			if(((MODBUS_RESPONSE*) svc_args[0])->registered_func < FUNC_READ_ID) {
				if(((MODBUS_RESPONSE*) svc_args[0])->registered_reg_start < 1000 || ((MODBUS_RESPONSE*) svc_args[0])->registered_reg_end < 1000) {
;
					print("SVC_Handler_main() Attempt to register Modbus1 responder for reg range below 1000 is not allowed!\r\n");
					svc_args[0] = -1;
					break;
				}
			}
			svc_args[0] = modbus_register_responder_ext1((MODBUS_RESPONSE*) svc_args[0]);
		} break;

		case SVC_MODBUS1_UNREGISTER_RESPONDER: {
			svc_args[0] = modbus_unregister_responder_ext1((MODBUS_RESPONSE*) svc_args[0]);
		} break;

		case SVC_MODBUS1_SUBMIT_RESPONSE: {
			svc_args[0] = modbus_submit_response_ext1((MODBUS_RESPONSE*) svc_args[0]);
		} break;

		#endif


		#ifdef DALI1_MODE
		case SVC_DALI1_ENQUEUE_REQUEST: {
			svc_args[0] = dali1_enqueue_request((DALI_REQUEST*) svc_args[0]);
		} break;

		case SVC_DALI1_REGISTER_RESPONDER: {
			svc_args[0] = dali1_register_responder((DALI_RESPONSE*) svc_args[0]);
		} break;

		case SVC_DALI1_UNREGISTER_RESPONDER: {
			svc_args[0] = dali1_unregister_responder((DALI_RESPONSE*) svc_args[0]);
		} break;

		case SVC_DALI1_SUBMIT_RESPONSE: {
			svc_args[0] = dali1_submit_response((DALI_RESPONSE*) svc_args[0]);
		} break;

		#endif


		#ifdef DALI2_MODE
		case SVC_DALI2_ENQUEUE_REQUEST: {
			svc_args[0] = dali2_enqueue_request((DALI_REQUEST*) svc_args[0]);
		} break;

		case SVC_DALI2_REGISTER_RESPONDER: {
			svc_args[0] = dali2_register_responder((DALI_RESPONSE*) svc_args[0]);
		} break;

		case SVC_DALI2_UNREGISTER_RESPONDER: {
			svc_args[0] = dali2_unregister_responder((DALI_RESPONSE*) svc_args[0]);
		} break;

		case SVC_DALI2_SUBMIT_RESPONSE: {
			svc_args[0] = dali2_submit_response((DALI_RESPONSE*) svc_args[0]);
		} break;

		#endif

		
		#ifdef USART_EXT2
		case SVC_MODBUS2_ENQUEUE_REQUEST: {
			svc_args[0] = modbus_enqueue_request_ext2((MODBUS_REQUEST*) svc_args[0]);
		} break;

		case SVC_MODBUS2_REGISTER_RESPONDER: {
			if(((MODBUS_RESPONSE*) svc_args[0])->registered_func < FUNC_READ_ID) {
				if(((MODBUS_RESPONSE*) svc_args[0])->registered_reg_start < 1000 || ((MODBUS_RESPONSE*) svc_args[0])->registered_reg_end < 1000) {
;
					print("SVC_Handler_main() Attempt to register Modbus2 responder for reg range below 1000 is not allowed!\r\n");
					svc_args[0] = -1;
					break;
				}
			}
			svc_args[0] = modbus_register_responder_ext2((MODBUS_RESPONSE*) svc_args[0]);
		} break;

		case SVC_MODBUS2_UNREGISTER_RESPONDER: {
			svc_args[0] = modbus_unregister_responder_ext2((MODBUS_RESPONSE*) svc_args[0]);
		} break;

		case SVC_MODBUS2_SUBMIT_RESPONSE: {
			svc_args[0] = modbus_submit_response_ext2((MODBUS_RESPONSE*) svc_args[0]);
		} break;

		#endif


		case SVC_VAULT_GET: {
			svc_args[0] = vault_get_var(application_tfs->name, (char*)svc_args[0], (void **)svc_args[1]);
		} break;


		case SVC_VAULT_SET: {
			svc_args[0] = vault_set_var(application_tfs->name, (char*)svc_args[0], (void*)svc_args[1], svc_args[2], svc_args[3]);
		} break;


		case SVC_VAULT_DEL: {
			svc_args[0] = vault_del_var(application_tfs->name, (char*)svc_args[0]);
		} break;


		case SVC_VAULT_ENUM: {
			svc_args[0] = vault_enum(svc_args[0], (char *)svc_args[1], (char *)svc_args[2], (int*) svc_args[3]);
		} break;

		case SVC_GET_TIME: {
			RTC_TimeTypeDef *t = (RTC_TimeTypeDef *) svc_args[0];
                        RTC_GetTime(RTC_Format_BIN, t);
			svc_args[0] = 0;
		} break;

		case SVC_GET_DATE: {
			RTC_DateTypeDef *d = (RTC_DateTypeDef *) svc_args[0];
                        RTC_GetDate(RTC_Format_BIN, d);
			svc_args[0] = 0;
		} break;

		case SVC_SET_TIME: {
			RTC_TimeTypeDef *t = (RTC_TimeTypeDef *)&svc_args[0] ;
                        PWR_BackupAccessCmd(ENABLE);
                        RTC_WriteProtectionCmd(DISABLE);
                        svc_args[0] = RTC_SetTime(RTC_Format_BIN, t);
                        RTC_WriteProtectionCmd(ENABLE);
                        PWR_BackupAccessCmd(DISABLE);
		} break;

		case SVC_SET_DATE: {
			RTC_DateTypeDef *d = (RTC_DateTypeDef *)&svc_args[0] ;
                        PWR_BackupAccessCmd(ENABLE);
                        RTC_WriteProtectionCmd(DISABLE);
                        svc_args[0] = RTC_SetDate(RTC_Format_BIN, d);
                        RTC_WriteProtectionCmd(ENABLE);
                        PWR_BackupAccessCmd(DISABLE);
		} break;

		case SVC_SET_ROOT_WINDOW: {
			SCREEN = (WND*) svc_args[0];
			svc_args[0] = 0;
                } break;

		case SVC_GET_ROOT_WINDOW: {
                        svc_args[0] = (unsigned int) SCREEN;
                } break;

                case SVC_WND_CREATE: {
                        svc_args[0] = (unsigned int) wnd_create((WNDCLASS*) svc_args[0]);
                } break;

                case SVC_WND_DESTROY: {
                        wnd_destroy((WND*) svc_args[0]);
                        svc_args[0] = 0;
                } break;

                case SVC_WND_GET_CLASS: {
                        svc_args[0] = (unsigned int) wnd_get_class((const char*) svc_args[0]);
                } break;

		case SVC_WND_REGISTER_CLASS: {
			svc_args[0] = (unsigned int) wnd_register_class((WNDCLASS*) svc_args[0]);
		} break;

		case SVC_GET_FONT: {
			switch(svc_args[0]) {
			case 0:
			default:
				svc_args[0] = (unsigned int) font_medium;
				break;
			case 1:
				svc_args[0] = (unsigned int) font_small;
                                break;
			case 2:
				svc_args[0] = (unsigned int) font_large;
                                break;
			};
		} break;

		case SVC_SHOW_CALIBRATE_TOUCH: {
			svc_args[0] = (uint32_t) show_calibrate_touch((BOOL)svc_args[0]);
                } break;

                case SVC_SHOW_INPUT_TEXT: {
                        svc_args[0] = (uint32_t) show_input_text((INPUT_TEXT_PARAM*) svc_args[0], (char*) svc_args[1]);
                } break;

		case SVC_SHOW_INPUT_NUM: {
			svc_args[0] = (uint32_t) show_input_num((INPUT_TEXT_PARAM*) svc_args[0]);
		} break;

		case SVC_SHOW_INPUT_NUM_FORMAT: {
			svc_args[0] = (uint32_t) show_input_num_format((INPUT_TEXT_PARAM*) svc_args[0], (char*) svc_args[1]);
                } break;

		case SVC_SHOW_SELECT_LIST: {
			svc_args[0] = (uint32_t) show_select_list((INPUT_TEXT_PARAM*) svc_args[0], (char**) svc_args[1], svc_args[2], svc_args[3]);
		} break;

		case SVC_SHOW_ALERT: {
			svc_args[0] = (uint32_t) show_alert((char*) svc_args[0]);
		} break;

		case SVC_SHOW_SET_TIME: {
			svc_args[0] = (uint32_t) show_set_time();
		} break;

		case SVC_SHOW_ADC_SETUP: {
			svc_args[0] = (uint32_t) show_adc_setup(svc_args[0]);
		} break;

		case SVC_LOGGER_WRITE_DATA: {
			uint8_t *data = (uint8_t *)svc_args[0];
			unsigned int data_len = svc_args[1];
			svc_args[0] = logger_write_data ( data, data_len);
		} break;

		case SVC_LOGGER_ERASE_CURRENT_SECTOR: {
			svc_args[0] = logger_erase_current_sector();
		} break;

		case SVC_LOGGER_GET_PREVIOUS_ITEM: {
			svc_args[0] = (unsigned int) logger_get_previous_item((struct LOGGER_ITEM* ) svc_args[0]);
		} break;

		case SVC_LOGGER_GET_CURRENT_ITEM: {
			svc_args[0] = (unsigned int) current_logger_item;
		} break;

		case SVC_GPIO_INIT_PULLUP: {
			gpio_init(svc_args[0], GPIO_SPEED, (void*)(svc_args[1]), svc_args[2], svc_args[3], GPIO_OType_PP, GPIO_PuPd_UP);
			svc_args[0] = 0;
		} break;
	
		case SVC_GPIO_INIT_NOPULL: {
			gpio_init(svc_args[0], GPIO_SPEED, (void*)(svc_args[1]), svc_args[2], svc_args[3], GPIO_OType_PP, GPIO_PuPd_NOPULL);
			svc_args[0] = 0;
		} break;

		case SVC_GPIO_IRQ_FALL: {
			gpio_irq(svc_args[0], svc_args[1], svc_args[2], svc_args[3], EXTI_Trigger_Falling);
			svc_args[0] = 0;
		} break;

		case SVC_GPIO_IRQ_RISE: {
			gpio_irq(svc_args[0], svc_args[1], svc_args[2], svc_args[3], EXTI_Trigger_Rising);
			svc_args[0] = 0;
		} break;

		case SVC_GPIO_IRQ_RISEFALL: {
			gpio_irq(svc_args[0], svc_args[1], svc_args[2], svc_args[3], EXTI_Trigger_Rising_Falling);
			svc_args[0] = 0;
		} break;

		case SVC_NSVG_RASTERIZER: {
			svc_args[0] = (unsigned int) SVGRAST;
		} break;
		
		case SVC_NSVG_DRAW: {
			gc_draw_nsvgimage( (Context*) svc_args[0], (NSVGimage*) svc_args[1], svc_args[2], svc_args[3]);
			svc_args[0] = 0;
		} break;

		case SVC_EXT_IRQ_SET: {
			svc_args[0] = ext_irq_set(svc_args[0], (EXTIRQ_PROC) svc_args[1], application_process_msg_addr);
		} break;

		case SVC_EXT_IRQ_GET: {
                        svc_args[0] = (unsigned int) ext_irq_get(svc_args[0]);
                } break;

		case SVC_EXT_IRQ_CLEAR: {
                        svc_args[0] = ext_irq_clear(svc_args[0], application_process_msg_addr);
                } break;

		case SVC_LISTENER_SET: {
			listener_set(svc_args[0], (LISTENER_PROC)  svc_args[1]);
			svc_args[0] = 0;
		} break;

		case SVC_LISTENER_REMOVE: {
                        listener_remove(svc_args[0], (LISTENER_PROC)  svc_args[1]);
                        svc_args[0] = 0;
                } break;


		#ifdef EXT_SPI
		case SVC_EXT_SPI_REQUEST: {
                        svc_args[0] = ext_spi_dma_request((char*) svc_args[0], (char*) svc_args[1], svc_args[2]);
                } break;

		case SVC_EXT_SPI_STOP: {
			ext_spi_stop();
                        svc_args[0] = 0;
                } break;
		#endif


		default:
			print("SVC_Handler_main() Called SVC %d is not suppored!\r\n", svc_number);
			break;
	}
}

void DMA1_Stream0_IRQHandler(void) {
        __disable_irq();
        int tc = DMA_GetITStatus(DMA1_Stream0, DMA_IT_TCIF0);
	//_print("\r\nDMA1_Stream0_IRQHandler() tc = %d\r\n", tc);
        if(tc) {
		TIM4->CCR1 = 0x0;
                DMA_ClearITPendingBit(DMA1_Stream0, DMA_IT_TCIF0);
                PostMessageIRQ(DC_PWM_1_DMA_TC, 1, 0, 0);
        }
        __enable_irq();
}

void DMA1_Stream7_IRQHandler(void) {
        __disable_irq();
        int tc = DMA_GetITStatus(DMA1_Stream7, DMA_IT_TCIF7);
	//_print("\r\nDMA1_Stream7_IRQHandler() tc = %d\r\n", tc);
        if(tc) {
		TIM4->CCR3 = 0x0;
                DMA_ClearITPendingBit(DMA1_Stream7, DMA_IT_TCIF7);
                PostMessageIRQ(DC_PWM_3_DMA_TC, 1, 0, 0);
        }
        __enable_irq();
}



