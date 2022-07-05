#include "wnd.h"
#include "utils.h"

int button_window_proc(WND* hwnd, int msg, int p1, int p2) {
        switch(msg) {

        case WM_CREATE: {
                //print("BUTTON::WM_CREATE\n");
		WINDOW->wnd_proc(hwnd, msg, p1, p2);

		hwnd->button =  malloc(sizeof(BUTTON_DATA));
		memset(hwnd->button, 0, sizeof(BUTTON_DATA));

		hwnd->button->background=wnd_create(IMAGE);
		wnd_add_subview(hwnd, hwnd->button->background);

        	hwnd->button->icon=wnd_create(IMAGE);
		wnd_add_subview(hwnd, hwnd->button->icon);

        	hwnd->button->label=wnd_create(LABEL);
		hwnd->button->label->label->highlighted_text_color = WHITE;
		hwnd->button->label->label->alignment = center;
		wnd_add_subview(hwnd, hwnd->button->label);

        	SetRect( &hwnd->button->margins, 5,5,5,5);
		hwnd->button->inner_space = 5;
		
		hwnd->bgcolor = GREY;
        } break;

	case WM_DESTROY: {
		if(hwnd->button) {
			free(hwnd->button);
			hwnd->button = NULL;
		}
		WINDOW->wnd_proc(hwnd, msg, p1, p2);
	} break;

	case WM_SET_TEXT: {
		wnd_proc_call(hwnd->button->label, msg, p1, p2);
        } break;

	case WM_GET_TEXT: {
		return wnd_proc_call(hwnd->button->label, msg, p1, p2);
	} break;

	case WM_SET_IMAGE: {
		switch(p2) {
                case NORMAL: 
			hwnd->button->icon->image->bitmap = (Bitmap*) p1;
			break;
                case HIGHLIGHTED:
			hwnd->button->icon->image->highlighted_bitmap = (Bitmap*) p1;
			break;
		case BACKGROUND:
			hwnd->button->background->image->bitmap = (Bitmap*) p1;
			break;
		case BACKGROUND_HIGHLIGHTED:
			hwnd->button->background->image->highlighted_bitmap = (Bitmap*) p1;
			break;
		}
        } break;

	case WM_SET_FONT: {
		hwnd->button->label->label->font = (Font*) p1;
        } break;

	case WM_SET_TEXT_ALIGNMENT: {
		hwnd->button->label->label->alignment = (TextAlignment) p1;
	} break;

	case WM_SET_TEXT_COLOR: {
		wnd_proc_call(hwnd->button->label, msg, p1, p2);
	} break;

	case WM_DRAW_RECT: {
                Context * gc=(Context*)p1;
                if(gc == NULL) break;
		
		//print("BUTTON::WM_DRAW_RECT\n");
                if(wnd_get_flags(hwnd, FLAG_CUSTOM) == 0) {
			if(wnd_get_flags(hwnd, FLAG_HIGHLIGHTED) == 0) { //hwnd->button->state == 0) {
				gc_fill_rect(gc, 0, 0, RECT_WIDTH(hwnd->frame), RECT_HEIGHT(hwnd->frame), hwnd->bgcolor);
				gc_draw_line(gc, 0, 0, RECT_WIDTH(hwnd->frame)-1, 0, WHITE);
				gc_draw_line(gc, 0, 0, 0, RECT_HEIGHT(hwnd->frame)-1, WHITE);

				gc_draw_line(gc,RECT_WIDTH(hwnd->frame)-2, 1, RECT_WIDTH(hwnd->frame)-2, RECT_HEIGHT(hwnd->frame)-2, GREY_DARK);
                        	gc_draw_line(gc,1, RECT_HEIGHT(hwnd->frame)-2, RECT_WIDTH(hwnd->frame)-1, RECT_HEIGHT(hwnd->frame)-2, GREY_DARK);

				gc_draw_line(gc,RECT_WIDTH(hwnd->frame)-1, 0, RECT_WIDTH(hwnd->frame)-1, RECT_HEIGHT(hwnd->frame)-1, BLACK);
				gc_draw_line(gc,0, RECT_HEIGHT(hwnd->frame)-1, RECT_WIDTH(hwnd->frame), RECT_HEIGHT(hwnd->frame)-1, BLACK);
			} else {
				gc_fill_rect(gc, 0, 0, RECT_WIDTH(hwnd->frame), RECT_HEIGHT(hwnd->frame), GREY_DARK);

                        	gc_draw_line(gc, 0, 0, RECT_WIDTH(hwnd->frame)-1, 0, BLACK);
                        	gc_draw_line(gc, 0, 0, 0, RECT_HEIGHT(hwnd->frame)-1, BLACK);
                        	gc_draw_line(gc,RECT_WIDTH(hwnd->frame)-1, 0, RECT_WIDTH(hwnd->frame)-1, RECT_HEIGHT(hwnd->frame)-1, BLACK);
                        	gc_draw_line(gc,0, RECT_HEIGHT(hwnd->frame)-1, RECT_WIDTH(hwnd->frame)-1, RECT_HEIGHT(hwnd->frame)-1, BLACK);
			}
		}
		//}
        } break;

	case WM_TOUCH_PRESSED: {
		wnd_set_flags(hwnd->button->label, FLAG_HIGHLIGHTED);
		wnd_set_flags(hwnd->button->background, FLAG_HIGHLIGHTED);
		wnd_set_flags(hwnd->button->icon, FLAG_HIGHLIGHTED);
		wnd_set_flags(hwnd, FLAG_HIGHLIGHTED);
	} break;

	case WM_TOUCH_RELEASED: {
		wnd_reset_flags(hwnd->button->label, FLAG_HIGHLIGHTED);
		wnd_reset_flags(hwnd->button->background, FLAG_HIGHLIGHTED);
		wnd_reset_flags(hwnd->button->icon, FLAG_HIGHLIGHTED);
		wnd_reset_flags(hwnd, FLAG_HIGHLIGHTED);
		if(hwnd->superview) {
			if(hwnd->tag) {
				wnd_proc_call(hwnd->superview, WM_COMMAND,  hwnd->tag, (int)hwnd);
			}
		}
	} break;

	case WM_TOUCH_CANCELLED: {
		wnd_reset_flags(hwnd->button->label, FLAG_HIGHLIGHTED);
                wnd_reset_flags(hwnd->button->background, FLAG_HIGHLIGHTED);
                wnd_reset_flags(hwnd->button->icon, FLAG_HIGHLIGHTED);
                wnd_reset_flags(hwnd, FLAG_HIGHLIGHTED);
	} break;

	case WM_LAYOUT: {
		Size contentSize = MakeSize( RECT_WIDTH(hwnd->frame) - hwnd->button->margins.left - hwnd->button->margins.right, RECT_HEIGHT(hwnd->frame) - hwnd->button->margins.top - hwnd->button->margins.bottom);

        	if(contentSize.cx <=0 || contentSize.cy <= 0) {
                	hwnd->button->label->frame = MakeRect(0,0,0,0);
                	hwnd->button->icon->frame = MakeRect(0,0,0,0);
                	return 0;
        	}


        	Size iconSize=wnd_size_that_fits(hwnd->button->icon);
        	if(iconSize.cx > contentSize.cx) iconSize.cx = contentSize.cx;
        	if(iconSize.cy > contentSize.cy) iconSize.cy = contentSize.cy;

		Size labelSize=wnd_size_that_fits(hwnd->button->label);
		if(labelSize.cx > contentSize.cx) labelSize.cx = contentSize.cx;
		if(labelSize.cy > contentSize.cy) labelSize.cy = contentSize.cy;


        	Point iconOrigin={0};
        	Point labelOrigin={0};
        	Size innerSpace={0};
        	Size size={0};

        	//UIButtonIconPositionLeft
                if(iconSize.cx && labelSize.cx) innerSpace.cx = hwnd->button->inner_space;
                size.cx = iconSize.cx + innerSpace.cx + labelSize.cx;

		if(size.cx >= contentSize.cx) {
			iconOrigin.x = 0;
		} else {
			switch(hwnd->button->label->label->alignment) {
			case center:
                		iconOrigin.x = (contentSize.cx - size.cx)/2;
				break;
			case left:
				iconOrigin.x = 0;
				break;
			case right:
				iconOrigin.x = contentSize.cx - size.cx;
				break;
			}
		}

                iconOrigin.y = (contentSize.cy - iconSize.cy)/2;

                labelOrigin.x = iconOrigin.x + iconSize.cx + innerSpace.cx;
                labelOrigin.y = (contentSize.cy - labelSize.cy)/2;

                if(labelOrigin.x + labelSize.cx > contentSize.cx) {
                        labelSize.cx = contentSize.cx - labelOrigin.x;
                        if(labelSize.cx < 0 ) {
                                labelSize.cx = 0;
                        }
                }
		//end UIButtonIconPositionLeft

		hwnd->button->label->frame = MakeRect(hwnd->button->margins.left + labelOrigin.x, hwnd->button->margins.top + labelOrigin.y, labelSize.cx, labelSize.cy);
        	hwnd->button->icon->frame = MakeRect(hwnd->button->margins.left + iconOrigin.x, hwnd->button->margins.top + iconOrigin.y, iconSize.cx, iconSize.cy);
		hwnd->button->background->frame = MakeRect(0, 0, RECT_WIDTH(hwnd->frame), RECT_HEIGHT(hwnd->frame));

	} break;

        case WM_SIZE_THAT_FITS: {
		if(p1==NULL) break;
                Size *size=(Size*) p1;
	
		Size labelSize={0}; 
		wnd_proc_call(hwnd->button->label, WM_SIZE_THAT_FITS, (int)&labelSize, 0);
	
        	Size iconSize={0};
		wnd_proc_call(hwnd->button->icon, WM_SIZE_THAT_FITS, (int)&iconSize, 0);

        	Size backgroundSize={0};
		wnd_proc_call(hwnd->button->background, WM_SIZE_THAT_FITS, (int)&backgroundSize, 0);

        	Size innerSpace={0};

        	//switch(mIconPosition) {
        	//case UIButtonIconPositionLeft:
        	//case UIButtonIconPositionRight: {
                if(iconSize.cx && labelSize.cx) innerSpace.cx = hwnd->button->inner_space;
                size->cx = iconSize.cx + innerSpace.cx + labelSize.cx;
                size->cy = fmax(iconSize.cy, labelSize.cy);

        	//} break;
        	//}switch


        	size->cx += hwnd->button->margins.left + hwnd->button->margins.right;
        	size->cy += hwnd->button->margins.top + hwnd->button->margins.bottom;

        	size->cx = fmax(size->cx, backgroundSize.cx);
        	size->cy = fmax(size->cy, backgroundSize.cy);

		//return Pack(size.cx, size.cy);

        } break;

        default: {
                return WINDOW->wnd_proc(hwnd, msg, p1, p2);
        } break;
        }//switch

        return 0;

}

