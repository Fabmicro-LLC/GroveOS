#include <stdio.h>
#include <string.h>
#include <float.h>

#define strtoll strtol

//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"
#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

int convert(char *input_file) {
	NSVGimage *image = NULL;
	NSVGrasterizer *rast = NULL;
	unsigned char* img = NULL;
	int w, h;

	//printf("parsing %s\n", input_file);
	image = nsvgParseFromFile(input_file, "px", 96.0f);
	if (image == NULL) {
		//printf("Could not open SVG image.\n");
		goto error;
	}
	w = (int)image->width;
	h = (int)image->height;

	rast = nsvgCreateRasterizer();
	if (rast == NULL) {
		//printf("Could not init rasterizer.\n");
		goto error;
	}

	img = malloc(w*h*4);
	if (img == NULL) {
		//printf("Could not alloc image buffer.\n");
		goto error;
	}

	//printf("rasterizing image %d x %d\n", w, h);
	nsvgRasterize(rast, image, 0,0,1, img, w, h, w*4);

error:
	nsvgDeleteRasterizer(rast);
	nsvgDelete(image);

	return 0;
}
