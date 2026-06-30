#ifndef IMAGE_VIEWER_IMAGE_VIEWER_UTILS_H
#define IMAGE_VIEWER_IMAGE_VIEWER_UTILS_H

#include <stdint.h>
#include <stdio.h>

typedef enum {
    PBM,
    PGM,
    PPM,
    UNSUPPORTED
}  img_file_type;

typedef struct {
    size_t len;
    img_file_type type;
    uint8_t bytes[255];
} img_file_headers;

#define NUMBER_OF_SUPPORTED_HEADERS 6
extern const img_file_headers headers[NUMBER_OF_SUPPORTED_HEADERS];

typedef struct {
    uint8_t r, g, b;
} rgb;

typedef struct {
    int64_t width, height;
    rgb *data;

} img_data;

uint8_t *read_file_bytes(FILE* fd, long *file_len);
img_file_type determine_file_type(const uint8_t *bytes, size_t bytes_count);

#endif //IMAGE_VIEWER_IMAGE_VIEWER_UTILS_H
