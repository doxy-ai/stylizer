#ifndef WAYLIB_WINDOW_IS_AVAILABLE
#define WAYLIB_WINDOW_IS_AVAILABLE

#ifndef __cplusplus
	#include <GLFW/glfw3.h>
#endif

#include "waylib/core/core.h"

typedef struct WAYLIB_PREFIXED(window_config) { // TODO: Fill out optional parameters flag
	bool resizable
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool visible
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool decorated
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool focused
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool auto_iconify
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool floating
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	; bool maximized
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	; bool center_cursor
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool transparent_framebuffer
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	; bool focus_on_show
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool scale_to_monitor
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= false
#endif

	; int red_bits
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= 8
#endif
	; int green_bits
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= 8
#endif
	; int blue_bits
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= 8
#endif
	; int alpha_bits
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= 8
#endif
	; int depth_bits
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= 24
#endif
	; int stencil_bits
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= 8
#endif
	; int accum_red_bits
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= 0
#endif
	; int accum_green_bits
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= 0
#endif
	; int accum_blue_bits
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= 0
#endif
	; int accum_alpha_bits
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= 0
#endif

	; int aux_buffers
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= 0
#endif
	; int samples
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= 0
#endif
	; int refresh_rate
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= GLFW_DONT_CARE
#endif
	; bool stereo
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	; bool srgb_capable
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool double_buffer
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	;
} WAYLIB_PREFIXED(window_config);

// WAYLIB_PREFIXED(window)* WAYLIB_PREFIXED(create_window) (
// 	WAYLIB_PREFIXED(vec2u) size,
// 	const char* title
// #ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
// 		= "Waylib"
// #endif
// 	, WAYLIB_PREFIXED(window_config) config
// #ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
// 		= {}
// #endif
// );

// void WAYLIB_PREFIXED(release_window) (
// 	WAYLIB_PREFIXED(window)* window
// );

// bool WAYLIB_PREFIXED(window_should_close)(
// 	WAYLIB_PREFIXED(window)* window,
// 	bool poll_events
// #ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
// 		= true
// #endif
// );

// vec2u WAYLIB_PREFIXED(window_get_dimensions)(
// 	WAYLIB_PREFIXED(window)* window
// );

// WGPUSurface WAYLIB_PREFIXED(window_get_surface)(
// 	WAYLIB_PREFIXED(window)* window,
// 	WGPUInstance instance
// );

// WAYLIB_OPTIONAL(WAYLIB_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)) WAYLIB_PREFIXED(window_create_default_state)(
// 	WAYLIB_PREFIXED(window)* window,
// 	bool prefer_low_power
// #ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
// 		= false
// #endif
// );

// bool WAYLIB_PREFIXED(window_configure_surface)(
// 	WAYLIB_PREFIXED(window)* window,
// 	WAYLIB_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)* state,
// 	WAYLIB_PREFIXED(surface_configuration) config
// #ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
// 		= {}
// #endif
// );

// bool WAYLIB_PREFIXED(window_reconfigure_surface_on_resize)(
// 	WAYLIB_PREFIXED(window)* window,
// 	WAYLIB_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)* state,
// 	WAYLIB_PREFIXED(surface_configuration) config
// #ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
// 		= {}
// #endif
// );

#endif // WAYLIB_WINDOW_IS_AVAILABLE