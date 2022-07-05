#ifndef __HARDWARE_GROVEX_H__
#define __HARDWARE_GROVEX_H__

#define	OS_PRODUCT_NAME			"Grovex"
#define STM32F42X                       1       // We are running on STM32F42x machine

//#define	TIM5_RESOLUTION		125		// INFO timer is 8000 Hz (125usec)
#define	TIM5_RESOLUTION		104		// INFO timer is 9600 Hz (104usec) for DALI clock  = 2400 Hz

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


#define LCD_SPI_BAUDRATE_PRESCALER      	SPI_BaudRatePrescaler_2
#define LCD_GPIO_SPEED                          GPIO_Speed_100MHz
#define LCD_SPI_CS_GPIO_PERIPH                  RCC_AHB1Periph_GPIOB
#define LCD_SPI_CS_GPIO_PORT                    GPIOB
#define LCD_SPI_CS_GPIO_PIN                     GPIO_Pin_9
#define LCD_SPI_PD_GPIO_PERIPH                  RCC_AHB1Periph_GPIOE
#define LCD_SPI_PD_GPIO_PORT                    GPIOE
#define LCD_SPI_PD_GPIO_PIN                     GPIO_Pin_1
#define LCD_SPI_INT_GPIO_PERIPH                 RCC_AHB1Periph_GPIOE
#define LCD_SPI_INT_GPIO_PORT                   GPIOE
#define LCD_SPI_INT_GPIO_PIN                    GPIO_Pin_0
#define LCD_SPI_INT_IRQ_PORT_SRC                EXTI_PortSourceGPIOE
#define LCD_SPI_INT_IRQ_PIN_SRC                 EXTI_PinSource0
#define LCD_SPI_INT_IRQ_LINE                    EXTI_Line0
#define LCD_SPI_INT_IRQ_CHAN                    EXTI0_IRQn      
#define LCD_SPI_INT_IRQ_RAISFALL                EXTI_Trigger_Falling    
#define LCD_SPI_SCK_GPIO_PERIPH                 RCC_AHB1Periph_GPIOD
#define LCD_SPI_SCK_GPIO_PORT                   GPIOD
#define LCD_SPI_SCK_GPIO_PIN                    GPIO_Pin_3
#define LCD_SPI_SCK_GPIO_PIN_SRC                GPIO_PinSource3
#define LCD_SPI_MISO_GPIO_PERIPH                RCC_AHB1Periph_GPIOB
#define LCD_SPI_MISO_GPIO_PORT                  GPIOB
#define LCD_SPI_MISO_GPIO_PIN                   GPIO_Pin_14
#define LCD_SPI_MISO_GPIO_PIN_SRC               GPIO_PinSource14
#define LCD_SPI_MOSI_GPIO_PERIPH                RCC_AHB1Periph_GPIOB
#define LCD_SPI_MOSI_GPIO_PORT                  GPIOB
#define LCD_SPI_MOSI_GPIO_PIN                   GPIO_Pin_15
#define LCD_SPI_MOSI_GPIO_PIN_SRC               GPIO_PinSource15
#define LCD_SPI                                 SPI2
#define LCD_SPI_AF                              GPIO_AF_SPI2
#define LCD_SPI_PERIPH                          RCC_APB1Periph_SPI2
#define LCD_DMA_PERIPH                          RCC_AHB1Periph_DMA1
#define LCD_DMA_TX_STREAM_IRQ_HANDLER           DMA1_Stream4_IRQHandler
#define LCD_DMA_TX_STREAM                       DMA1_Stream4
#define LCD_DMA_IT                              DMA_IT_TCIF4
#define LCD_DMA_CHANNEL                         DMA_Channel_0
#define LCD_DMA_NVIC_IRQCHANNEL                 DMA1_Stream4_IRQn
#define LCD_DMA_TX_TCFLAG			DMA_FLAG_TCIF4

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


#define ADC_NUM_OF_CHANNELS     (10 + 1) // Includes on-board temperature sensor
#define	ADC_MODE		ADC_MODE_CONTINUOUS
#define	SENSORS_ADC_VREF	3.3

