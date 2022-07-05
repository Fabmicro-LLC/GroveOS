#ifndef __HARDWARE_SPUTNIK_H__
#define __HARDWARE_SPUTNIK_H__

#define	OS_PRODUCT_NAME		"Sputnik-Integration"
#define TIM5_RESOLUTION		125		// INFO timer is 8000 MHz (125usec)
#define STM32F42X		1		// We are running on STM32F42x machine

#define	LED_GPIO_PORT		GPIOB
#define	LED_GPIO_PIN		GPIO_Pin_8
#define	LED_PERIPH		RCC_AHB1Periph_GPIOB


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
#define	USART_EXT1_RS485_TX_ENL_GPIO_PIN	GPIO_Pin_4

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
#define	USART_EXT2_RS485_TX_ENL_PERIPH		RCC_AHB1Periph_GPIOD
#define	USART_EXT2_RS485_TX_ENL_GPIO_PORT	GPIOD
#define	USART_EXT2_RS485_TX_ENL_GPIO_PIN	GPIO_Pin_11


// EXT SPI
#define EXT_SPI_GPIO_SPEED                      GPIO_Speed_100MHz
#define EXT_SPI_CS_GPIO_PERIPH                  RCC_AHB1Periph_GPIOB
#define EXT_SPI_CS_GPIO_PORT                    GPIOB
#define EXT_SPI_CS_GPIO_PIN                     GPIO_Pin_12
#define EXT_SPI_CS_GPIO_PIN_SRC			GPIO_PinSource12
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
#define EXT_SPI_IRQ				SPI2_IRQn
#define EXT_SPI_IRQ_HANDLER			SPI2_IRQHandler
#define EXT_SPI_DMA_PERIPH                      RCC_AHB1Periph_DMA1
#define EXT_SPI_DMA_TX_STREAM_IRQ_HANDLER       DMA1_Stream4_IRQHandler
#define EXT_SPI_DMA_TX_STREAM                   DMA1_Stream4
#define EXT_SPI_DMA_RX_STREAM                   DMA1_Stream3
#define EXT_SPI_DMA_CHANNEL                     DMA_Channel_0
#define EXT_SPI_DMA_TX_STREAM_IRQ		DMA1_Stream4_IRQn
#define EXT_SPI_DMA_TX_TCFLAG			DMA_FLAG_TCIF4
#define EXT_SPI_DMA_IT                          DMA_IT_TCIF4


#define CONFIG_SECTOR_NUMBER			1 //config section begins with 0x08004000


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


#define ADC_NUM_OF_CHANNELS     (4 + 1) // Includes on-board temperature sensor
#define	ADC_MODE		ADC_MODE_CONTINUOUS
#define	SENSORS_ADC_VREF	2.5

#define	SENSOR1_PERIPH		RCC_AHB1Periph_GPIOA
#define	SENSOR1_PORT		GPIOA
#define	SENSOR1_PIN		GPIO_Pin_0		// I_SENSOR1
#define	SENSOR1_ADC_CHANNEL	ADC_Channel_0
#define	SENSOR2_PERIPH		RCC_AHB1Periph_GPIOA
#define	SENSOR2_PORT		GPIOA
#define	SENSOR2_PIN		GPIO_Pin_1
#define	SENSOR2_ADC_CHANNEL	ADC_Channel_1		// I_SENSOR2
#define	SENSOR3_PERIPH		RCC_AHB1Periph_GPIOA
#define	SENSOR3_PORT		GPIOA
#define	SENSOR3_PIN		GPIO_Pin_3
#define	SENSOR3_ADC_CHANNEL	ADC_Channel_3		// TEMP_SENSOR
#define	SENSOR4_PERIPH		RCC_AHB1Periph_GPIOA
#define	SENSOR4_PORT		GPIOA
#define	SENSOR4_PIN		GPIO_Pin_4
#define	SENSOR4_ADC_CHANNEL	ADC_Channel_4		// PWR_SENSOR


// General Purpose Input/Output
#define	GPIO_CHANNELS		14
//MOTOR1_ENC1
#define	GPIO1_DESCR		"MOTOR1_ENC1"
#define	GPIO1_PERIPH		RCC_AHB1Periph_GPIOE	
#define	GPIO1_MODE		GPIO_Mode_IN
#define	GPIO1_PORT		GPIOE
#define	GPIO1_PIN		GPIO_Pin_0		
#define GPIO1_IRQ_PORT_SRC	EXTI_PortSourceGPIOE
#define GPIO1_IRQ_PIN_SRC	EXTI_PinSource0
#define GPIO1_IRQ_LINE		EXTI_Line0
#define GPIO1_IRQ_CHAN		EXTI0_IRQn
#define GPIO1_IRQ_RAISFALL	EXTI_Trigger_Rising

