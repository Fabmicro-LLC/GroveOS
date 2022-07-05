#define _CRT_SECURE_NO_WARNINGS 1

#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include <gdiplus.h>
#include "resource.h"
#include <string>
#include <sstream>
#include <strsafe.h>

#pragma warning(disable:4995)

#define RECT_HEIGHT(rect)	(rect.bottom - rect.top)
#define RECT_WIDTH(rect)	(rect.right - rect.left)

static WNDPROC		mOldEditWindowProc=NULL;
static HINSTANCE g_hInstance=NULL;;

Gdiplus::GdiplusStartupInput	gdiplusStartupInput;
ULONG_PTR			gdiplusToken=0;
   
static Gdiplus::Bitmap* g_bitmap=NULL;

static char buf[200];

static std::string symbols_info;
static std::string symbols_data;
static int symbol_data_index=0;

static LRESULT editWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void centerWithParent(HWND hwnd, HWND parent) {
	if(hwnd==NULL) {
		return;
	}
	
	if(parent==NULL) {
		parent=GetDesktopWindow();
	}

	RECT rect;
	RECT rectParent;
	
	GetWindowRect(hwnd, &rect); 
	GetWindowRect(parent, &rectParent); 
	
	int w=RECT_WIDTH(rect);
	int h=RECT_HEIGHT(rect);
	int x= rectParent.left + (RECT_WIDTH(rectParent) - RECT_WIDTH(rect))/2;
	int y= rectParent.top + (RECT_HEIGHT(rectParent) - RECT_HEIGHT(rect))/2;
	
	RECT workArea;
	SystemParametersInfo(SPI_GETWORKAREA,0,(LPVOID)&workArea,0);

	if(x + w > workArea.right) {
		x = workArea.right - w;
	} else if(x<workArea.left) {
		x = workArea.left;
	}

	if(y + h > workArea.bottom ) {
		y = workArea.bottom - h;
	} else if (y<workArea.top ) {
		y = workArea.top;
	}
		
	MoveWindow(hwnd, x, y, w, h,TRUE); 
}

void convert_symbol(HWND hDlg, HDC hdc, wchar_t c) {
	RECT r={0,0,200,200};		
	wchar_t symbol[2]={0};
	symbol[0]=c;
	DrawText(hdc, symbol, 1, &r, DT_CALCRECT );
	DrawText(hdc, symbol, 1, &r, 0);
	
	sprintf(buf, "\t{0x%x, %d, %d},\r\n", c,RECT_WIDTH(r),symbol_data_index);
	symbols_info.append(buf);

	std::string s="\t";
	sprintf(buf, "0x%x", c);
	s.append("//symbol ").append(buf).append("\r\n");

	for(int y=r.top; y<r.bottom; y++) {
		s.append("\t");
		for(int x=r.left; x<r.right; x++) {
			COLORREF bgr = GetPixel(hdc, x, y);
			BYTE alpha = (BYTE) (bgr >> 16);
			sprintf(buf, "0x%02x,", alpha);
			s.append(buf);
			symbol_data_index ++;
		}
		s.append("\r\n");
	}

	symbols_data.append(s);
	
}

