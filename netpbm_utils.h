#ifndef IMAGE_VIEWER_NETPBM_UTILS_H
#define IMAGE_VIEWER_NETPBM_UTILS_H

#include <stdint.h>
#include <stddef.h>

#include "image_viewer_utils.h"

img_file_type detect_netpmb(const uint8_t *bytes, size_t bytes_count);
img_data load_ppm_from_memory(const uint8_t *bytes, size_t bytes_count);


#endif //IMAGE_VIEWER_NETPBM_UTILS_H
