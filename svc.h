/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#ifndef _SVC_H_
#define	_SVC_H_

#include <stdint.h> 

#define	IN_PORT_1		0
#define	IN_PORT_2		1
#define	IN_PORT_3		2
#define	IN_PORT_4		3
#define	IN_PORT_5		4
#define	IN_PORT_6		5
#define	IN_PORT_7		6
#define	IN_PORT_8		7
#define	IN_PORT_9		8
#define	IN_PORT_10		9

#define SENSOR_PORT_1		0
#define SENSOR_PORT_2		1
#define SENSOR_PORT_3		2
#define SENSOR_PORT_4		3
#define SENSOR_PORT_5		4
#define SENSOR_PORT_6		5
#define SENSOR_PORT_7		6
#define SENSOR_PORT_8		7
#define SENSOR_PORT_9		8
#define SENSOR_PORT_10		9
#define SENSOR_PORT_11		10	
#define SENSOR_PORT_12		11	
#define SENSOR_PORT_13		12	
#define SENSOR_PORT_14		13	
#define SENSOR_PORT_15		14	
#define SENSOR_PORT_16		15	

#define DC_PORT_1		0
#define DC_PORT_2		1
#define DC_PORT_3		2
#define DC_PORT_4		3

#define AC_PORT_1		0
#define AC_PORT_2		1
#define AC_PORT_3		2
#define AC_PORT_4		3

#define	SVC_DEBUG_PRINT		1	// Output string to debug console
#define	SVC_GET_TEXT		2	// Returns address of application text segment  
#define	SVC_EXEC		3	// Execute application by name, if name is NULL - execute default app. Returns 0 on success 
#define	SVC_STOP		4	// Stop execution of user application
#define	SVC_GET_DATA		5	// Returns application data segment address
#define	SVC_POST_MESSAGE	6	// Post message to system message queue
#define	SVC_GET_OS_VERSION	7	// Get operating system build number

#define	SVC_TFS_CREATE		21	// Create new file on TFS file system
#define	SVC_TFS_WRITE		22	// Write block to newly created file
#define	SVC_TFS_CLOSE		23	// Close created file
#define	SVC_TFS_FIND		24	// Find file by its name (returns TFS header)
#define	SVC_TFS_FIND_NEXT	25	// Find next file file in a list
#define	SVC_TFS_FORMAT		26	// Erase all data on Flash
#define	SVC_TFS_GET_BEGIN	27	// Returns TFS pointing to beginnign of file system
#define	SVC_TFS_APLAY		28	// Play PCMU audio file 
#define	SVC_TFS_GET_FREE	29	// Returns max size of a file possible to create

#define	SVC_OLED_PRINT		30	// Print text string onto OLED display
#define	SVC_OLED_BLIT		31	// Blit bitmap onto OLED display
#define	SVC_OLED_CLEAR		32	// Clear OLED display

#define	SVC_VAULT_GET		35	// Get named variable value from vault storage
#define	SVC_VAULT_SET		36	// Set named variable value in vault storage
#define	SVC_VAULT_DEL		37	// Delete named variable from vault storage 
#define	SVC_VAULT_ENUM		38	// Enumerate variables vault storage 

#define	SVC_READ_IN		41	// Read state of IN port 
#define	SVC_SET_PWM		43	// Write PWM value to any port
#define	SVC_SET_DC_PWM		43	// Write PWM value for DC port 
#define	SVC_SET_AC_PWM		44	// Write PWM value for AC port 
#define	SVC_GET_PWM		45	// Read PWM value for any port 
#define	SVC_READ_DC_PWM		45	// Read PWM value for DC port 
#define	SVC_READ_AC_PWM		46	// Read PWM value for AC port 
#define	SVC_SET_PWM_CLOCK	47	// Set CLOCK value for DC and AC ports 
#define	SVC_SET_DC_CLOCK	47	// Write CLOCK value for DC port 
#define	SVC_SET_AC_CLOCK	48	// Write CLOCK value for AC port 
#define	SVC_SET_CONFIG		49	// Set global configuration variable
#define	SVC_READ_CONFIG		40	// Read global configuration variable

#define	SVC_SOFTTIMER_RUN	50	// Run new software timer
#define	SVC_SOFTTIMER_STOP	51	// Stop software timer