#define	SENSOR1_PERIPH		RCC_AHB1Periph_GPIOA
#define	SENSOR1_PORT		GPIOA
#define	SENSOR1_PIN		GPIO_Pin_0
#define	SENSOR1_ADC_CHANNEL	ADC_Channel_0
#define	SENSOR2_PERIPH		RCC_AHB1Periph_GPIOA
#define	SENSOR2_PORT		GPIOA
#define	SENSOR2_PIN		GPIO_Pin_3
#define	SENSOR2_ADC_CHANNEL	ADC_Channel_3
#define	SENSOR3_PERIPH		RCC_AHB1Periph_GPIOB
#define	SENSOR3_PORT		GPIOB
#define	SENSOR3_PIN		GPIO_Pin_0
#define	SENSOR3_ADC_CHANNEL	ADC_Channel_8
#define	SENSOR4_PERIPH		RCC_AHB1Periph_GPIOB
#define	SENSOR4_PORT		GPIOB
#define	SENSOR4_PIN		GPIO_Pin_1
#define	SENSOR4_ADC_CHANNEL	ADC_Channel_9
#define	SENSOR5_PERIPH		RCC_AHB1Periph_GPIOA
#define	SENSOR5_PORT		GPIOA
#define	SENSOR5_PIN		GPIO_Pin_4
#define	SENSOR5_ADC_CHANNEL	ADC_Channel_4
#define	SENSOR6_PERIPH		RCC_AHB1Periph_GPIOA
#define	SENSOR6_PORT		GPIOA
#define	SENSOR6_PIN		GPIO_Pin_5
#define	SENSOR6_ADC_CHANNEL	ADC_Channel_5
#define	SENSOR7_PERIPH		RCC_AHB1Periph_GPIOA
#define	SENSOR7_PORT		GPIOA
#define	SENSOR7_PIN		GPIO_Pin_6
#define	SENSOR7_ADC_CHANNEL	ADC_Channel_6
#define	SENSOR8_PERIPH		RCC_AHB1Periph_GPIOC
#define	SENSOR8_PORT		GPIOC
#define	SENSOR8_PIN		GPIO_Pin_0
#define	SENSOR8_ADC_CHANNEL	ADC_Channel_10
#define	SENSOR9_PERIPH		RCC_AHB1Periph_GPIOC
#define	SENSOR9_PORT		GPIOC
#define	SENSOR9_PIN		GPIO_Pin_2
#define	SENSOR9_ADC_CHANNEL	ADC_Channel_12
#define	SENSOR10_PERIPH		RCC_AHB1Periph_GPIOC
#define	SENSOR10_PORT		GPIOC
#define	SENSOR10_PIN		GPIO_Pin_3
#define	SENSOR10_ADC_CHANNEL	ADC_Channel_13
#define	SENSOR11_PERIPH		RCC_AHB1Periph_GPIOC
#define	SENSOR11_PORT		GPIOC
#define	SENSOR11_PIN		GPIO_Pin_3
#define	SENSOR11_ADC_CHANNEL	ADC_Channel_14
#define	SENSOR12_PERIPH		RCC_AHB1Periph_GPIOC
#define	SENSOR12_PORT		GPIOC
#define	SENSOR12_PIN		GPIO_Pin_3
#define	SENSOR12_ADC_CHANNEL	ADC_Channel_14



