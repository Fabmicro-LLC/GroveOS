#ifndef _BITMAP_H_
#define _BITMAP_H_

typedef struct {
	int width;
	int height;
	unsigned char* data;//rgb
	int data_size;
	unsigned char* alpha_data;
	int alpha_size;
} Bitmap;

#endif
