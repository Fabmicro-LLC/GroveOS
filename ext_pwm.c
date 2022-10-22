/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2016-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include "hardware.h"
#include "ext_pwm.h"
#include "utils.h"
#include <stm32f4xx_tim.h>

int ext_pwm_set_clock(int port, int clock)
{

	if(port >=0 && port < 4) {
		TIM_TimeBaseInitTypeDef    timer;
		TIM_TimeBaseStructInit(&timer);
		if(clock >= 500) {
			timer.TIM_Prescaler = 4 - 1;
			timer.TIM_Period = 21000000 /  clock - 1;
		} else {
			timer.TIM_Prescaler = 1680 - 1;
			timer.TIM_Period = 50000 /  clock - 1;
		}
		timer.TIM_ClockDivision = TIM_CKD_DIV1;
		timer.TIM_RepetitionCounter = 0;
		timer.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseInit(TIM4, &timer);
		TIM_ITConfig(TIM4, TIM_DIER_CC1DE | TIM_DIER_CC2DE | TIM_DIER_CC3DE | TIM_DIER_CC4DE, ENABLE);
		return clock;

	} else if(port > 3 && port < 8) {

		TIM_TimeBaseInitTypeDef    timer;
		TIM_TimeBaseStructInit(&timer);
		if(clock > 500) {
			timer.TIM_Prescaler = 4 - 1;
			timer.TIM_Period = 21000000 /  clock - 1;
		} else {
			timer.TIM_Prescaler = 1680 - 1;
			timer.TIM_Period = 50000 /  clock - 1;
		}
		timer.TIM_ClockDivision = TIM_CKD_DIV1;
		timer.TIM_RepetitionCounter = 0;
		timer.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseInit(TIM3, &timer);
		TIM_ITConfig(TIM3, TIM_DIER_CC1DE | TIM_DIER_CC2DE | TIM_DIER_CC3DE | TIM_DIER_CC4DE, ENABLE);
		return clock;

	}

	return -1;
}



int ext_pwm_set(int port, float duty)
{
			TIM_OCInitTypeDef TIM_OCStruct;
			TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM1;
			TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
			TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_High;

			uint32_t period = 0;
			uint32_t pulse_length = 0;
			
			switch(port) {
				case PWM_PORT_1:
					period = TIM4->ARR;
					pulse_length = ((period + 1) * duty) / 100 - 1;
					TIM_OCStruct.TIM_Pulse = pulse_length;
					TIM_OC1Init(TIM4, &TIM_OCStruct);
					TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
					break;

				case PWM_PORT_2:
					period = TIM4->ARR;
					pulse_length = ((period + 1) * duty) / 100 - 1;
					TIM_OCStruct.TIM_Pulse = pulse_length;
					TIM_OC2Init(TIM4, &TIM_OCStruct);
					TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
					break;

				case PWM_PORT_3:
					period = TIM4->ARR;
					pulse_length = ((period + 1) * duty) / 100 - 1;
					TIM_OCStruct.TIM_Pulse = pulse_length;
					TIM_OC3Init(TIM4, &TIM_OCStruct);
					TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
					break;

				case PWM_PORT_4:
					period = TIM4->ARR;
					pulse_length = ((period + 1) * duty) / 100 - 1;
					TIM_OCStruct.TIM_Pulse = pulse_length;
					TIM_OC4Init(TIM4, &TIM_OCStruct);
					TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
					break;

				case PWM_PORT_5:
					period = TIM3->ARR;
					pulse_length = ((period + 1) * duty) / 100 - 1;
					TIM_OCStruct.TIM_Pulse = pulse_length;
					TIM_OC1Init(TIM3, &TIM_OCStruct);
					TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
					break;

				case PWM_PORT_6:
					period = TIM3->ARR;
					pulse_length = ((period + 1) * duty) / 100 - 1;
					TIM_OCStruct.TIM_Pulse = pulse_length;
					TIM_OC2Init(TIM3, &TIM_OCStruct);
					TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
					break;

				case PWM_PORT_7:
					period = TIM3->ARR;
					pulse_length = ((period + 1) * duty) / 100 - 1;
					TIM_OCStruct.TIM_Pulse = pulse_length;
					TIM_OC3Init(TIM3, &TIM_OCStruct);
					TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
					break;

				case PWM_PORT_8:
					period = TIM3->ARR;
					pulse_length = ((period + 1) * duty) / 100 - 1;
					TIM_OCStruct.TIM_Pulse = pulse_length;
					TIM_OC4Init(TIM3, &TIM_OCStruct);
					TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
					break;

				default:
					return -1;
			}

	return 0;
}


float ext_pwm_get(int port)
{

                        uint32_t period = 1;
                        uint32_t pulse_length;
                        float duty = -1.0;

                        switch(port) {
                                case PWM_PORT_1:
                                        pulse_length = TIM4->CCR1;
                                        period = TIM4->ARR;
                                        break;

                                case PWM_PORT_2:
                                        pulse_length = TIM4->CCR2;
                                        period = TIM4->ARR;
                                        break;

                                case PWM_PORT_3:
                                        pulse_length = TIM4->CCR3;
                                        period = TIM4->ARR;
                                        break;

                                case PWM_PORT_4:
                                        pulse_length = TIM4->CCR4;
                                        period = TIM4->ARR;
                                        break;

                                case PWM_PORT_5:
                                        pulse_length = TIM3->CCR1;
                                        period = TIM3->ARR;
                                        break;

                                case PWM_PORT_6:
                                        pulse_length = TIM3->CCR2;
                                        period = TIM3->ARR;
                                        break;

                                case PWM_PORT_7:
                                        pulse_length = TIM3->CCR3;
                                        period = TIM3->ARR;
                                        break;

                                case PWM_PORT_8:
                                        pulse_length = TIM3->CCR4;
                                        period = TIM3->ARR;
                                        break;

                                default:
                                        pulse_length = -1;

			}

                        duty = (100.0 * pulse_length + 1) / (period + 1);

                        return duty;
}

