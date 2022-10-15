/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/

#include "utils.h"
#include "hardware.h"
#include "oled.h"

#ifdef OLED_DATA_GPIO_PORT

extern const char font_oled_6x8[];

void OLED_DATA(uint32_t val)
{
	uint32_t old_val = OLED_DATA_GPIO_PORT->ODR; 
	old_val &= OLED_DATA_GPIO_MASK;
	old_val |= (val << OLED_DATA_GPIO_SHIFT);
	OLED_DATA_GPIO_PORT->ODR = old_val;
}


void OLED_WEG010032_CMD(uint8_t cmd)
{
	OLED_RS(0); // RS=L
	OLED_RW(0); // RW=L

        // Remember that data bus is PE11-PE14

        // Write high nibble of data
	OLED_DATA((cmd>> 4) & 0x0F);
	OLED_E(1);
        DelayLoopMicro(5);
	OLED_E(0)
        DelayLoopMicro(5);

        // Write low nibble of data
	OLED_DATA(cmd & 0x0F);
	OLED_E(1);
	DelayLoopMicro(5);
	OLED_E(0);
	OLED_RW(1);
        DelayLoopMicro(5);
}



void OLED_WEG010032_CMD_HIGH(uint8_t cmd)
{
	OLED_RS(0);
	OLED_RW(0);

        // Remember that data bus is PE11-PE14 

        // Write high nibble of data
	OLED_DATA((cmd>> 4) & 0x0F);
        DelayLoopMicro(5);
	OLED_E(0);
	OLED_RW(1);
        DelayLoopMicro(5);
}



void OLED_WEG010032_DATA(uint8_t data)
{
	OLED_RS(1);
	OLED_RW(0);

        // Remember that data bus is PE11-PE14 

        // Write high nibble of data
	OLED_DATA((data>> 4) & 0x0F);

	OLED_E(1);
	DelayLoopMicro(1);
	OLED_E(0);
	DelayLoopMicro(1);

        // Write low nibble of data
	OLED_DATA(data & 0x0F);

	OLED_E(1);
        DelayLoopMicro(1);
	OLED_E(0);
	OLED_RW(1);
        DelayLoopMicro(1);
}




void OLED_WEG010032_INIT(void)
{
/*
	// Disable everything
	OLED_DATA(0x02); // 4bit mode
	OLED_CS1(1);
	OLED_CS2(1);
	OLED_E(1);
	OLED_RS(0);
	OLED_RW(0);
        DelayLoop(200);

        // Enable both chips
	OLED_CS1(0);
	OLED_CS2(0);
	OLED_E(0);
        DelayLoop(200);
*/
        DelayLoop(200);
	OLED_DATA(0x02);
	OLED_CS1(1);
	OLED_CS2(1);
	OLED_E(1);
	OLED_RS(0);
	OLED_RW(0);
        DelayLoop(200);

	OLED_E(0);
	DelayLoop(5);



        // Sequence to switch to 4 bit mode
        OLED_WEG010032_CMD_HIGH(0x30); // now switch to 4bit mode
        DelayLoop(5);
        OLED_WEG010032_CMD_HIGH(0x30); // now switch to 4bit mode
        DelayLoop(5);
        OLED_WEG010032_CMD_HIGH(0x30); // now switch to 4bit mode
        DelayLoop(5);
        OLED_WEG010032_CMD_HIGH(0x20); // now switch to 4bit mode
        DelayLoop(5);

        OLED_WEG010032_CMD(0x28); // 2-line
        //OLED_WEG010032_CMD(0x20); // 1-line
        OLED_WEG010032_CMD(0x1f); // Graphics mode
        //OLED_WEG010032_CMD(0x17); // Character mode
        OLED_WEG010032_CMD(0x0c); // Display ON
        OLED_WEG010032_CMD(0x06);
        OLED_WEG010032_CMD(0x01); // Clean
        OLED_WEG010032_CMD(0x02); // Return home
        DelayLoop(20);

}

void OLED_WEG010032_CHAR(unsigned char c, int x)
{
        char charmap[6];

        memcpy(charmap, font_oled_6x8 + c*6, 6);

        for(int i=0; i<6; i++) {
                if(x >= 0 && x < OLED_LINE_WIDTH) // only visible graphics should be output
                        OLED_WEG010032_DATA(charmap[i]);
                x++;
        }
}



void OLED_WEG010032_CLEAR(void)
{
	OLED_CS1(0);
	OLED_CS2(0);
        OLED_WEG010032_CMD(0x01); // Clean
        OLED_WEG010032_CMD(0x02); // Return home
        DelayLoop(20);
}