BOOL CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	
	switch (message){
	
	case WM_INITDIALOG: {
		Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

		centerWithParent(hDlg,GetParent(hDlg));
		SetDlgItemText(hDlg,IDC_FONTNAME, L"");
		EnableWindow(GetDlgItem(hDlg, IDC_CONVERT), FALSE);

		mOldEditWindowProc = (WNDPROC) GetWindowLong(GetDlgItem(hDlg, IDC_EDIT1), GWL_WNDPROC);
		SetWindowLong(GetDlgItem(hDlg, IDC_EDIT1), GWL_WNDPROC, (LONG)editWindowProc);

		HFONT hFont=CreateFont(-12,0,0,0,0,0,0,0,0,0,0,0,0,TEXT("Courier New"));
		SendMessage(GetDlgItem(hDlg, IDC_EDIT1),WM_SETFONT,(WPARAM)hFont,0);

		SendMessage(GetDlgItem(hDlg, IDC_INCLUDE_ALPHA),BM_SETCHECK, BST_CHECKED, 0);
	} break;
	
	case WM_DESTROY: {
		SetWindowLong(GetDlgItem(hDlg, IDC_EDIT1), GWL_WNDPROC, (LONG)mOldEditWindowProc);
		return FALSE;
	} break;

	case WM_COMMAND: {
		if(wParam==IDC_CHOOSE_IMAGE) {
			OPENFILENAME ofn={0};       // common dialog box structure
			TCHAR szFile[260];       // buffer for file name

			// Initialize OPENFILENAME
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hDlg;
			ofn.lpstrFile = szFile;
			// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
			// use the contents of szFile to initialize itself.
			ofn.lpstrFile[0] = '\0';
			ofn.nMaxFile = sizeof(szFile);
			ofn.lpstrFilter = L"All\0*.*\0Image\0*.PNG;*.JPG\0";
			ofn.nFilterIndex = 2;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

			// Display the Open dialog box. 

			if (GetOpenFileName(&ofn)==TRUE) {
				SetDlgItemText(hDlg, IDC_FONTNAME, ofn.lpstrFile);
				EnableWindow(GetDlgItem(hDlg, IDC_CONVERT), TRUE);

				Gdiplus::Bitmap* img=Gdiplus::Bitmap::FromFile(ofn.lpstrFile);
				OutputDebugString(L"DRAW IMAGE::0\n");

				if(img ) {//&& img->GetLastStatus() == Gdiplus::Ok) {
					g_bitmap  = img;
					OutputDebugString(L"DRAW IMAGE::1\n");
					HDC hdc=GetDC(GetDlgItem(hDlg, IDC_PIC));
					Gdiplus::Graphics g(hdc);
					g.Clear(Gdiplus::Color(0xffffffff));
					g.DrawImage(g_bitmap,0,0,g_bitmap->GetWidth(), g_bitmap->GetHeight());
					ReleaseDC(hDlg, hdc);
				} else {
					delete img;
					img=NULL;
				}
			}


		} else if(wParam==IDC_CONVERT) {

			if(g_bitmap == NULL)  return FALSE;

			int Width = g_bitmap->GetWidth();
			int Height = g_bitmap->GetHeight();
			Gdiplus::BitmapData* bitmapData = new Gdiplus::BitmapData;
			Gdiplus::Rect rect(0, 0, Width, Height);

			g_bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB , bitmapData);
			UINT*  pixels = (UINT*)bitmapData->Scan0;
			INT iStride =abs(bitmapData->Stride);



			std::string rgb_data="";
			std::string alpha_data="";

			int counter = 0;
			
			for (int row = 0; row < Height; ++row) {
				for (int col = 0; col < Width; ++col) {

					unsigned int curColor = pixels[row * iStride / 4 + col];
					int b = curColor & 0xff;
					int g = (curColor & 0xff00) >> 8;
					int r = (curColor & 0xff0000) >> 16;
					int a = (curColor & 0xff000000) >> 24;
					
					int color = (((r >> 5) & 0x07) << 5) | (((g >> 5) & 0x07) << 2) | ((b >> 6) & 0x03);
					
					sprintf(buf, "0x%02x,", color);
					rgb_data.append(buf);
					
					sprintf(buf, "0x%02x,", a);
					alpha_data.append(buf);

					counter++;
					if(counter % 32 == 0) {
						rgb_data.append("\r\n");
						alpha_data.append("\r\n");
						if(counter % 256 == 0) {
							sprintf(buf, "//pixel #%d\r\n", counter);
							rgb_data.append(buf);
							alpha_data.append(buf);
						}
					}
			
				}
        	}

    		g_bitmap->UnlockBits(bitmapData);
			bool include_alpha = (SendMessage(GetDlgItem(hDlg, IDC_INCLUDE_ALPHA), BM_GETCHECK, 0, 0) == BST_CHECKED);
			if(!include_alpha) alpha_data = "";

			std::string s;
			sprintf(buf, 
				"#ifndef RESOURCE_IMPL\r\n\r\n"\
				"struct {\r\n"\
				"	unsigned short width;\r\n"\
				"	unsigned short height;\r\n"\
				"	unsigned char rgb_data[%d];\r\n"\
				"	unsigned char alpha_data[%d];\r\n"\
				"} imagename;\r\n\r\n"\
				"#else\r\n\r\n"\
				".imagename = {\r\n\r\n"\
				".width = %d,\r\n"\
				".height = %d,\r\n",
				Width*Height, Width*Height, Width, Height);
				
			s=buf;
			s+=".rgb_data = {\r\n"+rgb_data+"\r\n},\r\n\r\n"; 
			s+=".alpha_data = {\r\n"+alpha_data+"\r\n},\r\n\r\n";
			s+="},\r\n#endif";

			SetWindowTextA(GetDlgItem(hDlg, IDC_EDIT1), s.data());
			SendMessage(GetDlgItem(hDlg, IDC_EDIT1), EM_SETSEL, 0, s.length () );


			MessageBox(hDlg,L"Success", L"Convert completed",MB_OK);
		};
	} break;
	
	case WM_CLOSE: {
		DestroyWindow(hDlg);
	} break;
	
	default:
		return FALSE;
	}
	
	return TRUE;
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
	InitCommonControls(); 
	g_hInstance = hInstance;
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc); 
	return TRUE;
}

LRESULT editWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	//LOG("editWindowProc msg="+(int)msg);
	switch(msg) {
	
	case WM_CHAR: {
		if(wParam==1 && GetKeyState(VK_CONTROL)<0 ) { //ctrl-a
			SendMessage(hwnd, EM_SETSEL, 0 , -1);
			return 0; 
		}    
	} break;
		
	}
	return CallWindowProc(mOldEditWindowProc, hwnd, msg, wParam, lParam);
}
