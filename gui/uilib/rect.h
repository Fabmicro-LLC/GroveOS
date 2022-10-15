/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#ifndef _RECT_H_
#define _RECT_H_

#include <stdint.h>

typedef int8_t BOOL;

#define FALSE 0
#define TRUE  1

#define SIZE(x) (sizeof(x)/sizeof(x[0]))

typedef struct {
        int16_t left;
        int16_t top;
        int16_t right;
        int16_t bottom;
} Rect;

#define RECT_HEIGHT(rect)       ((rect).bottom - (rect).top)
#define RECT_WIDTH(rect)        ((rect).right - (rect).left)

typedef union {
	struct {
		int16_t x;
		int16_t y;
	};
	uint32_t data;
} Point;

typedef union {
	struct {
		int16_t cx;
		int16_t cy;
	};
	uint32_t data;
} Size;

BOOL PtInRect(Rect *rect, int16_t x, int16_t y);
void SetRect(Rect *rect, int16_t left, int16_t top, int16_t right, int16_t bottom);
BOOL OffsetRect(Rect* rect, int16_t dx, int16_t dy);
BOOL IsRectEmpty(Rect *rect);
BOOL EqualRect(Rect *a, Rect *b);
BOOL IntersectRect(Rect *dst, const Rect* src1, const Rect* src2);
Rect Bounds(Rect *rect);
Rect ZeroRect();
Rect MakeRect(int16_t left, int16_t top, int16_t width, int16_t height);
Point MakePoint(int16_t x, int16_t y);
Size MakeSize(int16_t cx, int16_t cy);

#endif //_RECT_H_

#ifdef RECT_IMPL

#ifndef _RECT_H_IMPL_
#define _RECT_H_IMPL_

Point MakePoint(int16_t x, int16_t y) {
	Point pt;
	pt.x=x;
	pt.y=y;
	return pt;
}

Rect MakeRect(int16_t left, int16_t top, int16_t width, int16_t height) {
	Rect result;
	SetRect(&result, left, top, left+width, top+height);
	return result;
}

Rect ZeroRect() {
	Rect result = {0,0,0,0};
	return result;
}

BOOL PtInRect(Rect *rect, int16_t x, int16_t y) {
        return (rect && x>=rect->left && x<rect->right && y>=rect->top && y<rect->bottom);
}

void SetRect(Rect *rect, int16_t left, int16_t top, int16_t right, int16_t bottom) {
        if(rect) {
                rect->left=left;
                rect->top=top;
                rect->right=right;
                rect->bottom=bottom;
        }
}

BOOL OffsetRect(Rect* rect, int16_t dx, int16_t dy) {
        if(rect) {
                rect->left += dx;
                rect->right += dx;

                rect->top += dy;
                rect->bottom += dy;
        }
}

BOOL IsRectEmpty(Rect *rect) {
        return rect && (rect->right <= rect->left || rect->bottom <= rect->top);
}

BOOL EqualRect(Rect *a, Rect *b) {
        return (a && b && a->left == b->left && a->top == b->top && a->right == b->right && a->bottom == b->bottom);
}

BOOL IntersectRect(Rect *dst, const Rect* src1, const Rect* src2) {
        if(src1==0|| src2==0) {
                return FALSE;
        }

        Rect tmp={0};
	
        if(src1->left >= src2->left && src1->left <= src2->right) {
                tmp.left = src1->left;
        } else if(src2->left >= src1->left && src2->left <= src1->right) {
                tmp.left = src2->left;
        }

        if(src1->top >= src2->top && src1->top <= src2->bottom) {
                tmp.top = src1->top;
        } else if(src2->top >= src1->top && src2->top <= src1->bottom) {
                tmp.top = src2->top;
        }

        if(src1->right >= src2->left && src1->right <= src2->right) {
                tmp.right = src1->right;
        } else if(src2->right >= src1->left && src2->right <= src1->right) {
                tmp.right = src2->right;
        }

        if(src1->bottom >= src2->top && src1->bottom <= src2->bottom) {
                tmp.bottom = src1->bottom;
        } else if(src2->bottom >= src1->top && src2->bottom <= src1->bottom) {
                tmp.bottom = src2->bottom;
        }

        if(IsRectEmpty(&tmp)) {
                if(dst) SetRect(dst, 0, 0, 0, 0);
                return FALSE;
        } else {
                if(dst) *dst = tmp;
                return TRUE;
        }
}

Rect Bounds(Rect* rect) {
	Rect result;
	if(rect) {
		result.left = 0;
		result.top = 0;
		result.right = (rect->right - rect->left);
		result.bottom= (rect->bottom - rect->top);
	}
	return result;
}

Size MakeSize(int16_t cx, int16_t cy) {
	Size result;

	result.cx = cx;
	result.cy = cy;

	return result;
}

#endif // _RECT_H_IMPL_

#endif // RECT_IMPL
