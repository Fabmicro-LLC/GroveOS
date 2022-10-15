/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
/* HD44780 command set */
#define HD44780_CLEAR                 0x01    /* Clear display */
#define HD44780_RETURN                0x02    /* Return display to home */

#define HD44780_ENTRY_MODE            0x04    /* Set Cursor, Display move and direction */
#define HD44780_ENTRY_MODE_CURSOR_S   0x00
#define HD44780_ENTRY_MODE_BOTH_S     0x01
#define HD44780_ENTRY_MODE_DEC        0x00
#define HD44780_ENTRY_MODE_INC        0x02

#define HD44780_DISPLAY_CTRL          0x08    /* Set display and cursor mode */
#define HD44780_DISPLAY_CTRL_C_NORM   0x00
#define HD44780_DISPLAY_CTRL_C_BLINK  0x01
#define HD44780_DISPLAY_CTRL_C_OFF    0x00
#define HD44780_DISPLAY_CTRL_C_ON     0x02
#define HD44780_DISPLAY_CTRL_D_OFF    0x00
#define HD44780_DISPLAY_CTRL_D_ON     0x04

#define HD44780_DISPLAY_MOVE          0x10    /* Display and cursor move */
#define HD44780_DISPLAY_MOVE_LEFT     0x00
#define HD44780_DISPLAY_MOVE_RIGHT    0x04
#define HD44780_DISPLAY_MOVE_C        0x00
#define HD44780_DISPLAY_MOVE_D        0x08

#define HD44780_FUNCTION_SET          0x20    /* Set display function */
#define HD44780_FUNCTION_SET_DOT_5_8  0x00
#define HD44780_FUNCTION_SET_DOT_5_10 0x04
#define HD44780_FUNCTION_SET_1_LINE   0x00
#define HD44780_FUNCTION_SET_2_LINE   0x08
#define HD44780_FUNCTION_SET_4_BIT    0x00
#define HD44780_FUNCTION_SET_8_BIT    0x10

#define HD44780_SET_CGRAM_ADD         0x40    /* Set AC point to CGRAM */
#define HD44780_SET_DDRAM_ADD         0x80    /* Set AC point to DDRAM */

// HD44780 Read Status
#define HD44780_STATUS_AC_MASK        0x7F    /* AC Mask */
#define HD44780_STATUS_BUSY_MASK      0x80    /* Busy Mask */

#define HD44780_BUSY_CHECK            TRUE
#define HD44780_NOT_BUSY_CHECK        FALSE

