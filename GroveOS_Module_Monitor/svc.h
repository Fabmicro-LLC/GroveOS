/*
        GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers.

	Written by Ruslan Zalata and Evgeny Korolenko.
        
	Copyright (c) 2017-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/

#ifndef __SVC_H__
#define	__SVC_H__

#include <stdint.h> 
#include "tfs.h"
#include "modbus_common.h"
#include "wnd_common.h"

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
#define	SVC_SET_DC_PWM		43	// Write PWM value for DC port 
#define	SVC_SET_AC_PWM		44	// Write PWM value for AC port 
#define	SVC_READ_DC_PWM		45	// Read PWM value for DC port 
#define	SVC_READ_AC_PWM		46	// Read PWM value for AC port 
#define	SVC_SET_DC_CLOCK	47	// Write CLOCK value for DC port 
#define	SVC_SET_AC_CLOCK	48	// Write CLOCK value for AC port 
#define	SVC_SET_CONFIG		49	// Set global configuration variable
#define	SVC_READ_CONFIG		40	// Read global configuration variable
#define	SVC_SAVE_CONFIG		42	// Save global config profile to NAND 

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

#define SVC_GET_SCREEN          100
#define SVC_WND_CREATE          101
#define SVC_WND_DESTROY         102
#define SVC_WND_ADD_SUBVIEW     103
#define SVC_WND_REMOVE_FROM_SUPERVIEW   104
#define SVC_WND_WITH_TAG        105
#define SVC_WND_GET_TEXT_SIZE   106
#define SVC_WND_GET_CLASS       107
#define SVC_GET_FONT		108
#define SVC_WND_REGISTER_CLASS	109
#define SVC_WND_DRAW_TEXT_STRUCT 110
#define SVC_WND_TOUCH2SCREEN_PT 111

#define SVC_GET_USER_CONFIG     130
#define SVC_CRC16               131
#define SVC_POST_MESSAGE_IRQ    133
#define SVC_SAVE_USER_CONFIG	134
#define SVC_SHOW_CALIBRATE_TOUCH 135
#define SVC_GET_EVENT_LOGGING	136
#define SVC_LOGGER_WRITE_DATA	138
#define SVC_LOGGER_ERASE_CURRENT_SECTOR 139
#define SVC_LOGGER_GET_PREVIOUS_ITEM 140
#define SVC_LOGGER_GET_CURRENT_ITEM 141
#define	SVC_SET_ADC_COEFF	142	// ADC voltage conversion coefficient
#define	SVC_SET_ADC_OFFSET	143	// ADC voltage offset 
#define	SVC_GET_ADC_COEFF	144	// ADC voltage conversion coefficient
#define	SVC_GET_ADC_OFFSET	145	// ADC voltage offset 
#define	SVC_READ_ADC		146	// Read raw value of ADC channel
#define	SVC_READ_SENSOR		147	// Read RMS value of ADC channel



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
void svc_set_dc_pwm(int dc_port_num, float pwm);
void svc_set_ac_pwm(int ac_port_num, float pwm);
void svc_set_dc_clock(int clock);
void svc_set_ac_clock(int clock);
float svc_read_dc_pwm(int dc_port_num);
float svc_read_ac_pwm(int ac_port_num);
void svc_softtimer_run(int id, int timeout, int p1, int p2);
void svc_softtimer_stop(int id);
int svc_save_config(void);
int svc_set_config(int conf_var_id, void* data);
int svc_get_config(int conf_var_id, void* data);
// ADC
int svc_set_adc_coeff(int adc_channel, float val);
int svc_set_adc_offset(int adc_channel, float val);
int svc_get_adc_coeff(int adc_channel, float* val);
int svc_get_adc_offset(int adc_channel, float* val);
int svc_read_adc(int adc_channel, int* val); // read raw value of ADC 
int svc_read_sensor(int adc_channel, float* val); // read RMS value of ADC channel

// Below API provides a simple file system to be deployed on NAND flash
// 1. Files a contiguous, means there is on one large data block for each file of a variable size
// 2. Files cannot be removed, but can be marked as deleted
// 3. File size is defined upon creation.
// 4. Files cannot be appended. To append data one has to copy file to a new one with a bigger creation size


TFS_HEADER* svc_tfs_create(char *name, int name_len, int file_size); // creates new file, return pointer to new TFS or NULL if no free space available
								     // CAUTION: If file exists it will be deleted and a new file with same name created

int svc_tfs_write(TFS_HEADER* tfs, char* block, int block_size, int offset); // writes a piece of data to this file, offset must be tracked by user.
                                                                             // CAUTION: writing to same offset twice will get data corrupted!

int svc_tfs_close(TFS_HEADER* tfs); // calculates and sets CRC16 for this file, called after file is fully written. Returns CRC16.

TFS_HEADER* svc_tfs_find(char *name); // finds and returns TFS of an existing file by null terminated string.

TFS_HEADER* svc_tfs_find_next(TFS_HEADER* tfs); // finds next file following provided TFS. To find first file use tfs=NULL

TFS_HEADER* svc_tfs_get_begin(void); // Returns TFS pointing to beginning of file system 

int svc_tfs_get_free(void); // returns max size of file that is possible to write to file system

void svc_tfs_format(void); // Erases all data on Flash including application's own code. The only feasible call after this is svc_close(), otherwise Exception!

int svc_tfs_aplay(char *name); // Play PCMU audio file.



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


// Modbus EXT1 stuff 
int svc_modbus1_enqueue_request(MODBUS_REQUEST* modbus_req);
int svc_modbus1_register_responder(MODBUS_RESPONSE* modbus_resp);
int svc_modbus1_unregister_responder(MODBUS_RESPONSE* modbus_resp);
int svc_modbus1_submit_response(MODBUS_RESPONSE* modbus_resp);

// Modbus EXT2 stuff 
int svc_modbus2_enqueue_request(MODBUS_REQUEST* modbus_req);
int svc_modbus2_register_responder(MODBUS_RESPONSE* modbus_resp);
int svc_modbus2_unregister_responder(MODBUS_RESPONSE* modbus_resp);
int svc_modbus2_submit_response(MODBUS_RESPONSE* modbus_resp);

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


// Windows API for GUI

typedef struct {
	Context* gc; const char* s; int len; Font *font; Color rgb; Rect* rect; int cursor_pos;
} WND_DRAW_TEXT_STRUCT;

WND* svc_get_screen();
WND* svc_wnd_create(WNDCLASS* wnd_class);
void svc_wnd_destroy(WND* hwnd);
void svc_wnd_add_subview(WND* superview, WND* subview);
void svc_wnd_remove_from_superview(WND* view);
WND* svc_wnd_with_tag(WND* hwnd, int tag);
Size svc_wnd_get_text_size(Font* font, char* text);
WNDCLASS* svc_wnd_get_class(const char* class_name);
WNDCLASS* svc_wnd_register_class(WNDCLASS *wnd_class);
int svc_wnd_draw_text(Context* gc, const char* s, int len, Font *font, Color rgb, Rect* rect, int cursor_pos);
int svc_wnd_draw_text_struct(WND_DRAW_TEXT_STRUCT *bb);
Point svc_wnd_touch2screen_pt(int touch_x, int touch_y);

Font* svc_get_font(int font_size);

//
unsigned char* svc_get_user_config();
int svc_save_user_config();
unsigned short svc_crc16(const unsigned char *buf, int len);
int svc_post_message_irq(int message,  int unique, int p1, int p2);

WND* svc_show_calibrate_touch(int test);
int svc_get_event_logging();

void svc_set_adc_update_proc(void* proc);


// Logger API
int svc_logger_write_data(uint8_t* data, unsigned int data_len);
int svc_logger_erase_current_sector();
struct LOGGER_ITEM* svc_logger_get_previous_item(struct LOGGER_ITEM* item);
struct LOGGER_ITEM* svc_logger_get_current_item();


#endif


