#pragma once
#include "waylib.h"

#ifdef __cplusplus
extern "C" {
#else
#include "GLFW/glfw3.h"
#endif

#ifdef __cplusplus
using window = GLFWwindow;
using monitor = GLFWmonitor;
#else
struct GLFWwindow;
typedef GLFWwindow window;
struct GLFWmonitor;
typedef GLFWmonitor monitor;
#endif

typedef struct fullscreen_window_initialization_configuration {
	; monitor* monitor
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	; bool auto_minimize_when_focus_lost
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	; bool forcibly_center_cursor
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	;
} fullscreen_window_initialization_configuration;

typedef struct window_initialization_configuration {
	; bool user_resizable
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool initially_visible
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool initially_focused
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool initially_maximized
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	; bool decorated
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; bool always_on_top
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	; bool transparent_framebuffer
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	; bool focus_on_show
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	; fullscreen_window_initialization_configuration fullscreen;
} window_initialization_configuration;



monitor* get_primary_monitor();

void close_all_windows();

bool poll_all_window_events();

window_initialization_configuration default_window_initialization_configuration();

window* create_window(
	size_t width,
	size_t height,
	const char* title
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= "waylib"
#endif
	, window_initialization_configuration config
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= default_window_initialization_configuration()
#endif
);

void release_window(
	window* window
);

bool window_should_close(
	window* window,
	bool should_poll_events
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
);

WAYLIB_C_OR_CPP_TYPE(WGPUSurface, wgpu::Surface) window_get_surface(window* window, WGPUInstance instance);

webgpu_state window_get_webgpu_state(window* window, WGPUDevice device);

bool window_configure_surface(
	window* window, 
	webgpu_state state, 
	WGPUPresentMode present_mode
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= wgpu::PresentMode::Mailbox
#endif  
	, WGPUCompositeAlphaMode alpha_mode
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= wgpu::CompositeAlphaMode::Auto
#endif
);

void window_automatically_reconfigure_surface_on_resize(
	window* window, 
	webgpu_state state, 
	WGPUPresentMode present_mode
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= wgpu::PresentMode::Mailbox
#endif  
	, WGPUCompositeAlphaMode alpha_mode
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= wgpu::CompositeAlphaMode::Auto
#endif
	, bool configure_now
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
);

webgpu_state create_default_device_from_window(
	window* window, bool prefer_low_power
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
);

#ifdef __cplusplus
} // End extern "C"
#endif