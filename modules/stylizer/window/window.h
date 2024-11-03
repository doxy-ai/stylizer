#ifndef STYLIZER_WINDOW_IS_AVAILABLE
#define STYLIZER_WINDOW_IS_AVAILABLE

#ifndef __cplusplus
	#include <GLFW/glfw3.h>
#endif

#include "stylizer/core/core.h"

typedef struct STYLIZER_PREFIXED(window_config) { // TODO: Fill out optional parameters flag
	bool resizable
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool visible
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool decorated
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool focused
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool auto_iconify
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool floating
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	; bool maximized
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	; bool center_cursor
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool transparent_framebuffer
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	; bool focus_on_show
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool scale_to_monitor
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= false
#endif

	; int red_bits
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= 8
#endif
	; int green_bits
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= 8
#endif
	; int blue_bits
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= 8
#endif
	; int alpha_bits
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= 8
#endif
	; int depth_bits
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= 24
#endif
	; int stencil_bits
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= 8
#endif
	; int accum_red_bits
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= 0
#endif
	; int accum_green_bits
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= 0
#endif
	; int accum_blue_bits
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= 0
#endif
	; int accum_alpha_bits
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= 0
#endif

	; int aux_buffers
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= 0
#endif
	; int samples
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= 0
#endif
	; int refresh_rate
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= GLFW_DONT_CARE
#endif
	; bool stereo
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	; bool srgb_capable
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool double_buffer
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	;
} STYLIZER_PREFIXED(window_config);

// STYLIZER_PREFIXED(window)* STYLIZER_PREFIXED(create_window) (
// 	STYLIZER_PREFIXED(vec2u) size,
// 	const char* title
// #ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
// 		= "Stylizer"
// #endif
// 	, STYLIZER_PREFIXED(window_config) config
// #ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
// 		= {}
// #endif
// );

// void STYLIZER_PREFIXED(release_window) (
// 	STYLIZER_PREFIXED(window)* window
// );

// bool STYLIZER_PREFIXED(window_should_close)(
// 	STYLIZER_PREFIXED(window)* window,
// 	bool poll_events
// #ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
// 		= true
// #endif
// );

// vec2u STYLIZER_PREFIXED(window_get_dimensions)(
// 	STYLIZER_PREFIXED(window)* window
// );

// WGPUSurface STYLIZER_PREFIXED(window_get_surface)(
// 	STYLIZER_PREFIXED(window)* window,
// 	WGPUInstance instance
// );

// STYLIZER_OPTIONAL(STYLIZER_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)) STYLIZER_PREFIXED(window_create_default_state)(
// 	STYLIZER_PREFIXED(window)* window,
// 	bool prefer_low_power
// #ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
// 		= false
// #endif
// );

// bool STYLIZER_PREFIXED(window_configure_surface)(
// 	STYLIZER_PREFIXED(window)* window,
// 	STYLIZER_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)* state,
// 	STYLIZER_PREFIXED(surface_configuration) config
// #ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
// 		= {}
// #endif
// );

// bool STYLIZER_PREFIXED(window_reconfigure_surface_on_resize)(
// 	STYLIZER_PREFIXED(window)* window,
// 	STYLIZER_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)* state,
// 	STYLIZER_PREFIXED(surface_configuration) config
// #ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
// 		= {}
// #endif
// );

#endif // STYLIZER_WINDOW_IS_AVAILABLE