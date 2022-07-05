#include <stdio.h>
#include <string.h>
#include <float.h>

#define NANOSVG_IMPLEMENTATION
#include "svg_parse.h"

#define SVGRAST_TO_EDGES_IMPLEMENTATION
#include "svg_rast_to_edges.h"


void nsvg2cstruct(NSVGimage * img, char* struct_name);

int main(int argc, char** argv)
{

	char *input_file;
	char *struct_name = "svg_struct_name";

	if(argc<2) {
		fprintf(stderr, "Usage:\n\n%s image.svg [svg_struct_name]\n\n", argv[0]);
                return -1;
	}

	if(argc>1) input_file = argv[1];
	if(argc>2) struct_name = argv[2];

	NSVGimage *image = nsvgParseFromFile(input_file, "px", 96.0f);
	if (image == NULL) {
		fprintf(stderr, "Could not parse file %s\n", input_file);
		goto error;
	}

	nsvgRasterizeToEdges(image, 0, 0, 1.0);

	nsvg2cstruct(image, struct_name);
error:
	nsvgDelete(image);
	return 0;
}

void nsvg2cstruct(NSVGimage* img, char* struct_name) {

	int shapes_len = 0;
	int edges_fill_count=0;
	int edges_stroke_count=0;

        NSVGshape* shape=img->shapes;
        while(shape != NULL) {
		edges_fill_count += shape->nedges_fill;
                edges_stroke_count += shape->nedges_stroke;
                shape = shape->next;
                shapes_len++;
        }


	printf("#ifndef RESOURCE_IMPL\r\n\r\n");
	printf("struct {\r\n");
        printf("\tfloat width;\r\n");
        printf("\tfloat height;\r\n");
        printf("\tint shapes_len;\r\n");
        printf("\tNSVGshape shapes[%d];\r\n", shapes_len);
        printf( "\tNSVGedge edges_fill[%d];\r\n", edges_fill_count);
        printf( "\tNSVGedge edges_stroke[%d];\r\n", edges_stroke_count);
	printf("} %s;\r\n\r\n", struct_name);
	printf( "#else\r\n\r\n");
	printf(".%s = {\r\n", struct_name);
	printf("\t.width = %f,\r\n", img->width);
	printf("\t.height = %f,\r\n", img->height);
	printf("\t.shapes_len = %d,\r\n", shapes_len);

	int edges_fill_offset = 0;
	int edges_stroke_offset = 0;
	int i = 0;

	shape=img->shapes;
        while(shape != NULL) {
		printf("\t.shapes[%d] = {\r\n", i);
		printf("\t\t.fill = {\r\n");
                printf("\t\t\t.type = 0x%x,\r\n", shape->fill.type);
                printf("\t\t\t.color = 0x%x,\r\n", shape->fill.color);
                printf("\t\t},\r\n");
                printf("\t\t.stroke = {\r\n");
                printf("\t\t\t.type = 0x%x,\r\n", shape->stroke.type);
                printf("\t\t\t.color = 0x%x,\r\n", shape->stroke.color);
                printf("\t\t},\r\n");
                printf("\t\t.opacity = %f,\r\n", shape->opacity);
                printf("\t\t.strokeWidth = %f,\r\n", shape->strokeWidth);
                printf("\t\t.fillRule = 0x%x,\r\n", shape->fillRule);
               	printf("\t\t.flags = 0x%x,\r\n", shape->flags);
                printf("\t\t.nedges_fill = %d,\r\n", shape->nedges_fill);
                printf("\t\t.nedges_stroke = %d,\r\n", shape->nedges_stroke);
		printf("\t\t.edges_fill = (NSVGedge*)%d, \r\n", edges_fill_offset);
		printf("\t\t.edges_stroke = (NSVGedge*)%d, \r\n", edges_stroke_offset);
		printf("\t},\r\n");	

		edges_fill_offset += shape->nedges_fill;
		edges_stroke_offset += shape->nedges_stroke;

		//fprintf(stderr, "shape=%d, nedges_stroke=%d, edges_stroke_offset =%d\n", i, shape->nedges_stroke, edges_stroke_offset);
		//fprintf(stderr, "shape=%d, nedges_fill=%d, edges_fill_offset =%d\n", i, shape->nedges_fill, edges_fill_offset);

                shape = shape->next;
		i++;
	
	}

	printf("\t.edges_fill = {\r\n");	
	shape=img->shapes;
        while(shape != NULL) {
		for(int j=0; j < shape->nedges_fill; j++) {
                      NSVGedge edge = shape->edges_fill[j];
                      printf("\t\t{ .x0 = %f, .y0 = %f, .x1 = %f, .y1 = %f, .dir = %d,  },\r\n", edge.x0, edge.y0, edge.x1, edge.y1, edge.dir);
                }
		shape = shape->next;
	}
	printf("\t},\r\n");

	printf("\t.edges_stroke = {\r\n");        
	shape=img->shapes;
        while(shape != NULL) {
                for(int j=0; j < shape->nedges_stroke; j++) {
                      NSVGedge edge = shape->edges_stroke[j];
                      printf("\t\t{ .x0 = %f, .y0 = %f, .x1 = %f, .y1 = %f, .dir = %d,  },\r\n", edge.x0, edge.y0, edge.x1, edge.y1, edge.dir);
                }
		shape = shape->next;
        }
	printf("\t},\r\n\r\n");

	printf("},\r\n\r\n");
	printf("#endif\r\n");

}
