/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2017-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#ifndef __HARDWARE_APS_H__
#define __HARDWARE_APS_H__

#define	OS_PRODUCT_NAME		"APS"
#define TIM5_RESOLUTION         125             // INFO timer is 8000 MHz (125usec)

#define	STM32F42X			1	// We are running on STM32F42x machine

#define LED_GPIO_PORT           GPIOD
#define LED_GPIO_PIN            GPIO_Pin_1
#define LED_PERIPH              RCC_AHB1Periph_GPIOD



// EXT1 Serial port configuration
#define	USART_EXT1			USART2
#define	USART_EXT1_BAUD			9600	
#define	USART_EXT1_GPIO_PORT		GPIOD
#define	USART_EXT1_GPIO_AF		GPIO_AF_USART2
#define	USART_EXT1_USART_PERIPH		RCC_APB1Periph_USART2
#define	USART_EXT1_GPIO_PERIPH		RCC_AHB1Periph_GPIOD
#define	USART_EXT1_GPIO_TX_PIN		GPIO_Pin_5
#define	USART_EXT1_GPIO_RX_PIN		GPIO_Pin_6
#define	USART_EXT1_GPIO_TX_SRC_PIN	GPIO_PinSource5
#define	USART_EXT1_GPIO_RX_SRC_PIN	GPIO_PinSource6
//RS485
#define	USART_EXT1_RS485_TX_ENL_PERIPH		RCC_AHB1Periph_GPIOD
#define	USART_EXT1_RS485_TX_ENL_GPIO_PORT	GPIOD
#define	USART_EXT1_RS485_TX_ENL_GPIO_PIN	GPIO_Pin_2

// EXT2 Serial port configuration
#define	USART_EXT2			USART3
#define	USART_EXT2_BAUD			9600
#define	USART_EXT2_GPIO_PORT		GPIOD
#define	USART_EXT2_GPIO_AF		GPIO_AF_USART3
#define	USART_EXT2_USART_PERIPH		RCC_APB1Periph_USART3
#define	USART_EXT2_GPIO_PERIPH		RCC_AHB1Periph_GPIOD
#define	USART_EXT2_GPIO_TX_PIN		GPIO_Pin_8
#define	USART_EXT2_GPIO_RX_PIN		GPIO_Pin_9
#define	USART_EXT2_GPIO_TX_SRC_PIN	GPIO_PinSource8
#define	USART_EXT2_GPIO_RX_SRC_PIN	GPIO_PinSource9
//RS485
#define	USART_EXT2_RS485_TX_ENL_PERIPH		RCC_AHB1Periph_GPIOC
#define	USART_EXT2_RS485_TX_ENL_GPIO_PORT	GPIOC
#define	USART_EXT2_RS485_TX_ENL_GPIO_PIN	GPIO_Pin_13