#define	SVC_GET_TIME		55	// Get RTC time
#define	SVC_GET_DATE		56	// Get RTC date
#define	SVC_SET_TIME		57	// Set RTC time
#define	SVC_SET_DATE		58	// Set RTC date


#define	SVC_MALLOC		60	// Allocate memory block in system area for DMA/IO operation
#define	SVC_FREE		61	// Free allocated memory
#define	SVC_DC_PWM_XFER		62	// Transfer sequence of PWM over DC port

#define	SVC_MODBUS1_ENQUEUE_REQUEST	70	// Enqueue new modbus request
#define	SVC_MODBUS1_REGISTER_RESPONDER	71	// Register modbus responder
#define	SVC_MODBUS1_UNREGISTER_RESPONDER	72	// Unregister modbus responder
#define	SVC_MODBUS1_SUBMIT_RESPONSE	73	// Submit modbus response

#define	SVC_MODBUS2_ENQUEUE_REQUEST	74	// Enqueue new modbus request
#define	SVC_MODBUS2_REGISTER_RESPONDER	75	// Register modbus responder
#define	SVC_MODBUS2_UNREGISTER_RESPONDER	76	// Unregister modbus responder
#define	SVC_MODBUS2_SUBMIT_RESPONSE	77	// Submit modbus response

#define	SVC_EXT_SPI_REQUEST	78	// Submit SPI transfer
#define	SVC_EXT_SPI_STOP	79	// Cancel SPI transfer

#define	SVC_DALI1_ENQUEUE_REQUEST	80	// Enqueue new DALI request
#define	SVC_DALI1_REGISTER_RESPONDER	81	// Register DALI responder
#define	SVC_DALI1_UNREGISTER_RESPONDER	82	// Unregister DALI responder
#define	SVC_DALI1_SUBMIT_RESPONSE	83	// Submit modbus response

#define	SVC_DALI2_ENQUEUE_REQUEST	84	// Enqueue new DALI request
#define	SVC_DALI2_REGISTER_RESPONDER	85	// Register DALI responder
#define	SVC_DALI2_UNREGISTER_RESPONDER	86	// Unregister DALI responder
#define	SVC_DALI2_SUBMIT_RESPONSE	87	// Submit modbus response

#define SVC_GET_ROOT_WINDOW	99	// Return root window (screen)
#define SVC_SET_ROOT_WINDOW	100	// Set new root window
#define SVC_WND_CREATE          101	// Create a window
#define SVC_WND_DESTROY         102	// Destroy the window
#define SVC_WND_GET_CLASS       107	// Return a window class by the name
#define SVC_GET_FONT		108	// Return default fonts
#define SVC_WND_REGISTER_CLASS	109	// Register a new window class
#define SVC_NSVG_RASTERIZER	120	// Return nanosvg rasterizer for drawing 
#define SVC_NSVG_DRAW		121	// Draw svg

#define SVC_SHOW_INPUT_NUM_FORMAT 129	// Dialog for entering numic data using user format
#define SVC_SHOW_ADC_SETUP	130	// Dialog for calibrating ADC
#define SVC_SHOW_SET_TIME	132	// Dialog for settings time and date
#define SVC_SHOW_INPUT_TEXT     133	// Dialog for entering text
#define SVC_SHOW_INPUT_NUM	134	// Dialog for entering numic data
#define SVC_SHOW_CALIBRATE_TOUCH 135	// Dialog for calibrate touch screen
#define SVC_SHOW_ALERT		136	// Dialog with alert message
#define SVC_SHOW_SELECT_LIST	137	// Dialog for entering value from list

