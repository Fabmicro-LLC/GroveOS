/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2017-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include "utils.h"
#include <stm32f4xx_usart.h>
#include <stm32f4xx_rtc.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "hardware.h"
#include <math.h>

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

extern volatile unsigned int uwTimingDelay;

char timestamp_string[32];

int debug_mode = DEBUG_TO_USART1_AND_SWD;

void RTC_init() {

  	RTC_InitTypeDef RTC_InitStructure;
	RTC_TimeTypeDef RTC_TimeStructure;
  	RTC_DateTypeDef RTC_DateStructure;
	RTC_AlarmTypeDef RTC_AlarmStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;

	//if (RTC_ReadBackupRegister(RTC_BKP_DR0) != BUILD_NUMBER || RTC_GetFlagStatus(RTC_FLAG_INITS) == 0) { // RTC was reset, needs initialization
	if (RTC_ReadBackupRegister(RTC_BKP_DR0) != BUILD_NUMBER ) { // RTC was reset, needs initialization
	//if (RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x1234 || RTC_GetFlagStatus(RTC_FLAG_INITS) == 0) { // RTC was reset, needs initialization

  		/* Enable the PWR clock */
	  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	  	/* Allow access to RTC */
  		PWR_BackupAccessCmd(ENABLE);

		//RCC_BackupResetCmd(ENABLE);
		//RCC_BackupResetCmd(DISABLE);

		RCC_LSEConfig(RCC_LSE_ON);

		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

		 /* Wait till LSE is ready */

		__disable_irq();
		uint32_t i;
		for(i = 0; i < 65536*1000; i++) { 
			if(RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET)
				break;
		}
		__enable_irq();

		if(i == 65536*1000) { 
			print("RTC_init() LSE never became ready, pls check clock crystal\r\n");
			return;
		}

		print("RTC_init() LSE ready\r\n");

		/* Enable the RTC Clock */
		RCC_RTCCLKCmd(ENABLE);

		/* Wait for RTC APB registers synchronisation */
		RTC_WaitForSynchro();
		print("RTC_init() RTC APB registers have been synced\r\n");

		/* Configure the RTC data register and RTC prescaler */

		/* ck_spre(1Hz) = RTCCLK(LSI) /(uwAsynchPrediv + 1)*(uwSynchPrediv + 1)*/
		RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
		RTC_InitStructure.RTC_SynchPrediv = 0xFF;
		RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
		RTC_Init(&RTC_InitStructure);

		/* Set default date: 2017-01-01 */
		// NOTE: Year is used by INITS flag to check whether RTC was previously initialized or not
		//RTC_DateStructure.RTC_Year = 0x18;
		//RTC_DateStructure.RTC_Month = RTC_Month_January;
		//RTC_DateStructure.RTC_Date = 0x0;
		//RTC_DateStructure.RTC_WeekDay = RTC_Weekday_Sunday;
		//RTC_SetDate(RTC_Format_BCD, &RTC_DateStructure);



		/* Configure the RTC Alarm A register */
		RTC_SetAlarm(RTC_Format_BCD, RTC_Alarm_A, &RTC_AlarmStructure);

		/* Enable RTC Alarm A Interrupt */
		RTC_ITConfig(RTC_IT_ALRA, ENABLE);

		/* Enable the alarm */
		RTC_AlarmCmd(RTC_Alarm_A, ENABLE);

		RTC_ClearFlag(RTC_FLAG_ALRAF);


		/* Indicator for the RTC configuration */
		RTC_WriteBackupRegister(RTC_BKP_DR0, BUILD_NUMBER);
	
  		PWR_BackupAccessCmd(DISABLE);

		print("RTC_init() Clock has been re-initialized because of memory loss\r\n");

	} else {
  		/* Enable the PWR clock */
	  	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

		/* Enable the RTC Clock */
		RCC_RTCCLKCmd(ENABLE);

		/* Wait for RTC APB registers synchronisation */
		RTC_WaitForSynchro();

		/* Clear the RTC Alarm Flag */
		RTC_ClearFlag(RTC_FLAG_ALRAF);

		/* Clear the EXTI Line 17 Pending bit (Connected internally to RTC Alarm) */
		EXTI_ClearITPendingBit(EXTI_Line17);

		print("RTC_init() Clock is OK\r\n");
	}

	/* RTC Alarm A Interrupt Configuration */
	/* EXTI configuration *********************************************************/
	EXTI_ClearITPendingBit(EXTI_Line17);
	EXTI_InitStructure.EXTI_Line = EXTI_Line17;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable the RTC Alarm Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);


}



void SWD_direct_puts(USART_TypeDef* USARTx, char *s, int len){

        for(int i = 0; i < len; i++) {
		ITM_SendChar( *s );
                s++;
        }
}


