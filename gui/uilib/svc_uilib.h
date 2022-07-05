#ifndef _SVC_UILIB_H_
#define	_SVC_UILIB_H_

#ifdef SVC_CLIENT 

#include "wnd_common.h"
#include "nsvg.h"

void svc_set_root_window(WND* hwnd);
WND* svc_wnd_create(WNDCLASS* wnd_class);
void svc_wnd_destroy(WND* hwnd);
WNDCLASS* svc_wnd_get_class(const char* class_name);
WNDCLASS* svc_wnd_register_class(WNDCLASS *wnd_class);

Font* svc_get_font(int font_size);

NSVGfromEdgesRasterizer* svc_nsvg_rasterizer();
void svc_nsvg_draw(Context* gc, NSVGimage* svg, int x, int y);

WND* svc_show_calibrate_touch(int test);
WND* svc_show_input_text(INPUT_TEXT_PARAM* param, char* text);
WND* svc_show_input_num(INPUT_TEXT_PARAM* param);
WND* svc_show_input_num_format(INPUT_TEXT_PARAM* param, char* format);
WND* svc_show_select_list(INPUT_TEXT_PARAM *param, char** list, unsigned int list_len, unsigned int selected_index);
WND* svc_show_alert(char* text);
WND* svc_show_set_time();
WND* svc_show_adc_setup(int aps_format);

#endif //SVC_CLIENT
#endif //_SVC_UILIB_H_

#ifdef SVC_CLIENT_IMPL

#ifndef _SVC_UILIB_H_IMPL_
#define _SVC_UILIB_H_IMPL_

__attribute__ ((noinline)) void svc_set_root_window(WND* hwnd)
{
        svc(SVC_SET_ROOT_WINDOW);
}

__attribute__ ((noinline)) WND* svc_get_root_window() {
        svc(SVC_GET_ROOT_WINDOW);
}

__attribute__ ((noinline)) WND* svc_wnd_create(WNDCLASS* wnd_class) {
        svc(SVC_WND_CREATE);
}

__attribute__ ((noinline)) void svc_wnd_destroy(WND* hwnd) {
        svc(SVC_WND_DESTROY);
}

__attribute__ ((noinline)) WNDCLASS* svc_wnd_get_class(const char* class_name) {
        svc(SVC_WND_GET_CLASS);
}

__attribute__ ((noinline)) WNDCLASS* svc_wnd_register_class(WNDCLASS *wnd_class) {
        svc(SVC_WND_REGISTER_CLASS);
}

__attribute__ ((noinline)) Font* svc_get_font(int font_size) {
        svc(SVC_GET_FONT);
}


__attribute__ ((noinline)) NSVGfromEdgesRasterizer* svc_nsvg_rasterizer() {
        svc(SVC_NSVG_RASTERIZER);
}

__attribute__ ((noinline)) void svc_nsvg_draw(Context* gc, NSVGimage* svg, int x, int y) {
        svc(SVC_NSVG_DRAW);
}

__attribute__ ((noinline)) void svc_show_calibrate_touch(int test) {
        svc(SVC_SHOW_CALIBRATE_TOUCH);
}

__attribute__ ((noinline)) void svc_show_input_text(INPUT_TEXT_PARAM* param, char* text) {
        svc(SVC_SHOW_INPUT_TEXT);
}

__attribute__ ((noinline)) void svc_show_input_num(INPUT_TEXT_PARAM* param) {
        svc(SVC_SHOW_INPUT_NUM);
}

__attribute__ ((noinline)) void svc_show_input_num_format(INPUT_TEXT_PARAM* param, char* format) {
        svc(SVC_SHOW_INPUT_NUM_FORMAT);
}

__attribute__ ((noinline)) void svc_show_select_list(INPUT_TEXT_PARAM *param, char** list, unsigned int list_len, unsigned int selected_index) {
        svc(SVC_SHOW_SELECT_LIST);
}

__attribute__ ((noinline)) void svc_show_alert(char* text) {
        svc(SVC_SHOW_ALERT);
}

__attribute__ ((noinline)) void svc_show_set_time() {
        svc(SVC_SHOW_SET_TIME);
}

__attribute__ ((noinline)) void svc_show_adc_setup(int aps_format) {
        svc(SVC_SHOW_ADC_SETUP);
}

#endif //_SVC_UILIB_H_IMPL_
#endif //SVC_CLIENT_IMPL
