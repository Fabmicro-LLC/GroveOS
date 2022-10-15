/*
        GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers.

	Written by Ruslan Zalata and Evgeny Korolenko.
        
	Copyright (c) 2017-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/

#ifdef UI_IMPL

#ifndef _UI_H_IMPL_
#define _UI_H_IMPL_

#define PIXEL_FONT_IMPL
#define RECT_IMPL
#define UTF8_IMPL
#define GRAPHICS_IMPL

#endif
#endif

#include "uilib/pixel_font.h"
#include "uilib/rect.h"
#include "uilib/utf8.h"
#include "uilib/graphics.h"
#include "uilib/bitmap.h"
#include "uilib/wnd.h"
