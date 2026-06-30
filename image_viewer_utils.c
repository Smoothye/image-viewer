#include "image_viewer_utils.h"
#include "netpbm_utils.h"

#include <stdlib.h>
#include <string.h>

const img_file_headers headers[NUMBER_OF_SUPPORTED_HEADERS] = {
    {.len = 3, .type = PBM, .bytes = "P1\n"},
    {.len = 3, .type = PBM, .bytes = "P4\n"},
    {.len = 3, .type = PGM, .bytes = "P2\n"},
    {.len = 3, .type = PGM, .bytes = "P5\n"},
    {.len = 3, .type = PPM, .bytes = "P3\n"},
    {.len = 3, .type = PPM, .bytes = "P6\n"},
};

uint8_t *read_file_bytes(FILE* fd, long *file_len) {

    fseek(fd, 0, SEEK_END);
    *file_len = ftell(fd);
    if (*file_len < 1) {
        *file_len = 0;
        return NULL;
    }

    rewind(fd);

    uint8_t *bytes = malloc(*file_len * sizeof(uint8_t));
    if (!bytes) {
        fputs("Could not allocate memory for bytes buffer.\n", stderr);
        *file_len = 0;
        return NULL;
    }

    const size_t bytes_read =  fread(bytes, 1, *file_len, fd);
    if (bytes_read != (size_t)*file_len) {

        free(bytes);
        *file_len = 0;
        return NULL;
    }

    return bytes;
}

img_file_type determine_file_type(const uint8_t *bytes, const size_t bytes_count) {

    for (size_t idx = 0; idx < NUMBER_OF_SUPPORTED_HEADERS; ++idx) {

        if (headers[idx].len > bytes_count)
            continue;

        if (memcmp( bytes, headers[idx].bytes, headers[idx].len ) == 0)
            return headers[idx].type;
    }

    return UNSUPPORTED;
}