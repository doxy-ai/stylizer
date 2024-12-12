#ifndef STYLIZER_WINDOW_IS_AVAILABLE
#define STYLIZER_WINDOW_IS_AVAILABLE

#ifndef __cplusplus
	#include <GLFW/glfw3.h>
#endif

#include "stylizer/core/core.h"

typedef struct STYLIZER_PREFIXED(window_config) { // TODO: Fill out optional parameters flag
	bool resizable STYLIZER_DEFAULT_PARAMETER(true);
	bool visible STYLIZER_DEFAULT_PARAMETER(true);
	bool decorated STYLIZER_DEFAULT_PARAMETER(true);
	bool focused STYLIZER_DEFAULT_PARAMETER(true);
	bool auto_iconify STYLIZER_DEFAULT_PARAMETER(true);
	bool floating STYLIZER_DEFAULT_PARAMETER(false);
	bool maximized STYLIZER_DEFAULT_PARAMETER(false);
	bool center_cursor STYLIZER_DEFAULT_PARAMETER(true);
	bool transparent_framebuffer STYLIZER_DEFAULT_PARAMETER(false);
	bool focus_on_show STYLIZER_DEFAULT_PARAMETER(true);
	bool scale_to_monitor STYLIZER_DEFAULT_PARAMETER(false);

	int red_bits STYLIZER_DEFAULT_PARAMETER(8);
	int green_bits STYLIZER_DEFAULT_PARAMETER(8);
	int blue_bits STYLIZER_DEFAULT_PARAMETER(8);
	int alpha_bits STYLIZER_DEFAULT_PARAMETER(8);
	int depth_bits STYLIZER_DEFAULT_PARAMETER(24);
	int stencil_bits STYLIZER_DEFAULT_PARAMETER(8);
	int accum_red_bits STYLIZER_DEFAULT_PARAMETER(0);
	int accum_green_bits STYLIZER_DEFAULT_PARAMETER(0);
	int accum_blue_bits STYLIZER_DEFAULT_PARAMETER(0);
	int accum_alpha_bits STYLIZER_DEFAULT_PARAMETER(0);

	int aux_buffers STYLIZER_DEFAULT_PARAMETER(0);
	int samples STYLIZER_DEFAULT_PARAMETER(0);
	int refresh_rate STYLIZER_DEFAULT_PARAMETER(-1); // Aka Don't Care!
	bool stereo STYLIZER_DEFAULT_PARAMETER(false);
	bool srgb_capable STYLIZER_DEFAULT_PARAMETER(true);
	bool double_buffer STYLIZER_DEFAULT_PARAMETER(true);
} STYLIZER_PREFIXED(window_config);

#endif // STYLIZER_WINDOW_IS_AVAILABLE