#define LCD_SPI_BAUDRATE_PRESCALER      	SPI_BaudRatePrescaler_4
#define LCD_GPIO_SPEED                          GPIO_Speed_100MHz
#define LCD_SPI_CS_GPIO_PERIPH                  RCC_AHB1Periph_GPIOA
#define LCD_SPI_CS_GPIO_PORT                    GPIOA
#define LCD_SPI_CS_GPIO_PIN                     GPIO_Pin_4
#define LCD_SPI_PD_GPIO_PERIPH                  RCC_AHB1Periph_GPIOD
#define LCD_SPI_PD_GPIO_PORT                    GPIOD
#define LCD_SPI_PD_GPIO_PIN                     GPIO_Pin_0
#define LCD_SPI_INT_GPIO_PERIPH                 RCC_AHB1Periph_GPIOE
#define LCD_SPI_INT_GPIO_PORT                   GPIOE
#define LCD_SPI_INT_GPIO_PIN                    GPIO_Pin_0
#define LCD_SPI_INT_IRQ_PORT_SRC                EXTI_PortSourceGPIOE
#define LCD_SPI_INT_IRQ_PIN_SRC                 EXTI_PinSource0
#define LCD_SPI_INT_IRQ_LINE                    EXTI_Line0
#define LCD_SPI_INT_IRQ_CHAN                    EXTI0_IRQn      
#define LCD_SPI_INT_IRQ_RAISFALL                EXTI_Trigger_Falling    
#define LCD_SPI_SCK_GPIO_PERIPH                 RCC_AHB1Periph_GPIOA
#define LCD_SPI_SCK_GPIO_PORT                   GPIOA
#define LCD_SPI_SCK_GPIO_PIN                    GPIO_Pin_5
#define LCD_SPI_SCK_GPIO_PIN_SRC                GPIO_PinSource5
#define LCD_SPI_MISO_GPIO_PERIPH                RCC_AHB1Periph_GPIOA
#define LCD_SPI_MISO_GPIO_PORT                  GPIOA
#define LCD_SPI_MISO_GPIO_PIN                   GPIO_Pin_6
#define LCD_SPI_MISO_GPIO_PIN_SRC               GPIO_PinSource6
#define LCD_SPI_MOSI_GPIO_PERIPH                RCC_AHB1Periph_GPIOA
#define LCD_SPI_MOSI_GPIO_PORT                  GPIOA
#define LCD_SPI_MOSI_GPIO_PIN                   GPIO_Pin_7
#define LCD_SPI_MOSI_GPIO_PIN_SRC               GPIO_PinSource7
#define LCD_SPI                                 SPI1
#define LCD_SPI_AF                              GPIO_AF_SPI1
#define LCD_SPI_PERIPH                          RCC_APB2Periph_SPI1
#define LCD_DMA_PERIPH                          RCC_AHB1Periph_DMA2
#define LCD_DMA_TX_STREAM_IRQ_HANDLER           DMA2_Stream5_IRQHandler
#define LCD_DMA_TX_STREAM                       DMA2_Stream5
#define LCD_DMA_IT                              DMA_IT_TCIF5
#define LCD_DMA_CHANNEL                         DMA_Channel_3
#define LCD_DMA_NVIC_IRQCHANNEL                 DMA2_Stream5_IRQn
#define LCD_DMA_TX_TCFLAG			DMA_FLAG_TCIF5


// ACD SENSORS
#define SENSORS_ADC_MAX         4096
#define SENSORS_DC_RATIO        (12.0/(12.0+47.0)) // resistor divider for analog DC sensor ports
#define SENSORS_AC_RATIO        (1.4*9.1/(9.1+1000.0)/2) // resistor divider for tri-phase AC ports
#define ADC_TO_DC_VOLTS(X)      (((float)(X) * SENSORS_ADC_VREF / SENSORS_ADC_MAX) / SENSORS_DC_RATIO)
#define ADC_TO_AC_VOLTS(X)      (((float)(X) * SENSORS_ADC_VREF / SENSORS_ADC_MAX) / SENSORS_AC_RATIO)

#define	SENSORS_TIM_PERIPH	RCC_APB1Periph_TIM2
#define	SENSORS_TIM		TIM2	
#define	SENSORS_TIM_IRQ		TIM2_IRQn
#define SENSORS_TIM_IRQ_HANDLER	TIM2_IRQHandler
#define	SENSORS_DMA_PERIPH	RCC_AHB1Periph_DMA2
#define	SENSORS_DMA		DMA2
#define	SENSORS_DMA_CHANNEL	DMA_Channel_0
#define	SENSORS_DMA_STREAM	DMA2_Stream0
#define	SENSORS_DMA_IRQ		DMA2_Stream0_IRQn
#define SENSORS_DMA_IRQ_HANDLER	DMA2_Stream0_IRQHandler
#define	SENSORS_ADC		ADC1
#define	SENSORS_ADC_PERIPH	RCC_APB2Periph_ADC1
//#define	SENSORS_ADC_CYCLES	ADC_SampleTime_15Cycles
#define	SENSORS_ADC_CYCLES	ADC_SampleTime_144Cycles
#define ADC_NUM_OF_CHANNELS     (12 + 1) // Includes on-board temperature sensor
#define	ADC_MODE		ADC_MODE_INTERFRAME
#define	SENSORS_ADC_VREF	2.5

