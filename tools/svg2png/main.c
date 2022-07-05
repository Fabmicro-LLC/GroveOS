#include <stdio.h>
#include <string.h>
#include <float.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

int main(int argc, char** argv) {

	char *input_file, *output_file;
        if(argc != 3) {
                fprintf(stderr, "Usage:\n\n%s input_file.svg output_file.png\n\n", argv[0]);
                return -1;
        }

        input_file = argv[1];
        output_file = argv[2];

	NSVGimage *image = NULL;
	NSVGrasterizer *rast = NULL;
	unsigned char* img = NULL;
	int w, h;

	fprintf(stderr, "parsing %s\n", input_file);
	image = nsvgParseFromFile(input_file, "px", 96.0f);
	if (image == NULL) {
		fprintf(stderr, "Could not open SVG image.\n");
		goto error;
	}
	w = (int)image->width;
	h = (int)image->height;

	rast = nsvgCreateRasterizer();
	if (rast == NULL) {
		fprintf(stderr, "Could not init rasterizer.\n");
		goto error;
	}

	img = malloc(w*h*4);
	if (img == NULL) {
		fprintf(stderr, "Could not alloc image buffer.\n");
		goto error;
	}

	fprintf(stderr, "rasterizing image %d x %d\n", w, h);
	nsvgRasterize(rast, image, 0,0,1, img, w, h, w*4);

	fprintf(stderr, "writing %s\n", output_file);
 	stbi_write_png(output_file, w, h, 4, img, w*4);

error:
	nsvgDeleteRasterizer(rast);
	nsvgDelete(image);

	return 0;
}
