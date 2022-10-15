/*
        GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers.

	Written by Ruslan Zalata and Evgeny Korolenko.
        
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/

#include "svc.h"


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


// Save global config profile to NAND 
__attribute__ ((noinline)) int svc_save_config(void)
{
        svc(SVC_SAVE_CONFIG);
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
__attribute__ ((noinline)) int svc_read_sensor(int adc_channel, float* val)
{
        svc(SVC_READ_SENSOR);
}


// Read value of global configuration variable
__attribute__ ((noinline)) int svc_get_config(int conf_var_id, void* data)
{
        svc(SVC_READ_CONFIG);
}


// creates new file, return pointer to new TFS or NULL if no free space available
// CAUTION: If file exists it will be deleted and a new file with same name created
__attribute__ ((noinline)) TFS_HEADER* svc_tfs_create(char *name, int name_len, int file_size)
{
	svc(SVC_TFS_CREATE);
}

// writes a piece of data to this file, offset must be tracked by user.
// CAUTION: writing to same offset twice will get data corrupted!
__attribute__ ((noinline)) int svc_tfs_write(TFS_HEADER* tfs, char* block, int block_size, int offset)
{
	svc(SVC_TFS_WRITE);
}

// calculates and sets CRC16 for this file, called after file is fully written. Returns CRC16.
__attribute__ ((noinline)) int svc_tfs_close(TFS_HEADER* tfs)
{
	svc(SVC_TFS_CLOSE);
}

// finds and returns TFS of an existing file by null terminated string.
__attribute__ ((noinline)) TFS_HEADER* svc_tfs_find(char *name)
{
	svc(SVC_TFS_FIND);
}

// finds next file following provided TFS. To find first file use tfs=NULL
__attribute__ ((noinline)) TFS_HEADER* svc_tfs_find_next(TFS_HEADER* tfs)
{
	svc(SVC_TFS_FIND_NEXT);
}

// returns max size of file that is possible to write to file system
__attribute__ ((noinline)) int svc_tfs_get_free(void)
{
	svc(SVC_TFS_GET_FREE);
}

// returns tfs pointing to beginnign of system
__attribute__ ((noinline)) TFS_HEADER* svc_tfs_get_begin(void)
{
	svc(SVC_TFS_GET_BEGIN);
}

// Erase all data on flash, including application own text. After this call the only feasible call is svc_ctop()
__attribute__ ((noinline)) void svc_tfs_format(void)
{
	svc(SVC_TFS_FORMAT);
}

// Play PCMU audio file.
__attribute__ ((noinline)) int svc_tfs_aplay(char *name)
{
	svc(SVC_TFS_APLAY);
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

// Set DC port PWM value
__attribute__ ((noinline)) void svc_set_dc_pwm(int dc_port_num, float pwm)
{
	svc(SVC_SET_DC_PWM);
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


// Modbus EXT1 operations
__attribute__ ((noinline)) int svc_modbus1_enqueue_request(MODBUS_REQUEST* modbus_req)
{
	svc(SVC_MODBUS1_ENQUEUE_REQUEST);
}

__attribute__ ((noinline)) int svc_modbus1_register_responder(MODBUS_RESPONSE* modbus_resp)
{
	svc(SVC_MODBUS1_REGISTER_RESPONDER);
}

__attribute__ ((noinline)) int svc_modbus1_unregister_responder(MODBUS_RESPONSE* modbus_resp)
{
	svc(SVC_MODBUS1_UNREGISTER_RESPONDER);
}

__attribute__ ((noinline)) int svc_modbus1_submit_response(MODBUS_RESPONSE* modbus_resp)
{
	svc(SVC_MODBUS1_SUBMIT_RESPONSE);
}

// Modbus EXT2 operations
__attribute__ ((noinline)) int svc_modbus2_enqueue_request(MODBUS_REQUEST* modbus_req)
{
	svc(SVC_MODBUS2_ENQUEUE_REQUEST);
}

__attribute__ ((noinline)) int svc_modbus2_register_responder(MODBUS_RESPONSE* modbus_resp)
{
	svc(SVC_MODBUS2_REGISTER_RESPONDER);
}

__attribute__ ((noinline)) int svc_modbus2_unregister_responder(MODBUS_RESPONSE* modbus_resp)
{
	svc(SVC_MODBUS2_UNREGISTER_RESPONDER);
}

__attribute__ ((noinline)) int svc_modbus2_submit_response(MODBUS_RESPONSE* modbus_resp)
{
	svc(SVC_MODBUS2_SUBMIT_RESPONSE);
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

// Get root window (screen)
__attribute__ ((noinline)) WND* svc_get_screen()
{
        svc(SVC_GET_SCREEN);
}

__attribute__ ((noinline)) WND* svc_wnd_create(WNDCLASS* wnd_class) {
        svc(SVC_WND_CREATE);
}

__attribute__ ((noinline)) void svc_wnd_destroy(WND* hwnd) {
        svc(SVC_WND_DESTROY);
}

__attribute__ ((noinline)) void svc_wnd_add_subview(WND* superview, WND* subview) {
        svc(SVC_WND_ADD_SUBVIEW);
}
__attribute__ ((noinline)) void svc_wnd_remove_from_superview(WND* view) {
        svc(SVC_WND_REMOVE_FROM_SUPERVIEW);
}

__attribute__ ((noinline)) WND* svc_wnd_with_tag(WND* hwnd, int tag) {
        svc(SVC_WND_WITH_TAG);
}

__attribute__ ((noinline)) Size svc_wnd_get_text_size(Font* font, char* text) {
        svc(SVC_WND_GET_TEXT_SIZE);
}

__attribute__ ((noinline)) WNDCLASS* svc_wnd_get_class(const char* class_name) {
        svc(SVC_WND_GET_CLASS);
}

__attribute__ ((noinline)) WNDCLASS* svc_wnd_register_class(WNDCLASS *wnd_class) {
	svc(SVC_WND_REGISTER_CLASS);
}

__attribute__ ((noinline)) int svc_wnd_draw_text(Context* gc, const char* s, int len, Font *font, Color rgb, Rect* rect, int cursor_pos) {
	WND_DRAW_TEXT_STRUCT bb = {
		.gc = gc, 
		.s = s,
		.len = len,
		.font = font,
		.rgb = rgb,
		.rect = rect,
		.cursor_pos = cursor_pos,
	};

	return svc_wnd_draw_text_struct(&bb);
}

__attribute__ ((noinline)) int svc_wnd_draw_text_struct(WND_DRAW_TEXT_STRUCT* bb) {
	svc(SVC_WND_DRAW_TEXT_STRUCT);
}

Point svc_wnd_touch2screen_pt(int touch_x, int touch_y) {
	svc(SVC_WND_TOUCH2SCREEN_PT);
}

__attribute__ ((noinline)) Font* svc_get_font(int font_size) {
        svc(SVC_GET_FONT);
}



__attribute__ ((noinline)) unsigned char* svc_get_user_config() {
        svc(SVC_GET_USER_CONFIG);
}

__attribute__ ((noinline)) unsigned short svc_crc16(const unsigned char *buf, int len) {
        svc(SVC_CRC16);
}

__attribute__ ((noinline)) int svc_post_message_irq(int message,  int unique, int p1, int p2) {
        svc(SVC_POST_MESSAGE_IRQ);
}

__attribute__ ((noinline)) int svc_save_user_config() {
        svc(SVC_SAVE_USER_CONFIG);
}

__attribute__ ((noinline)) WND* svc_show_calibrate_touch(int test) {
	svc(SVC_SHOW_CALIBRATE_TOUCH);
}

__attribute__ ((noinline)) int svc_get_event_logging() {
	svc(SVC_GET_EVENT_LOGGING);
}

__attribute__ ((noinline)) int svc_logger_write_data(uint8_t* data, unsigned int data_len) {
	svc(SVC_LOGGER_WRITE_DATA);
}

__attribute__ ((noinline)) int svc_logger_erase_current_sector() {
	svc(SVC_LOGGER_ERASE_CURRENT_SECTOR);
}

__attribute__ ((noinline)) struct LOGGER_ITEM* svc_logger_get_previous_item(struct LOGGER_ITEM* item) {
	svc(SVC_LOGGER_GET_PREVIOUS_ITEM);
}

__attribute__ ((noinline)) struct LOGGER_ITEM* svc_logger_get_current_item(struct LOGGER_ITEM* item) {
	svc(SVC_LOGGER_GET_CURRENT_ITEM);
}


