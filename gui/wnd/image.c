#include "wnd.h"
#include "utf8.h"
#include "utils.h"

int image_window_proc(WND* hwnd, int msg, int p1, int p2) {
        switch(msg) {

        case WM_CREATE: {
                //print("IMAGE::WM_CREATE\n");
		WINDOW->wnd_proc(hwnd, msg, p1, p2);
		hwnd->image = malloc(sizeof(IMAGE_DATA));
		memset(hwnd->image, 0, sizeof(IMAGE_DATA));

		wnd_reset_flags(hwnd, FLAG_FILLBG | FLAG_TOUCHES_ENABLED);
        } break;

	case WM_DESTROY: {
		if(hwnd->image) {
			free(hwnd->image);
			hwnd->image = NULL;
		}
		WINDOW->wnd_proc(hwnd, msg, p1, p2);
	} break;

	case WM_SET_IMAGE: {
		switch(p2) {
		case HIGHLIGHTED:
			hwnd->image->highlighted_bitmap = (Bitmap*) p1;
			break;
		case NORMAL:
			hwnd->image->bitmap = (Bitmap*) p1;
			break;
		}
	} break;

        case WM_SIZE_THAT_FITS: {
                if(p1==NULL) break;
                Size *size=(Size*) p1;

		Bitmap* bitmap=hwnd->image->bitmap;
                if(wnd_get_flags(hwnd, FLAG_HIGHLIGHTED)) { 
                        if(hwnd->image->highlighted_bitmap) {
                                bitmap = hwnd->image->highlighted_bitmap;
                        }
                }

                if(bitmap) {
                        size->cx = bitmap->width;
                        size->cy = bitmap->height;
                }
        } break;


        case WM_DRAW_RECT: {
                Context * gc=(Context*)p1;
                if(gc == NULL) break;

                if(wnd_get_flags(hwnd, FLAG_FILLBG)) gc_fill_rect(gc, 0, 0, RECT_WIDTH(hwnd->frame), RECT_HEIGHT(hwnd->frame), hwnd->bgcolor);

		Bitmap* bitmap=hwnd->image->bitmap;
		if(wnd_get_flags(hwnd, FLAG_HIGHLIGHTED)) {
			if(hwnd->image->highlighted_bitmap) {
				bitmap = hwnd->image->highlighted_bitmap;
			}
		}

                if(bitmap) gc_draw_bitmap_pixels(gc, bitmap, 0, 0, bitmap->width, bitmap->height, 0, 0, SRC_PAINT);
        } break;


        default: {
                return WINDOW->wnd_proc(hwnd, msg, p1, p2);
        } break;

        }//switch

        return 0;
}