int swd_print(const char *format, ...) {
	va_list list;
	va_start(list, format);
	int len = vsnprintf(0, 0, format, list);
	char *s;
	s = (char *)malloc(len + 1 );
	vsprintf(s, format, list);
	SWD_direct_puts(USART_DEBUG, s, len);
	free(s);
	va_end(list);
	return len;
}

void nmea_add_crc(unsigned char *buf)
{
        int len = strlen(buf);
        int i;
        int crc = 0;

        for(i = 1; i < len; i++) crc ^= buf[i];

        sprintf(buf + len, "*%02X", crc);

}

#ifdef USART_EXT1
int nmea_print(const char *format, ...) {
	va_list list;
	va_start(list, format);
	int len = vsnprintf(0, 0, format, list);
	char *s;
	s = (char *)malloc(len + 1 + 4);
	vsprintf(s, format, list);
	nmea_add_crc(s);
	USART_direct_puts(USART_EXT1, s, len);
	USART_direct_puts(USART_EXT1, "\r\n", 2);
	free(s);
	va_end(list);
	return len;
}
#endif


char* gettimestamp(void) {
	RTC_TimeTypeDef t;
	RTC_DateTypeDef d;
  	RTC_GetTime(RTC_Format_BIN, &t);
  	RTC_GetDate(RTC_Format_BIN, &d);
  	sprintf(timestamp_string, "%0.4d-%0.2d-%0.2d %0.2d:%0.2d:%0.2d ", d.RTC_Year+2000, d.RTC_Month, d.RTC_Date, t.RTC_Hours, t.RTC_Minutes, t.RTC_Seconds);
	return timestamp_string;
}


int print(const char *format, ...) {

        va_list list;
        va_start(list, format);
        int len = vsnprintf(0, 0, format, list);
        char s[len+1];
        vsprintf(s, format, list);
        va_end(list);

	gettimestamp();

        switch(debug_mode) {
                case DEBUG_TO_NONE:
                        break;
                case DEBUG_TO_USART1:
                        _print("%s %s", timestamp_string, s);
                        break;
                case DEBUG_TO_SWD:
                        swd_print("%s %s", timestamp_string, s);
                        break;
                case DEBUG_TO_USART1_AND_SWD:
                default:
                        _print("\r%s %s", timestamp_string, s);
                        swd_print("\r%s %s", timestamp_string, s);
			break;
        }


        return len;
}



void Delay(uint32_t nTime) {
  uwTimingDelay = nTime;
  while(uwTimingDelay != 0);
}


void DelayLoopMicro(__IO uint32_t nTime) {
        uint32_t n=nTime*6;
        for(__IO int i=0; i<n; i++) ;
}

void DelayLoop(__IO uint32_t nTime) {
	uint32_t n=nTime*10000;
	for(__IO int i=0; i<n; i++) ;
}


void gpio_pinconfig(void* GPIOx, unsigned short GPIO_Pin, unsigned short afconfig)
{
	GPIO_PinAFConfig(GPIOx, GPIO_Pin, afconfig);
}

void gpio_init(uint32_t RCC_AHB1Periph, unsigned short GPIO_Speed, void* GPIOx, unsigned short GPIO_Pin, unsigned short InOut, unsigned short PushPull, unsigned short PullUpDown)
{
        GPIO_InitTypeDef GPIO_InitStructure;

	if(RCC_AHB1Periph)
        	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph, ENABLE);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin;
        GPIO_InitStructure.GPIO_Mode = InOut; //GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = PushPull; //GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed; //GPIO_Speed_50MHz; //GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = PullUpDown; // GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOx, &GPIO_InitStructure);
        GPIO_ResetBits(GPIOx,  GPIO_Pin);
}

void gpio_irq(unsigned short PortSrc, unsigned short PortPin, unsigned short Line, unsigned short IrqChan, unsigned short RaiseFall)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;

        SYSCFG_EXTILineConfig(PortSrc, PortPin);

        EXTI_InitStructure.EXTI_Line = Line;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = RaiseFall;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        NVIC_InitStructure.NVIC_IRQChannel = IrqChan;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

}

void gpio_irq_disable(unsigned short PortSrc, unsigned short PortPin, unsigned short Line, unsigned short IrqChan)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;

        SYSCFG_EXTILineConfig(PortSrc, PortPin);

        EXTI_InitStructure.EXTI_Line = Line;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_LineCmd = DISABLE;
        EXTI_Init(&EXTI_InitStructure);

        NVIC_InitStructure.NVIC_IRQChannel = IrqChan;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
        NVIC_Init(&NVIC_InitStructure);
}


void gpio_set(GPIO_TypeDef* GPIOx, unsigned short GPIO_Pin, int val)
{

	switch(val) {
		case 0:
			GPIOx->BSRRH = GPIO_Pin;
			break;

		case 1:
			GPIOx->BSRRL = GPIO_Pin;
			break;
		
		case 2:
			GPIOx->ODR ^= GPIO_Pin;
			break;

		default:
			break;
	}
}


