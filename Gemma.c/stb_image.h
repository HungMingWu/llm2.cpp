#ifndef GEMMA_STB_IMAGE_H
#define GEMMA_STB_IMAGE_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

unsigned char *stbi_load(const char *filename, int *x, int *y, int *channels,
                         int requested_channels);
unsigned char *stbi_load_from_file(FILE *file, int *x, int *y, int *channels,
                                  int requested_channels);
void stbi_image_free(void *data);
const char *stbi_failure_reason(void);

enum { STBIR_RGB = 3 };
enum { STBIR_TYPE_UINT8 = 0 };
enum { STBIR_EDGE_CLAMP = 0 };
enum { STBIR_FILTER_CATMULLROM = 4 };
void *stbir_resize(const void *input, int input_width, int input_height,
                   int input_stride, void *output, int output_width,
                   int output_height, int output_stride, int pixel_layout,
                   int data_type, int edge_mode, int filter);

#ifdef __cplusplus
}
#endif

#endif