// General Purpose Input/Output
#define GPIO_CHANNELS           12
//IN1
#define GPIO1_DESCR             "IN1_IRQ_RF/DALI1_RX"
#define GPIO1_PERIPH            RCC_AHB1Periph_GPIOE
#define GPIO1_MODE              GPIO_Mode_IN
#define GPIO1_PORT              GPIOE
#define GPIO1_PIN               GPIO_Pin_7
#define GPIO1_IRQ_PORT_SRC      EXTI_PortSourceGPIOE
#define GPIO1_IRQ_PIN_SRC       EXTI_PinSource7
#define GPIO1_IRQ_LINE          EXTI_Line7
#define GPIO1_IRQ_CHAN          EXTI9_5_IRQn
#define GPIO1_IRQ_RAISFALL      EXTI_Trigger_Rising_Falling
//IN2
#define GPIO2_DESCR             "IN2_IRQ_RF/DALI2_RX"
#define GPIO2_PERIPH            RCC_AHB1Periph_GPIOE
#define GPIO2_MODE              GPIO_Mode_IN
#define GPIO2_PORT              GPIOE
#define GPIO2_PIN               GPIO_Pin_8
#define GPIO2_IRQ_PORT_SRC      EXTI_PortSourceGPIOE
#define GPIO2_IRQ_PIN_SRC       EXTI_PinSource8
#define GPIO2_IRQ_LINE          EXTI_Line8
#define GPIO2_IRQ_CHAN          EXTI9_5_IRQn
#define GPIO2_IRQ_RAISFALL      EXTI_Trigger_Rising_Falling
//IN3
#define GPIO3_DESCR             "IN3_IRQ_R"
#define GPIO3_PERIPH            RCC_AHB1Periph_GPIOE
#define GPIO3_MODE              GPIO_Mode_IN
#define GPIO3_PORT              GPIOE
#define GPIO3_PIN               GPIO_Pin_9
#define GPIO3_IRQ_PORT_SRC      EXTI_PortSourceGPIOE
#define GPIO3_IRQ_PIN_SRC       EXTI_PinSource9
#define GPIO3_IRQ_LINE          EXTI_Line9
#define GPIO3_IRQ_CHAN          EXTI9_5_IRQn
#define GPIO3_IRQ_RAISFALL      EXTI_Trigger_Rising
//IN4
#define GPIO4_DESCR             "IN4_IRQ_R"
#define GPIO4_PERIPH            RCC_AHB1Periph_GPIOE
#define GPIO4_MODE              GPIO_Mode_IN
#define GPIO4_PORT              GPIOE
#define GPIO4_PIN               GPIO_Pin_10
#define GPIO4_IRQ_PORT_SRC      EXTI_PortSourceGPIOE
#define GPIO4_IRQ_PIN_SRC       EXTI_PinSource10
#define GPIO4_IRQ_LINE          EXTI_Line10
#define GPIO4_IRQ_CHAN          EXTI15_10_IRQn
#define GPIO4_IRQ_RAISFALL      EXTI_Trigger_Rising
//IN5
#define GPIO5_DESCR             "IN5"
#define GPIO5_PERIPH            RCC_AHB1Periph_GPIOB
#define GPIO5_MODE              GPIO_Mode_IN
#define GPIO5_PORT              GPIOB
#define GPIO5_PIN               GPIO_Pin_7
//IN6
#define GPIO6_DESCR             "IN6"
#define GPIO6_PERIPH            RCC_AHB1Periph_GPIOB
#define GPIO6_MODE              GPIO_Mode_IN
#define GPIO6_PORT              GPIOB
#define GPIO6_PIN               GPIO_Pin_6
//IN7
#define GPIO7_DESCR             "IN7"
#define GPIO7_PERIPH            RCC_AHB1Periph_GPIOB
#define GPIO7_MODE              GPIO_Mode_IN
#define GPIO7_PORT              GPIOB
#define GPIO7_PIN               GPIO_Pin_5
//IN8
#define GPIO8_DESCR             "IN8"
#define GPIO8_PERIPH            RCC_AHB1Periph_GPIOC
#define GPIO8_MODE              GPIO_Mode_IN
#define GPIO8_PORT              GPIOC
#define GPIO8_PIN               GPIO_Pin_12
//IN9
#define GPIO9_DESCR             "IN9"
#define GPIO9_PERIPH            RCC_AHB1Periph_GPIOC
#define GPIO9_MODE              GPIO_Mode_IN
#define GPIO9_PORT              GPIOC
#define GPIO9_PIN               GPIO_Pin_11
//IN10
#define GPIO10_DESCR             "IN10"
#define GPIO10_PERIPH            RCC_AHB1Periph_GPIOC
#define GPIO10_MODE              GPIO_Mode_IN
#define GPIO10_PORT              GPIOC
#define GPIO10_PIN               GPIO_Pin_10
//IN11
#define GPIO11_DESCR             "OUT11/DALI1_TX"
#define GPIO11_PERIPH            RCC_AHB1Periph_GPIOD
#define GPIO11_MODE              GPIO_Mode_OUT
#define GPIO11_PORT              GPIOD
#define GPIO11_PIN               GPIO_Pin_7
//IN12
#define GPIO12_DESCR             "OUT12/DALI2_TX"
#define GPIO12_PERIPH            RCC_AHB1Periph_GPIOD
#define GPIO12_MODE              GPIO_Mode_OUT
#define GPIO12_PORT              GPIOD
#define GPIO12_PIN               GPIO_Pin_4