#define	SENSOR1_PERIPH		RCC_AHB1Periph_GPIOC
#define	SENSOR1_PORT		GPIOC
#define	SENSOR1_PIN		GPIO_Pin_0
#define	SENSOR1_ADC_CHANNEL	ADC_Channel_10
#define	SENSOR2_PERIPH		RCC_AHB1Periph_GPIOC
#define	SENSOR2_PORT		GPIOC
#define	SENSOR2_PIN		GPIO_Pin_1
#define	SENSOR2_ADC_CHANNEL	ADC_Channel_11
#define	SENSOR3_PERIPH		RCC_AHB1Periph_GPIOC
#define	SENSOR3_PORT		GPIOC
#define	SENSOR3_PIN		GPIO_Pin_2
#define	SENSOR3_ADC_CHANNEL	ADC_Channel_12
#define	SENSOR4_PERIPH		RCC_AHB1Periph_GPIOC
#define	SENSOR4_PORT		GPIOC
#define	SENSOR4_PIN		GPIO_Pin_3
#define	SENSOR4_ADC_CHANNEL	ADC_Channel_13
#define	SENSOR5_PERIPH		RCC_AHB1Periph_GPIOA
#define	SENSOR5_PORT		GPIOA
#define	SENSOR5_PIN		GPIO_Pin_0
#define	SENSOR5_ADC_CHANNEL	ADC_Channel_0
#define	SENSOR6_PERIPH		RCC_AHB1Periph_GPIOA
#define	SENSOR6_PORT		GPIOA
#define	SENSOR6_PIN		GPIO_Pin_1
#define	SENSOR6_ADC_CHANNEL	ADC_Channel_1
#define	SENSOR7_PERIPH		RCC_AHB1Periph_GPIOA
#define	SENSOR7_PORT		GPIOA
#define	SENSOR7_PIN		GPIO_Pin_2
#define	SENSOR7_ADC_CHANNEL	ADC_Channel_2
#define	SENSOR8_PERIPH		RCC_AHB1Periph_GPIOA
#define	SENSOR8_PORT		GPIOA
#define	SENSOR8_PIN		GPIO_Pin_3
#define	SENSOR8_ADC_CHANNEL	ADC_Channel_3
#define	SENSOR9_PERIPH		RCC_AHB1Periph_GPIOC
#define	SENSOR9_PORT		GPIOC
#define	SENSOR9_PIN		GPIO_Pin_4
#define	SENSOR9_ADC_CHANNEL	ADC_Channel_14
#define	SENSOR10_PERIPH		RCC_AHB1Periph_GPIOC
#define	SENSOR10_PORT		GPIOC
#define	SENSOR10_PIN		GPIO_Pin_5
#define	SENSOR10_ADC_CHANNEL	ADC_Channel_15
#define	SENSOR11_PERIPH		RCC_AHB1Periph_GPIOB
#define	SENSOR11_PORT		GPIOB
#define	SENSOR11_PIN		GPIO_Pin_0
#define	SENSOR11_ADC_CHANNEL	ADC_Channel_8
#define	SENSOR12_PERIPH		RCC_AHB1Periph_GPIOB
#define	SENSOR12_PORT		GPIOB
#define	SENSOR12_PIN		GPIO_Pin_1
#define	SENSOR12_ADC_CHANNEL	ADC_Channel_9


//EXTI1 (APS protocol)
#define	EXTI1_GPIO_PORT			GPIOE
#define	EXTI1_GPIO_PIN			GPIO_Pin_1
#define	EXTI1_PERIPH			RCC_AHB1Periph_GPIOE
#define	EXTI1_IRQ_PORT_SRC		EXTI_PortSourceGPIOE
#define	EXTI1_IRQ_PIN_SRC		EXTI_PinSource1
#define	EXTI1_IRQ_LINE			EXTI_Line1
#define	EXTI1_IRQ_CHAN			EXTI1_IRQn
#define EXTI1_IRQ_RAISFALL		EXTI_Trigger_Rising_Falling	


#endif //