void OLED_WEG010032_PRINT(uint16_t size, char *buf, int16_t x, int16_t y)
{

        if(y & 0x06 == 0) {
		OLED_CS1(0)
		OLED_CS2(1);
        } else if(y & 0x06 == 0x02) {
		OLED_CS1(1);
		OLED_CS2(0);
        } else {
		OLED_CS1(0);
		OLED_CS1(0);
        }

        if(x < 0 || x >= OLED_LINE_WIDTH) {
                OLED_WEG010032_CMD(0x80); // if X is out of range reset AC to 0
        } else {
                OLED_WEG010032_CMD(0x80 | (x & 0x007f)); // AC = X
        }
        OLED_WEG010032_CMD(0x40 | (y & 0x01)); // CGA = Y

        //OLED_WEG010032_CMD(0x08); // Display OFF
        for(int i = 0; i < size; i++) {
                OLED_WEG010032_CHAR(*buf++, x); // x - is a back reference
                x += OLED_FONT_WIDTH;
        }
        //OLED_WEG010032_CMD(0x0c); // Display ON
}



void OLED_WEG010032_BLIT(uint16_t size, char *buf, int16_t x, int16_t y)
{
        if(y & 0x06 == 0) {
		OLED_CS1(0)
		OLED_CS2(1);
        } else if(y & 0x06 == 0x02) {
		OLED_CS1(1);
		OLED_CS2(0);
        } else {
		OLED_CS1(0);
		OLED_CS1(0);
        }

        if(x < 0 || x >= OLED_LINE_WIDTH) {
                OLED_WEG010032_CMD(0x80); // if X is out of range reset AC to 0
        } else {
                OLED_WEG010032_CMD(0x80 | (x & 0x007f)); // AC = X
        }
        OLED_WEG010032_CMD(0x40 | (y & 0x01)); // CGA = Y

        //OLED_WEG010032_CMD(0x08); // Display OFF
        for(int i = 0; i < size; i++) {
                if(x >= 0 && x < OLED_LINE_WIDTH)
                        OLED_WEG010032_DATA(*buf++); // only visible data should be written
                x++;
        }
        //OLED_WEG010032_CMD(0x0c); // Display ON
}



int oled_init(void)
{
	print("oled_init() Inidializing hardware...\r\n");

	// Reset GPIO pins before init
	OLED_DATA(0x00); // 4bit mode
	OLED_E(0);
	OLED_RS(0);
	OLED_RW(0);
	OLED_CS1(1);
	OLED_CS2(1);

	gpio_init(OLED_E_PERIPH, GPIO_SPEED, OLED_E_GPIO_PORT, OLED_E_GPIO_PIN, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_NOPULL);
	gpio_init(OLED_RW_PERIPH, GPIO_SPEED, OLED_RW_GPIO_PORT, OLED_RW_GPIO_PIN, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_NOPULL);
	gpio_init(OLED_RS_PERIPH, GPIO_SPEED, OLED_RS_GPIO_PORT, OLED_RS_GPIO_PIN, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_NOPULL);
	gpio_init(OLED_CS1_PERIPH, GPIO_SPEED, OLED_CS1_GPIO_PORT, OLED_CS1_GPIO_PIN, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_NOPULL);
	gpio_init(OLED_CS2_PERIPH, GPIO_SPEED, OLED_CS2_GPIO_PORT, OLED_CS2_GPIO_PIN, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_NOPULL);
	gpio_init(OLED_D4_PERIPH, GPIO_SPEED, OLED_D4_GPIO_PORT, OLED_D4_GPIO_PIN, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_NOPULL);
	gpio_init(OLED_D5_PERIPH, GPIO_SPEED, OLED_D5_GPIO_PORT, OLED_D5_GPIO_PIN, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_NOPULL);
	gpio_init(OLED_D6_PERIPH, GPIO_SPEED, OLED_D6_GPIO_PORT, OLED_D6_GPIO_PIN, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_NOPULL);
	gpio_init(OLED_D7_PERIPH, GPIO_SPEED, OLED_D7_GPIO_PORT, OLED_D7_GPIO_PIN, GPIO_Mode_OUT, GPIO_OType_PP, GPIO_PuPd_NOPULL);

	print("oled_init() Inidializing OLED device...\r\n");
	OLED_WEG010032_INIT();
	print("oled_init() Done!\r\n");

	return 0;
}

#endif