// Keys
#define	KEY_UP_GPIO_PORT		GPIOE
#define	KEY_UP_GPIO_PIN			GPIO_Pin_2
#define	KEY_UP_PERIPH			RCC_AHB1Periph_GPIOE
#define	KEY_UP_IRQ_PORT_SRC		EXTI_PortSourceGPIOE
#define	KEY_UP_IRQ_PIN_SRC		EXTI_PinSource2
#define	KEY_UP_IRQ_LINE			EXTI_Line2
#define	KEY_UP_IRQ_CHAN			EXTI2_IRQn	
#define KEY_UP_IRQ_RAISFALL		EXTI_Trigger_Rising_Falling	

#define	KEY_DOWN_GPIO_PORT		GPIOE
#define	KEY_DOWN_GPIO_PIN		GPIO_Pin_3
#define	KEY_DOWN_PERIPH			RCC_AHB1Periph_GPIOE
#define	KEY_DOWN_IRQ_PORT_SRC		EXTI_PortSourceGPIOE
#define	KEY_DOWN_IRQ_PIN_SRC		EXTI_PinSource3
#define	KEY_DOWN_IRQ_LINE		EXTI_Line3
#define	KEY_DOWN_IRQ_CHAN		EXTI3_IRQn	
#define KEY_DOWN_IRQ_RAISFALL		EXTI_Trigger_Rising_Falling	

#define	KEY_LEFT_GPIO_PORT		GPIOE
#define	KEY_LEFT_GPIO_PIN		GPIO_Pin_4
#define	KEY_LEFT_PERIPH			RCC_AHB1Periph_GPIOE
#define	KEY_LEFT_IRQ_PORT_SRC		EXTI_PortSourceGPIOE
#define	KEY_LEFT_IRQ_PIN_SRC		EXTI_PinSource4
#define	KEY_LEFT_IRQ_LINE		EXTI_Line4
#define	KEY_LEFT_IRQ_CHAN		EXTI4_IRQn	
#define KEY_LEFT_IRQ_RAISFALL		EXTI_Trigger_Rising_Falling	

#define	KEY_RIGHT_GPIO_PORT		GPIOE
#define	KEY_RIGHT_GPIO_PIN		GPIO_Pin_5
#define	KEY_RIGHT_PERIPH		RCC_AHB1Periph_GPIOE
#define	KEY_RIGHT_IRQ_PORT_SRC		EXTI_PortSourceGPIOE
#define	KEY_RIGHT_IRQ_PIN_SRC		EXTI_PinSource5
#define	KEY_RIGHT_IRQ_LINE		EXTI_Line5
#define	KEY_RIGHT_IRQ_CHAN		EXTI9_5_IRQn
#define KEY_RIGHT_IRQ_RAISFALL		EXTI_Trigger_Rising_Falling	

//OLED
#define	OLED_E_PERIPH			RCC_AHB1Periph_GPIOD
#define	OLED_E_GPIO_PORT		GPIOD
#define	OLED_E_GPIO_PIN			GPIO_Pin_8

#define	OLED_RS_PERIPH			RCC_AHB1Periph_GPIOE
#define	OLED_RS_GPIO_PORT		GPIOE
#define	OLED_RS_GPIO_PIN		GPIO_Pin_15

#define	OLED_RW_PERIPH			RCC_AHB1Periph_GPIOB
#define	OLED_RW_GPIO_PORT		GPIOB
#define	OLED_RW_GPIO_PIN		GPIO_Pin_10

#define	OLED_CS1_PERIPH			RCC_AHB1Periph_GPIOD
#define	OLED_CS1_GPIO_PORT		GPIOD
#define	OLED_CS1_GPIO_PIN		GPIO_Pin_11

#define	OLED_CS2_PERIPH			RCC_AHB1Periph_GPIOD
#define	OLED_CS2_GPIO_PORT		GPIOD
#define	OLED_CS2_GPIO_PIN		GPIO_Pin_10

#define	OLED_D4_PERIPH			RCC_AHB1Periph_GPIOE
#define	OLED_D4_GPIO_PORT		GPIOE
#define	OLED_D4_GPIO_PIN		GPIO_Pin_11

#define	OLED_D5_PERIPH			RCC_AHB1Periph_GPIOE
#define	OLED_D5_GPIO_PORT		GPIOE
#define	OLED_D5_GPIO_PIN		GPIO_Pin_12

#define	OLED_D6_PERIPH			RCC_AHB1Periph_GPIOE
#define	OLED_D6_GPIO_PORT		GPIOE
#define	OLED_D6_GPIO_PIN		GPIO_Pin_13

