#include "netpbm_utils.h"

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <asm-generic/errno-base.h>

#define NETPBM_FILE_TOKEN_MAX_SIZE 16

img_file_type detect_netpmb(const uint8_t *bytes, const size_t bytes_count) {

    if (!bytes || bytes_count < 3)
        return UNSUPPORTED;

    if (bytes[0] != 'P' || bytes[2] != '\n')
        return UNSUPPORTED;

    switch (bytes[1]) {
        case '1':
        case '4':
            return PBM;

        case '2':
        case '5':
            return PGM;

        case '3':
        case '6':
            return PPM;

        default:
            return UNSUPPORTED;
    }
}

static int read_netpbm_token(const uint8_t **bytes, const uint8_t *end, unsigned char *out) {

    for (; *bytes < end; ++(*bytes)) {

        if (isspace((unsigned char) **bytes))
            continue;

        // comment => read until end of line
        if (**bytes == '#') {

            while (*bytes < end && **bytes != '\n')
                ++(*bytes);

            if (*bytes >= end)
                return 0;

            continue;
        }

        // exit loop when encountering possible token
        break;
    }

    size_t k = 0;
    for (; *bytes < end; ++(*bytes)) {

        if (isspace((unsigned char) **bytes))
            break;

        // token is too long (-1 for terminator character '\0')
        if (k >= NETPBM_FILE_TOKEN_MAX_SIZE - 1)
            return 0;

        out[k++] = **bytes;
    }
    out[k] = '\0';

    return k != 0;
}

static int map_values_255(const uint32_t value, const uint32_t max) {

    return floor(255.0 / max * value);
}

img_data load_ppm_or_pgm_from_memory(const uint8_t *bytes, const size_t bytes_count) {

    const uint8_t *end = bytes + bytes_count;
    const img_data FAIL = { .width = 0, .height = 0, .data = NULL };

    unsigned char token[NETPBM_FILE_TOKEN_MAX_SIZE];

    if (!read_netpbm_token(&bytes, end, token))
        return FAIL;

    const bool is_ascii     = token[1] == '2' || token[1] == '3';
    const bool is_grayscale = token[1] == '2' || token[1] == '5';

    if (!read_netpbm_token(&bytes, end, token))
        return FAIL;

    errno = 0;
    const int64_t width = strtol((char *) token, NULL, 10);
    if (width <= 0 || errno == ERANGE)
        return FAIL;

    if (!read_netpbm_token(&bytes, end, token))
        return FAIL;

    errno = 0;
    const int64_t height = strtol((char *) token, NULL, 10);
    if (height <= 0 || errno == ERANGE)
        return FAIL;

    if (!read_netpbm_token(&bytes, end, token))
        return FAIL;

    errno = 0;
    const size_t max_color_value = strtol((char *) token, NULL, 10);
    if (max_color_value <= 0 || errno == ERANGE)
        return FAIL;

    ++bytes; // get rid of last newline before image data;

    const size_t size_of_colors_array = width * height;

    rgb *colors = malloc(size_of_colors_array * sizeof(rgb));
    if (!colors)
        return FAIL;

    rgb *colors_itr = colors;
    const rgb *colors_end = colors + size_of_colors_array;

    uint_fast8_t ctr = 0;
    while (bytes < end && colors_itr < colors_end) {

        uint32_t color_value = *bytes;

        if (is_ascii) {
            if (!read_netpbm_token(&bytes, end, token)) {
                free(colors);
                return FAIL;
            }

            errno = 0;
            color_value = strtol((char *) token, NULL, 10);
            if (errno == ERANGE) {
                free (colors);
                return FAIL;
            }
        }
        color_value = map_values_255(color_value, max_color_value);

        if (is_grayscale) {
            colors_itr->r = color_value;
            colors_itr->g = color_value;
            colors_itr->b = color_value;

            ++colors_itr;
        }
        else {

            ++ctr;
            if (ctr == 1)
                colors_itr->r = color_value;

            else if (ctr == 2)
                colors_itr->g = color_value;

            else {
                colors_itr->b = color_value;
                ++colors_itr;
                ctr = 0;
            }
        }

        ++bytes;
    }

    const img_data result = { .width = width, .height = height, .data = colors };
    return result;
}