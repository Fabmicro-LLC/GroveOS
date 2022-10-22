/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2016-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#ifndef __HARDWARE_APS_H__
#define __HARDWARE_APS_H__

#define	OS_PRODUCT_NAME		"SmartPWR6"
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
#define	USART_EXT2_BAUD			115200	
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


// EXT SPI
#define EXT_SPI_GPIO_SPEED                      GPIO_Speed_100MHz
#define EXT_SPI_CS_GPIO_PERIPH                  RCC_AHB1Periph_GPIOB
#define EXT_SPI_CS_GPIO_PORT                    GPIOB
#define EXT_SPI_CS_GPIO_PIN                     GPIO_Pin_12
#define EXT_SPI_CS_GPIO_PIN_SRC                 GPIO_PinSource12
#define EXT_SPI_SCK_GPIO_PERIPH                 RCC_AHB1Periph_GPIOB
#define EXT_SPI_SCK_GPIO_PORT                   GPIOB
#define EXT_SPI_SCK_GPIO_PIN                    GPIO_Pin_13
#define EXT_SPI_SCK_GPIO_PIN_SRC                GPIO_PinSource13
#define EXT_SPI_MISO_GPIO_PERIPH                RCC_AHB1Periph_GPIOB
#define EXT_SPI_MISO_GPIO_PORT                  GPIOB
#define EXT_SPI_MISO_GPIO_PIN                   GPIO_Pin_14
#define EXT_SPI_MISO_GPIO_PIN_SRC               GPIO_PinSource14
#define EXT_SPI_MOSI_GPIO_PERIPH                RCC_AHB1Periph_GPIOB
#define EXT_SPI_MOSI_GPIO_PORT                  GPIOB
#define EXT_SPI_MOSI_GPIO_PIN                   GPIO_Pin_15
#define EXT_SPI_MOSI_GPIO_PIN_SRC               GPIO_PinSource15
#define EXT_SPI                                 SPI2
#define EXT_SPI_AF                              GPIO_AF_SPI2
#define EXT_SPI_PERIPH                          RCC_APB1Periph_SPI2
#define EXT_SPI_IRQ                             SPI2_IRQn
#define EXT_SPI_IRQ_HANDLER                     SPI2_IRQHandler
#define EXT_SPI_DMA_PERIPH                      RCC_AHB1Periph_DMA1
#define EXT_SPI_DMA_TX_STREAM_IRQ_HANDLER       DMA1_Stream4_IRQHandler
#define EXT_SPI_DMA_TX_STREAM                   DMA1_Stream4
#define EXT_SPI_DMA_RX_STREAM                   DMA1_Stream3
#define EXT_SPI_DMA_CHANNEL                     DMA_Channel_0
#define EXT_SPI_DMA_TX_STREAM_IRQ               DMA1_Stream4_IRQn
#define EXT_SPI_DMA_TX_TCFLAG                   DMA_FLAG_TCIF4
#define EXT_SPI_DMA_IT                          DMA_IT_TCIF4


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
#define ADC_MODE                ADC_MODE_CONTINUOUS
#define SENSORS_ADC_VREF        2.5



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



#define GPIO_CHANNELS		13	
//
#define GPIO1_DESCR             "PGOOD1"
#define GPIO1_PERIPH            RCC_AHB1Periph_GPIOD    
#define GPIO1_MODE              GPIO_Mode_IN
#define GPIO1_PORT              GPIOD
#define GPIO1_PIN               GPIO_Pin_4   
//
#define GPIO2_DESCR             "PGOOD2"
#define GPIO2_PERIPH            RCC_AHB1Periph_GPIOD    
#define GPIO2_MODE              GPIO_Mode_IN
#define GPIO2_PORT              GPIOD
#define GPIO2_PIN               GPIO_Pin_3   
//
#define GPIO3_DESCR             "PGOOD3"
#define GPIO3_PERIPH            RCC_AHB1Periph_GPIOC    
#define GPIO3_MODE              GPIO_Mode_IN
#define GPIO3_PORT              GPIOC
#define GPIO3_PIN               GPIO_Pin_12   
//
#define GPIO4_DESCR             "PGOOD4"
#define GPIO4_PERIPH            RCC_AHB1Periph_GPIOC    
#define GPIO4_MODE              GPIO_Mode_IN
#define GPIO4_PORT              GPIOC
#define GPIO4_PIN               GPIO_Pin_11
//
#define GPIO5_DESCR             "PGOOD5"
#define GPIO5_PERIPH            RCC_AHB1Periph_GPIOC    
#define GPIO5_MODE              GPIO_Mode_IN
#define GPIO5_PORT              GPIOC
#define GPIO5_PIN               GPIO_Pin_10   
//
#define GPIO6_DESCR             "PGOOD6"
#define GPIO6_PERIPH            RCC_AHB1Periph_GPIOD    
#define GPIO6_MODE              GPIO_Mode_IN
#define GPIO6_PORT              GPIOD
#define GPIO6_PIN               GPIO_Pin_7
//
#define GPIO7_DESCR             "POTEN"
#define GPIO7_PERIPH            RCC_AHB1Periph_GPIOE    
#define GPIO7_MODE              GPIO_Mode_OUT
#define GPIO7_PORT              GPIOE
#define GPIO7_PIN               GPIO_Pin_1
//
#define GPIO8_DESCR             "PWREN1"
#define GPIO8_PERIPH            RCC_AHB1Periph_GPIOC    
#define GPIO8_MODE              GPIO_Mode_OUT
#define GPIO8_PORT              GPIOC
#define GPIO8_PIN               GPIO_Pin_6
//
#define GPIO9_DESCR             "PWREN2"
#define GPIO9_PERIPH            RCC_AHB1Periph_GPIOC    
#define GPIO9_MODE              GPIO_Mode_OUT
#define GPIO9_PORT              GPIOC
#define GPIO9_PIN               GPIO_Pin_7
//
#define GPIO10_DESCR             "PWREN3"
#define GPIO10_PERIPH            RCC_AHB1Periph_GPIOC    
#define GPIO10_MODE              GPIO_Mode_OUT
#define GPIO10_PORT              GPIOC
#define GPIO10_PIN               GPIO_Pin_8
//
#define GPIO11_DESCR             "PWREN4"
#define GPIO11_PERIPH            RCC_AHB1Periph_GPIOC    
#define GPIO11_MODE              GPIO_Mode_OUT
#define GPIO11_PORT              GPIOC
#define GPIO11_PIN               GPIO_Pin_9
//
#define GPIO12_DESCR             "PWREN5"
#define GPIO12_PERIPH            RCC_AHB1Periph_GPIOD    
#define GPIO12_MODE              GPIO_Mode_OUT
#define GPIO12_PORT              GPIOD
#define GPIO12_PIN               GPIO_Pin_12
//
#define GPIO13_DESCR             "PWREN6"
#define GPIO13_PERIPH            RCC_AHB1Periph_GPIOD    
#define GPIO13_MODE              GPIO_Mode_OUT
#define GPIO13_PORT              GPIOD
#define GPIO13_PIN               GPIO_Pin_12




#endif //