#define SVC_LOGGER_WRITE_DATA	138	
#define SVC_LOGGER_ERASE_CURRENT_SECTOR 139
#define SVC_LOGGER_GET_PREVIOUS_ITEM 140
#define SVC_LOGGER_GET_CURRENT_ITEM 141
#define	SVC_SET_ADC_COEFF	142	// ADC voltage conversion coefficient
#define	SVC_SET_ADC_OFFSET	143	// ADC voltage offset 
#define	SVC_GET_ADC_COEFF	144	// ADC voltage conversion coefficient
#define	SVC_GET_ADC_OFFSET	145	// ADC voltage offset 
#define	SVC_READ_ADC		146	// Read raw value of ADC channel
#define	SVC_READ_SENSOR_RMS	147	// Read RMS value of ADC channel
#define	SVC_GPIO_INIT_PULLUP	148	// Init GPIO using pull-up resistor
#define	SVC_GPIO_INIT_NOPULL	149	// Init GPIO, no pull
#define	SVC_GPIO_IRQ_FALL	150	// Init IRQ on GPIO, trigger on falling edge
#define	SVC_GPIO_IRQ_RISE	151	// Init IRQ on GPIO, trigger on raising edge
#define	SVC_GPIO_IRQ_RISEFALL	152	// Init IRQ on GPIO, trigger both on raising and falling edge

#define SVC_EXT_IRQ_SET		153
#define SVC_EXT_IRQ_GET		154
#define SVC_EXT_IRQ_CLEAR	155

#define SVC_LISTENER_SET	156
#define SVC_LISTENER_REMOVE	157

#define SVC_GET_ALL_ADC_DATA	158
#define	SVC_READ_SENSOR_AVG	159	// Read AVG value of ADC channel


#define	CONFIG_DEFAULT_APP	1	// Name of the existing application to be run on system startup
#define	CONFIG_EXT1_BAUD_RATE	2	// EXT1/RS485 port baud rate
#define	CONFIG_EXT1_PARITY	3	// EXT1/RS485 port parity (0 - none, 1 - event, 2 - odd)
#define	CONFIG_EXT1_STOP_BITS	4	// EXT1/RS485 port stop bits 
#define	CONFIG_EXT1_MODBUS_MODE	5	// EXT1/RS485 port Modbus protocol mode (0 - off, 1 - master, 2 - slave)
#define	CONFIG_EXT1_MODBUS_ADDR	6	// EXT1/RS485 port Modbus address (master always has 0)
#define	CONFIG_EXT2_BAUD_RATE	7	// EXT2/RS485 port baud rate
#define	CONFIG_EXT2_PARITY	8	// EXT2/RS485 port parity (0 - none, 1 - event, 2 - odd)
#define	CONFIG_EXT2_STOP_BITS	9	// EXT2/RS485 port stop bits 
#define	CONFIG_EXT2_MODBUS_MODE	10	// EXT2/RS485 port Modbus protocol mode (0 - off, 1 - master, 2 - slave)
#define	CONFIG_EXT2_MODBUS_ADDR	11	// EXT2/RS485 port Modbus address (master always has 0)
#define CONFIG_LOGGER_ENABLED	20	// Logger enable flag 
#define CONFIG_LCD_ENABLED	21	// LCD enable flag 
#define CONFIG_ADC_ENABLED	22	// ADC enable flag 
#define CONFIG_USER_DATA	23	// User data (bytes array)
#define CONFIG_DEBUG_ENABLED	24	// Debug enable flag
#define	CONFIG_DALI1_MODE	25	// DALI PORT1 mode
#define	CONFIG_DALI1_SHORT_ADDR	26	// DALI PORT1 short address 
#define	CONFIG_DALI2_MODE	27	// DALI PORT1 mode
#define	CONFIG_DALI2_SHORT_ADDR	28	// DALI PORT1 short address 


#define svc(code) asm volatile ("vpush {s0-s3}"); \
                  asm volatile ("svc %[immediate]"::[immediate] "I" (code)); \
                  asm volatile ("vpop {s0-s3}");



void svc_stop(void);
int svc_debug_print(const char *string, int length);
int svc_get_text(void);
char* svc_get_data(void);
int svc_post_message(int msg, int uniq, int p1, int p2);
int svc_exec(const char* name);
int svc_get_os_version(void);
int svc_read_in(int in_port_num);
void svc_set_pwm(int port_num, float pwm);
float svc_get_pwm(int port_num);
void svc_set_dc_pwm(int dc_port_num, float pwm);
void svc_set_ac_pwm(int ac_port_num, float pwm);
void svc_set_dc_clock(int clock);
void svc_set_ac_clock(int clock);
float svc_read_dc_pwm(int dc_port_num);
float svc_read_ac_pwm(int ac_port_num);
void svc_softtimer_run(int id, int timeout, int p1, int p2);
void svc_softtimer_stop(int id);
int svc_set_config(int conf_var_id, void* data);
int svc_get_config(int conf_var_id, void* data);
void svc_microsec_timer_enable(int enable);

