#include <math.h>
#include "adc.h"
#include "config.h"
#include "hardware.h"
#include "utils.h"
#include "msg.h"

#define	ADC_BUF_SIZE	ADC_NUM_OF_CHANNELS 
#define SQ(X)   ((X) * (X))


volatile int adc_poll_flag = 0;
volatile uint32_t adc_counter = 0;
volatile struct _ADC_DATA ADC_DATA;


void adc_stop(void) {
  	TIM_Cmd(SENSORS_TIM, DISABLE);
	DMA_DeInit(SENSORS_DMA_STREAM);
	ADC_DeInit();

	__disable_irq();
	adc_counter = 0;
	for(int i = 0; i < ADC_NUM_OF_CHANNELS; i++) {
		ADC_DATA.adc_rms[i] = 0;
		ADC_DATA.adc_avg[i] = 0;
		ADC_DATA.adc_sum_rms[i] = 0;
		ADC_DATA.adc_sum_avg[i] = 0;
	}
	__enable_irq();

	print("adc_stop() ADC de-initialized!\r\n");
}

int adc_init(void) {
	ADC_InitTypeDef       ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	DMA_InitTypeDef       DMA_InitStructure;
	GPIO_InitTypeDef      GPIO_InitStructure;
	NVIC_InitTypeDef      NVIC_InitStructure;

	print("adc_init() Initializing ADC...\r\n");

  	TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
  	RCC_APB1PeriphClockCmd(SENSORS_TIM_PERIPH, ENABLE);
  	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  	TIM_TimeBaseStructure.TIM_Prescaler = 84 - 1;
  	TIM_TimeBaseStructure.TIM_Period = config_active.adc.tim_period - 1; //1000 HZ
  	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  	TIM_TimeBaseInit(SENSORS_TIM, &TIM_TimeBaseStructure);

  	TIM_SelectOutputTrigger(SENSORS_TIM, TIM_TRGOSource_Update);

	NVIC_EnableIRQ(SENSORS_TIM_IRQ);                 // Enable IRQ for TIMx in NVIC 
  	TIM_ITConfig(SENSORS_TIM, TIM_IT_Update, ENABLE); // Enable IRQ on update for TimerX

  	TIM_Cmd(SENSORS_TIM, ENABLE);

	RCC_AHB1PeriphClockCmd(SENSORS_DMA_PERIPH, ENABLE);
	
	DMA_DeInit(SENSORS_DMA_STREAM);
  	DMA_InitStructure.DMA_Channel = SENSORS_DMA_CHANNEL;
  	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &SENSORS_ADC->DR; 
  	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADC_DATA.adc_buf;
  	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  	DMA_InitStructure.DMA_BufferSize = ADC_BUF_SIZE;//samples
  	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;

	if(config_active.adc.mode == ADC_MODE_CONTINUOUS) {
  		DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	} else if(config_active.adc.mode == ADC_MODE_SINGLE || config_active.adc.mode == ADC_MODE_INTERFRAME) {
  		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	}

  	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  	DMA_Init(SENSORS_DMA_STREAM, &DMA_InitStructure);
	DMA_ITConfig(SENSORS_DMA_STREAM, DMA_IT_TC, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel =   SENSORS_DMA_IRQ;
      	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
      	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
      	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
      	NVIC_Init(&NVIC_InitStructure);

	// Configure GPIO inputs

	#ifdef SENSOR1_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 0 ) gpio_init(SENSOR1_PERIPH, GPIO_SPEED, SENSOR1_PORT, SENSOR1_PIN, GPIO_Mode_AN, GPIO_OType_PP, GPIO_PuPd_NOPULL); 
	#endif

	#ifdef SENSOR2_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 1 ) gpio_init(SENSOR2_PERIPH, GPIO_SPEED, SENSOR2_PORT, SENSOR2_PIN, GPIO_Mode_AN, GPIO_OType_PP, GPIO_PuPd_NOPULL); 
	#endif

	#ifdef SENSOR3_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 2 ) gpio_init(SENSOR3_PERIPH, GPIO_SPEED, SENSOR3_PORT, SENSOR3_PIN, GPIO_Mode_AN, GPIO_OType_PP, GPIO_PuPd_NOPULL); 
	#endif

	#ifdef SENSOR4_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 3 ) gpio_init(SENSOR4_PERIPH, GPIO_SPEED, SENSOR4_PORT, SENSOR4_PIN, GPIO_Mode_AN, GPIO_OType_PP, GPIO_PuPd_NOPULL); 
	#endif

	#ifdef SENSOR5_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 4 ) gpio_init(SENSOR5_PERIPH, GPIO_SPEED, SENSOR5_PORT, SENSOR5_PIN, GPIO_Mode_AN, GPIO_OType_PP, GPIO_PuPd_NOPULL); 
	#endif

	#ifdef SENSOR6_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 5 ) gpio_init(SENSOR6_PERIPH, GPIO_SPEED, SENSOR6_PORT, SENSOR6_PIN, GPIO_Mode_AN, GPIO_OType_PP, GPIO_PuPd_NOPULL); 
	#endif

	#ifdef SENSOR7_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 6 ) gpio_init(SENSOR7_PERIPH, GPIO_SPEED, SENSOR7_PORT, SENSOR7_PIN, GPIO_Mode_AN, GPIO_OType_PP, GPIO_PuPd_NOPULL); 
	#endif

	#ifdef SENSOR8_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 7 ) gpio_init(SENSOR8_PERIPH, GPIO_SPEED, SENSOR8_PORT, SENSOR8_PIN, GPIO_Mode_AN, GPIO_OType_PP, GPIO_PuPd_NOPULL); 
	#endif

	#ifdef SENSOR9_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 8 ) gpio_init(SENSOR9_PERIPH, GPIO_SPEED, SENSOR9_PORT, SENSOR9_PIN, GPIO_Mode_AN, GPIO_OType_PP, GPIO_PuPd_NOPULL); 
	#endif

	#ifdef SENSOR10_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 9 ) gpio_init(SENSOR10_PERIPH, GPIO_SPEED, SENSOR10_PORT, SENSOR10_PIN, GPIO_Mode_AN, GPIO_OType_PP, GPIO_PuPd_NOPULL); 
	#endif

	#ifdef SENSOR11_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 10 ) gpio_init(SENSOR11_PERIPH, GPIO_SPEED, SENSOR11_PORT, SENSOR11_PIN, GPIO_Mode_AN, GPIO_OType_PP, GPIO_PuPd_NOPULL); 
	#endif

	#ifdef SENSOR12_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 11 ) gpio_init(SENSOR12_PERIPH, GPIO_SPEED, SENSOR12_PORT, SENSOR12_PIN, GPIO_Mode_AN, GPIO_OType_PP, GPIO_PuPd_NOPULL); 
	#endif



  	RCC_APB2PeriphClockCmd(SENSORS_ADC_PERIPH, ENABLE);

  	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
  	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  	ADC_CommonInit(&ADC_CommonInitStructure);

  	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;

	if(config_active.adc.mode == ADC_MODE_CONTINUOUS) {
  		ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
  		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
	} else if(config_active.adc.mode == ADC_MODE_SINGLE || config_active.adc.mode == ADC_MODE_INTERFRAME) {
  		ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	}

  	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  	ADC_InitStructure.ADC_NbrOfConversion = ADC_NUM_OF_CHANNELS;
  	ADC_Init(SENSORS_ADC, &ADC_InitStructure);

	#ifdef SENSOR1_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 0 ) ADC_RegularChannelConfig(SENSORS_ADC, SENSOR1_ADC_CHANNEL, 1, SENSORS_ADC_CYCLES);
	#endif

	#ifdef SENSOR2_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 1 ) ADC_RegularChannelConfig(SENSORS_ADC, SENSOR2_ADC_CHANNEL, 2, SENSORS_ADC_CYCLES);
	#endif

	#ifdef SENSOR3_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 2 ) ADC_RegularChannelConfig(SENSORS_ADC, SENSOR3_ADC_CHANNEL, 3, SENSORS_ADC_CYCLES);
	#endif

	#ifdef SENSOR4_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 3 ) ADC_RegularChannelConfig(SENSORS_ADC, SENSOR4_ADC_CHANNEL, 4, SENSORS_ADC_CYCLES);
	#endif

	#ifdef SENSOR5_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 4 ) ADC_RegularChannelConfig(SENSORS_ADC, SENSOR5_ADC_CHANNEL, 5, SENSORS_ADC_CYCLES);
	#endif

	#ifdef SENSOR6_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 5 ) ADC_RegularChannelConfig(SENSORS_ADC, SENSOR6_ADC_CHANNEL, 6, SENSORS_ADC_CYCLES);
	#endif

	#ifdef SENSOR7_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 6 ) ADC_RegularChannelConfig(SENSORS_ADC, SENSOR7_ADC_CHANNEL, 7, SENSORS_ADC_CYCLES);
	#endif

	#ifdef SENSOR8_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 7 ) ADC_RegularChannelConfig(SENSORS_ADC, SENSOR8_ADC_CHANNEL, 8, SENSORS_ADC_CYCLES);
	#endif

	#ifdef SENSOR9_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 8 ) ADC_RegularChannelConfig(SENSORS_ADC, SENSOR9_ADC_CHANNEL, 9, SENSORS_ADC_CYCLES);
	#endif

	#ifdef SENSOR10_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 9 ) ADC_RegularChannelConfig(SENSORS_ADC, SENSOR10_ADC_CHANNEL, 10, SENSORS_ADC_CYCLES);
	#endif

	#ifdef SENSOR11_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 10 ) ADC_RegularChannelConfig(SENSORS_ADC, SENSOR11_ADC_CHANNEL, 11, SENSORS_ADC_CYCLES);
	#endif

	#ifdef SENSOR12_PORT
	if((ADC_NUM_OF_CHANNELS - 1) > 11 ) ADC_RegularChannelConfig(SENSORS_ADC, SENSOR12_ADC_CHANNEL, 12, SENSORS_ADC_CYCLES);
	#endif


	// Connect on-board temperature sensor to ADC1_CH16 and configure its reading to be the last element of array
	ADC_TempSensorVrefintCmd(ENABLE);
	ADC_VBATCmd(DISABLE);
	ADC_RegularChannelConfig(SENSORS_ADC, ADC_Channel_TempSensor, ADC_NUM_OF_CHANNELS, SENSORS_ADC_CYCLES);

 	// Enable DMA request after last transfer (Single-ADC mode) 
  	ADC_DMARequestAfterLastTransferCmd(SENSORS_ADC, ENABLE);

	if(config_active.adc.mode == ADC_MODE_CONTINUOUS) {
    		DMA_Cmd(SENSORS_DMA_STREAM, ENABLE);
    		ADC_DMACmd(SENSORS_ADC, ENABLE);
    		ADC_Cmd(SENSORS_ADC, ENABLE);
		print("adc_init() ADC mode: %s\r\n", "CONTINUOUS");
	} else if(config_active.adc.mode == ADC_MODE_SINGLE) {
		// Will do thiungs in adc_poll();
		print("adc_init() ADC mode: %s\r\n", "SINGLE");
	} else if(config_active.adc.mode == ADC_MODE_INTERFRAME) {
		// Will do thiungs in adc_poll();
		print("adc_init() ADC mode: %s\r\n", "INTERFRAME");
	} else {
		print("adc_init() ADC mode %d is not supported!\r\n", config_active.adc.mode);
		return -1;
	}

	print("adc_init() Done.\r\n");

	return 0;
}


int adc_poll(int timeout)
{
	int i;

   	DMA_Cmd(SENSORS_DMA_STREAM, ENABLE);
   	ADC_DMACmd(SENSORS_ADC, ENABLE);
	ADC_Cmd(SENSORS_ADC, ENABLE);

	adc_poll_flag = 1;

	ADC_SoftwareStartConv(SENSORS_ADC);

	for(i = 0; i < timeout; i++)
		//if(ADC_GetFlagStatus(SENSORS_ADC, ADC_FLAG_EOS))
		//if(SENSORS_ADC->SR & ADC_SR_EOC)
		if(adc_poll_flag == 0)
			break;

	if(i == timeout) {
		print("adc_poll() ADC timed out, i = %d!\r\n", i);
		return -1;
	}

	return 0;
}


void DMA2_Stream0_IRQHandler(void) {

	__disable_irq();
	if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0)) {

		// Average ADC inputs
		for(int i = 0; i < ADC_NUM_OF_CHANNELS; i++) {
			float adc_delta = (float)ADC_DATA.adc_buf[i] - config_active.adc.offset[i];
			ADC_DATA.adc_sum_rms[i] += (float)SQ(adc_delta);
			ADC_DATA.adc_sum_avg[i] += adc_delta;
		}

		if(++adc_counter >= config_active.adc.samples) {
			// Perform RMS and AVG calculation

			float N = 1.0 / (float)adc_counter;

			for(int i = 0; i < ADC_NUM_OF_CHANNELS; i++) {
				ADC_DATA.adc_rms[i] = sqrtf(ADC_DATA.adc_sum_rms[i] * N) * config_active.adc.coeff[i];
				ADC_DATA.adc_avg[i] = ADC_DATA.adc_sum_avg[i] * N * config_active.adc.coeff[i];
				ADC_DATA.adc_sum_rms[i] = 0;
				ADC_DATA.adc_sum_avg[i] = 0;
			}

			//print("ADC1_EVENT\r\n");
			PostMessageIRQ(ADC1_EVENT, 1, ADC_NUM_OF_CHANNELS, (int)&ADC_DATA);
			adc_counter = 0;
		}

                DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
		adc_poll_flag = 0;
        }
	__enable_irq();
}


void TIM2_IRQHandler(void) {
  	TIM_ClearITPendingBit(TIM2, TIM_IT_Update); // clear interrupt flag
}


