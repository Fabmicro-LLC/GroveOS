#ifndef _ADC_H_
#define _ADC_H_

#include <stdint.h>
#include "hardware.h"


#define	ADC_MODE_CONTINUOUS	0	// Scan ADC continuously
#define	ADC_MODE_SINGLE		1	// Scan ADC upon user request
#define	ADC_MODE_INTERFRAME	2	// Scan ADC between LCD frame updates

#define	ADC_MODE_SINGLE_TIMEOUT	600000L

#pragma pack(1)
struct _ADC_DATA {
        float adc_rms[ADC_NUM_OF_CHANNELS]; //should be first !!!!
        float adc_avg[ADC_NUM_OF_CHANNELS];
        float adc_sum_rms[ADC_NUM_OF_CHANNELS];
        float adc_sum_avg[ADC_NUM_OF_CHANNELS];
        uint16_t adc_buf[ADC_NUM_OF_CHANNELS];
};
#pragma pack()

extern volatile struct _ADC_DATA ADC_DATA;


typedef void (*ADCUPDATEPROC)(uint16_t* adc_data, uint32_t num_of_channels);
int adc_init(void);
int adc_poll(int timeout);
void adc_stop(void);
void adc_set_update_proc(ADCUPDATEPROC proc);

#endif