// ADC
int svc_set_adc_coeff(int adc_channel, float val);
int svc_set_adc_offset(int adc_channel, float val);
int svc_get_adc_coeff(int adc_channel, float* val);
int svc_get_adc_offset(int adc_channel, float* val);
int svc_read_adc(int adc_channel, int* val); // read raw value of ADC 
int svc_read_sensor_rms(int adc_channel, float* val); // read RMS value of ADC channel
int svc_read_sensor_avg(int adc_channel, float* val); // read Average value of ADC channel
#define	svc_read_sensor(A, B)	svc_read_sensor_rms((A), (B))

float* svc_get_all_adc_data();

// Print text string onto OLED. x - position in pixels, line - display line number.
int svc_oled_print(int size, char* text, int x, int line);

// Outputs bitmap to OLED display.
int svc_oled_blit(int size, char* bitmap, int x, int line);

// Clear OLED display
void svc_oled_clear(void);

// Memory management
 void* svc_malloc(int size);
int svc_free(void* buf);

// Transfer sequence of PWM over a DC port
int svc_dc_pwm_transfer(int dc_port, uint8_t* buf, uint32_t size, int cyclic);


// Vault - a simple permament named storage

// returns error or number of bytes read
int svc_vault_get(char *var_name, void **data);

// returns error or number of bytes written 
int svc_vault_set(char *var_name, void *data, int size, int offset);

// returns error or number of vars deleted
int svc_vault_del(char *var_name);

// Enumerates index list
int svc_vault_enum(int idx, char *app_name, char *var_name, int* size);

int svc_get_time(int *TimeBin); // returns RTC time in binary format
int svc_get_date(int *DateBin); // returns RTC date in binary format
int svc_set_time(int *TimeBin); // sets RTC time, TimeBin is RTC time in binary format
int svc_set_date(int *DateBin); // sets RTC date, DateBin is RTC date in binary format

//GPIO
int svc_gpio_init_pullup(uint32_t RCC_AHB1Periph, void* GPIOx, unsigned short GPIO_Pin, unsigned short InOut);
int svc_gpio_init_nopull(uint32_t RCC_AHB1Periph, void* GPIOx, unsigned short GPIO_Pin, unsigned short InOut);
int svc_gpio_irq_fall(unsigned short PortSrc, unsigned short PortPin, unsigned short Line, unsigned short IrqChan);
int svc_gpio_irq_rise(unsigned short PortSrc, unsigned short PortPin, unsigned short Line, unsigned short IrqChan);
int svc_gpio_irq_risefall(unsigned short PortSrc, unsigned short PortPin, unsigned short Line, unsigned short IrqChan);

#endif //_SVC_H_

#ifdef SVC_CLIENT_IMPL

char debug_str[256];
#define print(...) { svc_debug_print (debug_str, sprintf(debug_str,__VA_ARGS__)); }


#ifndef _SVC_H_IMPL_
#define _SVC_H_IMPL_

#include "svc.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"

// Stops execution of MSG queue function
__attribute__ ((noinline)) void svc_stop(void)
{
        svc(SVC_STOP);
}

// Outputs string of text to console
__attribute__ ((noinline)) int svc_debug_print(const char *string, int length)
{
        svc(SVC_DEBUG_PRINT);
}

// Returns application text segment address 
__attribute__ ((noinline)) int svc_get_text(void)
{
        svc(SVC_GET_TEXT);
}

// Returns application data segment address 
__attribute__ ((noinline)) char* svc_get_data(void)
{
	svc(SVC_GET_DATA);
}

// Post message to system message queue 
__attribute__ ((noinline)) int svc_post_message(int msg, int uniq, int p1, int p2)
{
        svc(SVC_POST_MESSAGE);
}

// Get operating system build number
__attribute__ ((noinline)) int svc_get_os_version()
{
        svc(SVC_GET_OS_VERSION);
}

