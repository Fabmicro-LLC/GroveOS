#ifdef SVC_CLIENT_IMPL
	#define UILIB_IMPL
#endif

#ifdef UILIB_IMPL

#ifndef _UILIB_H_IMPL_
#define _UILIB_H_IMPL_

#define PIXEL_FONT_IMPL
#define RECT_IMPL
#define UTF8_IMPL
#define GRAPHICS_IMPL
#define WND_IMPL
#define SVGLIB_IMPL

#endif //_UILIB_H_IMPL_
#endif //UILIB_IMPL


#include "uilib/pixel_font.h"
#include "uilib/rect.h"
#include "uilib/utf8.h"
#include "uilib/graphics.h"
#include "uilib/bitmap.h"
#include "uilib/wnd.h"
#include "uilib/svglib.h"