#define	OLED_D7_PERIPH			RCC_AHB1Periph_GPIOE
#define	OLED_D7_GPIO_PORT		GPIOE
#define	OLED_D7_GPIO_PIN		GPIO_Pin_14

#define	OLED_DATA_GPIO_PORT		GPIOE
#define	OLED_DATA_GPIO_MASK		0x87FF		// AND mask to clear OLED data
#define	OLED_DATA_GPIO_SHIFT		11		// D4 begins from Pin_11


// AC PWM ports
#define	PWM1_PERIPH			RCC_AHB1Periph_GPIOC
#define	PWM1_GPIO_PORT			GPIOC
#define	PWM1_GPIO_PIN			GPIO_Pin_6
#define	PWM1_PIN_SRC			GPIO_PinSource6
#define	PWM1_PIN_AFCONFIG		GPIO_AF_TIM3

#define	PWM2_PERIPH			RCC_AHB1Periph_GPIOC
#define	PWM2_GPIO_PORT			GPIOC
#define	PWM2_GPIO_PIN			GPIO_Pin_7
#define	PWM2_PIN_SRC			GPIO_PinSource7
#define	PWM2_PIN_AFCONFIG		GPIO_AF_TIM3

#define	PWM3_PERIPH			RCC_AHB1Periph_GPIOC
#define	PWM3_GPIO_PORT			GPIOC
#define	PWM3_GPIO_PIN			GPIO_Pin_8
#define	PWM3_PIN_SRC			GPIO_PinSource8
#define	PWM3_PIN_AFCONFIG		GPIO_AF_TIM3

#define	PWM4_PERIPH			RCC_AHB1Periph_GPIOC
#define	PWM4_GPIO_PORT			GPIOC
#define	PWM4_GPIO_PIN			GPIO_Pin_9
#define	PWM4_PIN_SRC			GPIO_PinSource9
#define	PWM4_PIN_AFCONFIG		GPIO_AF_TIM3

// DC PWM ports
#define	PWM5_PERIPH			RCC_AHB1Periph_GPIOD
#define	PWM5_GPIO_PORT			GPIOD
#define	PWM5_GPIO_PIN			GPIO_Pin_12
#define	PWM5_PIN_SRC			GPIO_PinSource12
#define	PWM5_PIN_AFCONFIG		GPIO_AF_TIM4

#define	PWM6_PERIPH			RCC_AHB1Periph_GPIOD
#define	PWM6_GPIO_PORT			GPIOD
#define	PWM6_GPIO_PIN			GPIO_Pin_13
#define	PWM6_PIN_SRC			GPIO_PinSource13
#define	PWM6_PIN_AFCONFIG		GPIO_AF_TIM4

#define	PWM7_PERIPH			RCC_AHB1Periph_GPIOD
#define	PWM7_GPIO_PORT			GPIOD
#define	PWM7_GPIO_PIN			GPIO_Pin_14
#define	PWM7_PIN_SRC			GPIO_PinSource14
#define	PWM7_PIN_AFCONFIG		GPIO_AF_TIM4

#define	PWM8_PERIPH			RCC_AHB1Periph_GPIOD
#define	PWM8_GPIO_PORT			GPIOD
#define	PWM8_GPIO_PIN			GPIO_Pin_15
#define	PWM8_PIN_SRC			GPIO_PinSource15
#define	PWM8_PIN_AFCONFIG		GPIO_AF_TIM4

// DALI protocol Port1 
#define	DALI1_MODE			1	// Master
#define	DALI1_TXRX_POL			1	// Inversed polarity
#define	DALI1_MANCHESTER_POL		0	// 0 - Low-High for "One" 
#define	DALI1_RX_GPIO			0	// GPIO1
#define	DALI1_TX_GPIO			10	// GPIO11
#define	DALI1_SHORT_ADDR		0

/*
// DALI protocol Port2 
#define	DALI2_MODE			0	// Disables
#define	DALI2_TXRX_POL			1	// Inversed polarity
#define	DALI2_MANCHESTER_POL		0	// 0 - Low-High for "One" 
#define	DALI2_RX_GPIO			1	// GPIO2
#define	DALI2_TX_GPIO			11	// GPIO12
#define	DALI2_SHORT_ADDR		0
*/


#endif //