// Execute application 
__attribute__ ((noinline)) int svc_exec(const char *name)
{
        svc(SVC_EXEC);
}

// Set value of global configuration variable
__attribute__ ((noinline)) int svc_set_config(int conf_var_id, void* data)
{
        svc(SVC_SET_CONFIG);
}

// Set value of ADC voltage conversion coefficients 
__attribute__ ((noinline)) int svc_set_adc_coeff(int adc_channel, float val)
{
        svc(SVC_SET_ADC_COEFF);
}

// Set value of ADC voltage offsets 
__attribute__ ((noinline)) int svc_set_adc_offset(int adc_channel, float val)
{
        svc(SVC_SET_ADC_OFFSET);
}

// Get value of ADC voltage conversion coefficients 
__attribute__ ((noinline)) int svc_get_adc_coeff(int adc_channel, float *val)
{
        svc(SVC_GET_ADC_COEFF);
}

// Get value of ADC voltage offsets 
__attribute__ ((noinline)) int svc_get_adc_offset(int adc_channel, float *val)
{
        svc(SVC_GET_ADC_OFFSET);
}

// Get raw value of ADC channel  
__attribute__ ((noinline)) int svc_read_adc(int adc_channel, int* val)
{
        svc(SVC_READ_ADC);
}

// Get RMS value of ADC channel  
__attribute__ ((noinline)) int svc_read_sensor_rms(int adc_channel, float* val)
{
        svc(SVC_READ_SENSOR_RMS);
}

// Get AVG value of ADC channel  
__attribute__ ((noinline)) int svc_read_sensor_avg(int adc_channel, float* val)
{
        svc(SVC_READ_SENSOR_AVG);
}

__attribute__ ((noinline)) float* svc_get_all_adc_data() 
{
	svc(SVC_GET_ALL_ADC_DATA);
}

// Read value of global configuration variable
__attribute__ ((noinline)) int svc_get_config(int conf_var_id, void* data)
{
        svc(SVC_READ_CONFIG);
}

// Print text string onto OLED. x - position in pixels, line - display line number.
__attribute__ ((noinline)) int svc_oled_print(int size, char* text, int x, int line)
{
	svc(SVC_OLED_PRINT);
}


// Outputs bitmap to OLED display.
__attribute__ ((noinline)) int svc_oled_blit(int size, char* bitmap, int x, int line)
{
	svc(SVC_OLED_PRINT);
}

// Clear OLED display.
__attribute__ ((noinline)) void svc_oled_clear(void)
{
	svc(SVC_OLED_CLEAR);
}

// Read state of IN port 
__attribute__ ((noinline)) int svc_read_in(int in_port_num)
{
	svc(SVC_READ_IN);
}

// Set any port PWM value
__attribute__ ((noinline)) void svc_set_pwm(int port_num, float pwm)
{
	svc(SVC_SET_PWM);
}

// Set DC port PWM value
__attribute__ ((noinline)) void svc_set_dc_pwm(int dc_port_num, float pwm)
{
	svc(SVC_SET_DC_PWM);
}
 
// Read any port PWM value
__attribute__ ((noinline)) float svc_get_pwm(int port_num)
{
	svc(SVC_GET_PWM);
	asm volatile ("vmov s0, r0\n");
}
 
// Read DC port PWM value
__attribute__ ((noinline)) float svc_read_dc_pwm(int dc_port_num)
{
	svc(SVC_READ_DC_PWM);
	asm volatile ("vmov s0, r0\n");
}
 
// Set AC port PWM value
__attribute__ ((noinline)) void svc_set_ac_pwm(int ac_port_num, float pwm)
{
	svc(SVC_SET_AC_PWM);
}

// Read AC port PWM value
__attribute__ ((noinline)) float svc_read_ac_pwm(int ac_port_num)
{
	svc(SVC_READ_AC_PWM);
	asm volatile ("vmov s0, r0\n");
}

// Set DC port clock 
__attribute__ ((noinline)) void svc_set_dc_clock(int clock)
{
	svc(SVC_SET_DC_CLOCK);
}

// Set AC port clock
__attribute__ ((noinline)) void svc_set_ac_clock(int clock)
{
	svc(SVC_SET_AC_CLOCK);
}