// MOTOR1_ENC2
#define	GPIO2_DESCR		"MOTOR1_ENC2"
#define	GPIO2_PERIPH		RCC_AHB1Periph_GPIOE	
#define	GPIO2_MODE		GPIO_Mode_IN
#define	GPIO2_PORT		GPIOE
#define	GPIO2_PIN		GPIO_Pin_1		
#define GPIO2_IRQ_PORT_SRC	EXTI_PortSourceGPIOE
#define GPIO2_IRQ_PIN_SRC	EXTI_PinSource1
#define GPIO2_IRQ_LINE		EXTI_Line1
#define GPIO2_IRQ_CHAN		EXTI1_IRQn
#define GPIO2_IRQ_RAISFALL	EXTI_Trigger_Rising
// MOTOR2_ENC1
#define	GPIO3_DESCR		"MOTOR2_ENC1"
#define	GPIO3_PERIPH		RCC_AHB1Periph_GPIOE	
#define	GPIO3_MODE		GPIO_Mode_IN
#define	GPIO3_PORT		GPIOE
#define	GPIO3_PIN		GPIO_Pin_2		
#define GPIO3_IRQ_PORT_SRC	EXTI_PortSourceGPIOE
#define GPIO3_IRQ_PIN_SRC	EXTI_PinSource2
#define GPIO3_IRQ_LINE		EXTI_Line2
#define GPIO3_IRQ_CHAN		EXTI2_IRQn
#define GPIO3_IRQ_RAISFALL	EXTI_Trigger_Rising
// MOTOR2_ENC2
#define	GPIO4_DESCR		"MOTOR2_ENC2"
#define	GPIO4_PERIPH		RCC_AHB1Periph_GPIOE	
#define	GPIO4_MODE		GPIO_Mode_IN
#define	GPIO4_PORT		GPIOE
#define	GPIO4_PIN		GPIO_Pin_3		
#define GPIO4_IRQ_PORT_SRC	EXTI_PortSourceGPIOE
#define GPIO4_IRQ_PIN_SRC	EXTI_PinSource3
#define GPIO4_IRQ_LINE		EXTI_Line3
#define GPIO4_IRQ_CHAN		EXTI3_IRQn
#define GPIO4_IRQ_RAISFALL	EXTI_Trigger_Rising
// MOTOR1_LIM1
#define	GPIO5_DESCR		"MOTOR1_LIM1"
#define	GPIO5_PERIPH		RCC_AHB1Periph_GPIOE	
#define	GPIO5_MODE		GPIO_Mode_IN
#define	GPIO5_PORT		GPIOE
#define	GPIO5_PIN		GPIO_Pin_5		
// MOTOR1_LIM2
#define	GPIO6_DESCR		"MOTOR1_LIM2"
#define	GPIO6_PERIPH		RCC_AHB1Periph_GPIOE	
#define	GPIO6_MODE		GPIO_Mode_IN
#define	GPIO6_PORT		GPIOE
#define	GPIO6_PIN		GPIO_Pin_6		
// MOTOR2_LIM1
#define	GPIO7_DESCR		"MOTOR2_LIM1"
#define	GPIO7_PERIPH		RCC_AHB1Periph_GPIOE	
#define	GPIO7_MODE		GPIO_Mode_IN
#define	GPIO7_PORT		GPIOE
#define	GPIO7_PIN		GPIO_Pin_7		
// MOTOR2_LIM2
#define	GPIO8_DESCR		"MOTOR2_LIM2"
#define	GPIO8_PERIPH		RCC_AHB1Periph_GPIOE	
#define	GPIO8_MODE		GPIO_Mode_IN
#define	GPIO8_PORT		GPIOE
#define	GPIO8_PIN		GPIO_Pin_8		
// INPUT1
#define	GPIO9_DESCR		"INPUT1"
#define	GPIO9_PERIPH		RCC_AHB1Periph_GPIOE	
#define	GPIO9_MODE		GPIO_Mode_IN
#define	GPIO9_PORT		GPIOE
#define	GPIO9_PIN		GPIO_Pin_9		
// INPUT2
#define	GPIO10_DESCR		"INPUT2"
#define	GPIO10_PERIPH		RCC_AHB1Periph_GPIOE	
#define	GPIO10_MODE		GPIO_Mode_IN
#define	GPIO10_PORT		GPIOE
#define	GPIO10_PIN		GPIO_Pin_10		
// MOTOR1_DIR1
#define	GPIO11_DESCR		"MOTOR1_DIR1"
#define	GPIO11_PERIPH		RCC_AHB1Periph_GPIOD	
#define	GPIO11_MODE		GPIO_Mode_OUT
#define	GPIO11_PORT		GPIOD
#define	GPIO11_PIN		GPIO_Pin_15		
// MOTOR1_DIR2
#define	GPIO12_DESCR		"MOTOR1_DIR2"
#define	GPIO12_PERIPH		RCC_AHB1Periph_GPIOD	
#define	GPIO12_MODE		GPIO_Mode_OUT
#define	GPIO12_PORT		GPIOD
#define	GPIO12_PIN		GPIO_Pin_14		
// MOTOR2_DIR1
#define	GPIO13_DESCR		"MOTOR2_DIR1"
#define	GPIO13_PERIPH		RCC_AHB1Periph_GPIOC	
#define	GPIO13_MODE		GPIO_Mode_OUT
#define	GPIO13_PORT		GPIOC
#define	GPIO13_PIN		GPIO_Pin_9		
// MOTOR2_DIR2
#define	GPIO14_DESCR		"MOTOR2_DIR2"
#define	GPIO14_PERIPH		RCC_AHB1Periph_GPIOC	
#define	GPIO14_MODE		GPIO_Mode_OUT
#define	GPIO14_PORT		GPIOC
#define	GPIO14_PIN		GPIO_Pin_8		


// PWM ports

// MOTOR1_EN
#define	PWM1_PERIPH			RCC_AHB1Periph_GPIOC
#define	PWM1_GPIO_PORT			GPIOC
#define	PWM1_GPIO_PIN			GPIO_Pin_6
#define	PWM1_PIN_SRC			GPIO_PinSource6
#define	PWM1_PIN_AFCONFIG		GPIO_AF_TIM3

// MOTOR2_EN
#define	PWM2_PERIPH			RCC_AHB1Periph_GPIOC
#define	PWM2_GPIO_PORT			GPIOC
#define	PWM2_GPIO_PIN			GPIO_Pin_7
#define	PWM2_PIN_SRC			GPIO_PinSource7
#define	PWM2_PIN_AFCONFIG		GPIO_AF_TIM3



#endif //

