#pragma once
#include "waylib.h"

#ifdef __cplusplus
extern "C" {
#endif

WAYLIB_OPTIONAL(image) load_image(
	const char * file_path
);

WAYLIB_OPTIONAL(image) load_image_from_memory(
	const unsigned char* data, size_t size
);

#ifdef __cplusplus
} // End extern "C"
#endif