// Create and run new software timer 
__attribute__ ((noinline)) void svc_softtimer_run(int id, int timeout, int p1, int p2)
{
	svc(SVC_SOFTTIMER_RUN);
}

// Stop and remove software timer 
__attribute__ ((noinline)) void svc_softtimer_stop(int id)
{
	svc(SVC_SOFTTIMER_STOP);
}


// Allocate system memory block (for DMA or hardware IO)
__attribute__ ((noinline)) void* svc_malloc(int size)
{
	svc(SVC_MALLOC);
}

// Free system memory block 
__attribute__ ((noinline)) int svc_free(void* buf)
{
	svc(SVC_FREE);
}

// Transfer sequence of PWM over a DC port 
__attribute__ ((noinline)) int svc_dc_pwm_transfer(int dc_port, uint8_t* buf, uint32_t size, int cyclic)
{
	svc(SVC_DC_PWM_XFER);
}

// returns error or number of bytes read
__attribute__ ((noinline)) int svc_vault_get(char *var_name, void **data)
{
	svc(SVC_VAULT_GET);
}

// returns error or number of bytes written
__attribute__ ((noinline)) int svc_vault_set(char *var_name, void *data, int size, int offset)
{
	svc(SVC_VAULT_SET);
}

// returns error or number of vars deleted
__attribute__ ((noinline)) int svc_vault_del(char *var_name)
{
	svc(SVC_VAULT_DEL);
}

// Enumerates index list
__attribute__ ((noinline)) int svc_vault_enum(int idx, char *app_name, char *var_name, int* size)
{
	svc(SVC_VAULT_ENUM);
}

// RTC Time/Date API 
__attribute__ ((noinline)) int svc_get_time(int *Time_BIN)
{
	svc(SVC_GET_TIME);
}

__attribute__ ((noinline)) int svc_get_date(int *Date_BIN)
{
	svc(SVC_GET_DATE);
}

__attribute__ ((noinline)) int svc_set_time(int *Time_BIN)
{
	svc(SVC_SET_TIME);
}

__attribute__ ((noinline)) int svc_set_date(int *Date_BIN)
{
	svc(SVC_SET_DATE);
}

// Initialize GPIO and pull it up
__attribute__ ((noinline)) int svc_gpio_init_pullup(uint32_t RCC_AHB1Periph, void* GPIOx, unsigned short GPIO_Pin, unsigned short InOut)
{
	svc(SVC_GPIO_INIT_PULLUP);
}

// Initialize GPIO, no pull 
__attribute__ ((noinline)) int svc_gpio_init_nopull(uint32_t RCC_AHB1Periph, void* GPIOx, unsigned short GPIO_Pin, unsigned short InOut)
{
	svc(SVC_GPIO_INIT_NOPULL);
}

// Enable IRQ on GPIO, trigger on falling edge
__attribute__ ((noinline)) int svc_gpio_irq_fall(unsigned short PortSrc, unsigned short PortPin, unsigned short Line, unsigned short IrqChan)
{
	svc(SVC_GPIO_IRQ_FALL);
}

// Enable IRQ on GPIO, trigger on raising edge
__attribute__ ((noinline)) int svc_gpio_irq_rise(unsigned short PortSrc, unsigned short PortPin, unsigned short Line, unsigned short IrqChan)
{
	svc(SVC_GPIO_IRQ_RISE);
}

// Enable IRQ on GPIO, trigger both on falling and raising edge
__attribute__ ((noinline)) int svc_gpio_irq_risefall(unsigned short PortSrc, unsigned short PortPin, unsigned short Line, unsigned short IrqChan)
{
	svc(SVC_GPIO_IRQ_RISEFALL);
}

// Start SPI transfer  
__attribute__ ((noinline)) int svc_ext_spi_request(char* tx_buf, char* rx_buf, int len)
{
	svc(SVC_EXT_SPI_REQUEST);
}

// Stop SPI transfer  
__attribute__ ((noinline)) int svc_ext_spi_stop(void)
{
	svc(SVC_EXT_SPI_STOP);
}

// Enable/disable microsec timer

#pragma GCC diagnostic pop


#endif //_SVC_H_IMPL_
#endif //SVC_CLIENT_IMPL