void gpio_write(GPIO_TypeDef* GPIOx, int val)
{
	GPIOx->ODR = val;
}


void sos(const char* msg) {

	GPIO_ResetBits(LED_GPIO_PORT, LED_GPIO_PIN);
	DelayLoopMicro(200000);

	GPIO_SetBits(LED_GPIO_PORT, LED_GPIO_PIN);
	DelayLoopMicro(200000);

	_print("%s\r\n", msg);
}



int utf8_to_utf16 (unsigned short *u16string, int *u16string_size, unsigned char* utf8) {
        int len=strlen(utf8);
        unsigned char* buf=utf8;
        unsigned short* cbuf =u16string;

        int pos=0;
        int sumb=0;
        int more=-1;

        for (int i = 0; i < len; i++) {
                unsigned char b=buf[i];
                // Decode byte b as UTF-8, sumb collects incomplete chars
                if ((b & 0xc0) == 0x80) {                       // 10xxxxxx (continuation byte)
                        sumb = (sumb << 6) | (b & 0x3f) ;       // Add 6 bits to mSumb
                        if (--more == 0) {
                                cbuf[pos++]=(unsigned short) sumb;      // Add char to sbuf
                        }
                } else if ((b & 0x80) == 0x00) {                // 0xxxxxxx (yields 7 bits)
                        cbuf[pos++]=(unsigned short) b;
                } else if ((b & 0xe0) == 0xc0) {                // 110xxxxx (yields 5 bits)
                        sumb = b & 0x1f;
                        more = 1;                               // Expect 1 more byte
                } else if ((b & 0xf0) == 0xe0) {                // 1110xxxx (yields 4 bits)
                        sumb = b & 0x0f;
                        more = 2;                               // Expect 2 more bytes
                } else if ((b & 0xf8) == 0xf0) {                // 11110xxx (yields 3 bits)
                        sumb = b & 0x07;
                        more = 3;                               // Expect 3 more bytes
                } else if ((b & 0xfc) == 0xf8) {                // 111110xx (yields 2 bits)
                        sumb = b & 0x03;
                        more = 4;                               // Expect 4 more bytes
                } else  {       // 1111110x (yields 1 bit) //if ((b & 0xfe) == 0xfc)
                        sumb = b & 0x01;
                        more = 5;                               // Expect 5 more bytes
                }

		if(pos>= *u16string_size) break;
        }

	 *u16string_size = pos;
        return pos;
}

double xround(double x, unsigned int precision) {
	double n=1;
	switch(precision) {
	case 0:
		n=1;
		break;
	case 1:
		n=10;
		break;
	case 2:
		n=100;
		break;
	case 3:
		n=1000;
		break;
	case 4:
		n=10000;
		break;
	case 5:
		n=100000;
		break;
	case 6:
	default:
		n=1000000;
		break;
	}

        return round(x*n)/n;
}


double degmod180(double deg) {
        deg = fmod(deg, 360.0);
        if(deg > 180.0) deg -= 360.0;
        else if(deg < -180.0) deg += 360.0;

        return deg;
}

char* strpbrkn(char* string_begin, char* accept, char* string_end)
{
        int i;
        char *s, *a;

        s = string_begin;

        while(s <= string_end) {
                a = accept;
                while(*a != 0) {
                        if(*s == *a) {
                                return s;
			}
                        a++;
                }

                s++;
        }

        return NULL;
}

char* my_index(char* str, char c, int len)
{
        for(int i = 0; i < len; i++) {
                if(*str == c)
                        return str;
                str++;
        }
        return NULL;
}

char * strnstrn(char *s, const char *find, int slen)
{
        char c, sc;
        size_t len;

        if ((c = *find++) != '\0') {
                len = strlen(find);
                do {
                        do {
                                if (slen < 1 || (sc = *s) == '\0')
                                        return (NULL);
                                --slen;
                                ++s;
                        } while (sc != c);
                        if (len > slen)
                                return (NULL);
                } while (strncmp(s, find, len) != 0);
                s--;
        }
        return s;
}

char * strnarg(char *s, int argn, int slen)
{
        int i, curargn = 0;
        char *retarg;

        retarg = s;

        for(i = 0; i <= slen; i++) {
                if(*s == 0) {
                        if(curargn == argn)
                                return retarg;
                        else {
                                while(i < slen) {
					if(s[1] != 0)
						break;
					s++;
					i++;
				}
				curargn++;
                                retarg = s + 1;
                        }
                }
                s++;
        }

        return NULL;
}



int32_t write_to_backup_sram( uint8_t *data, uint16_t bytes, uint16_t offset ) {
  const uint16_t backup_size = 0x1000;
  uint8_t* base_addr = (uint8_t *) BKPSRAM_BASE;
  uint16_t i;
  if( bytes + offset > backup_size ) {
    /* ERROR : the last byte is outside the backup SRAM region */
    return -1;
  }
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
  /* disable backup domain write protection */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);   // set RCC->APB1ENR.pwren
  PWR_BackupAccessCmd(ENABLE);                          // set PWR->CR.dbp = 1;
  /** enable the backup regulator (used to maintain the backup SRAM content in
    * standby and Vbat modes).  NOTE : this bit is not reset when the device
    * wakes up from standby, system reset or power reset. You can check that
    * the backup regulator is ready on PWR->CSR.brr, see rm p144 */
  PWR_BackupRegulatorCmd(ENABLE);     // set PWR->CSR.bre = 1;
  for( i = 0; i < bytes; i++ ) {
    *(base_addr + offset + i) = *(data + i);
  }
  PWR_BackupAccessCmd(DISABLE);                     // reset PWR->CR.dbp = 0;
  return bytes;
}

void* get_backup_sram_ptr(uint16_t offset ) {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
  return (void*) (((uint8_t *) BKPSRAM_BASE) + offset);
}


int32_t read_from_backup_sram( uint8_t *data, uint16_t bytes, uint16_t offset ) {
  const uint16_t backup_size = 0x1000;
  uint8_t* base_addr = (uint8_t *) BKPSRAM_BASE;
  uint16_t i;
  if( bytes + offset >= backup_size ) {
    /* ERROR : the last byte is outside the backup SRAM region */
    return -1;
  }
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
  for( i = 0; i < bytes; i++ ) {
    *(data + i) = *(base_addr + offset + i);
  }
  return bytes;
}

int32_t write_to_backup_rtc( uint32_t *data, uint16_t bytes, uint16_t offset ) {
  const uint16_t backup_size = 80;
  volatile uint32_t* base_addr = &(RTC->BKP0R);
  uint16_t i;
  if( bytes + offset >= backup_size ) {
    /* ERROR : the last byte is outside the backup SRAM region */
    return -1;
  } else if( offset % 4 || bytes % 4 ) {
    /* ERROR: data start or num bytes are not word aligned */
    return -2;
  } else {
    bytes >>= 2;      /* divide by 4 because writing words */
  }
  /* disable backup domain write protection */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);   // set RCC->APB1ENR.pwren
  PWR_BackupAccessCmd(ENABLE);                          // set PWR->CR.dbp = 1;
  for( i = 0; i < bytes; i++ ) {
    *(base_addr + offset + i) = *(data + i);
  }
  PWR_BackupAccessCmd(DISABLE);                     // reset PWR->CR.dbp = 0;
  // consider also disabling the power peripherial?
  return bytes;
}

int32_t read_from_backup_rtc( uint32_t *data, uint16_t bytes, uint16_t offset ) {
  const uint16_t backup_size = 80;
  volatile uint32_t* base_addr = &(RTC->BKP0R);
  uint16_t i;
  if( bytes + offset >= backup_size ) {
    /* ERROR : the last byte is outside the backup SRAM region */
    return -1;
  } else if( offset % 4 || bytes % 4 ) {
    /* ERROR: data start or num bytes are not word aligned */
    return -2;
  } else {
    bytes >>= 2;      /* divide by 4 because writing words */
  }
  /* read should be 32 bit aligned */
  for( i = 0; i < bytes; i++ ) {
    *(data + i) = *(base_addr + offset + i);
  }
  return bytes;
}


void memset16(unsigned short* dst, unsigned short val, int count)
{
        int i;
        for(i = 0; i < count; i++)
                *dst++ = val;
}


uint16_t modbus_crc16(uint8_t *buf, uint16_t len)
{
  uint16_t crc = 0xFFFF;
  for (int pos = 0; pos < len; pos++)
  {
  crc ^= (uint16_t)buf[pos];    // XOR byte into least sig. byte of crc

  for (int i = 8; i != 0; i--) {    // Loop over each bit
    if ((crc & 0x0001) != 0) {      // If the LSB is set
      crc >>= 1;                    // Shift right and XOR 0xA001
      crc ^= 0xA001;
    }
    else                            // Else LSB is not set
      crc >>= 1;                    // Just shift right
    }
  }

  return crc;
}


void modbus_add_crc(uint8_t *buf, uint16_t len)
{
        uint16_t crc = modbus_crc16(buf, len);
        buf[len] = (crc & 0xff);
        buf[len+1] = (crc >> 8) & 0xff;
}


void print_hex(uint8_t *buf, uint16_t len)
{
        char hex[1024];
        hex[0] = 0;

        for(int i = 0; i < MIN(len, 1024/5); i++) {
                sprintf(hex + strlen(hex), "0x%02X ", buf[i]);
        }
        _print("%s\r\n", hex);
}


