#pragma once
#include "common.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <webgpu/webgpu.hpp>

#ifdef WAYLIB_NAMESPACE_NAME
namespace WAYLIB_NAMESPACE_NAME {
#endif

#include "window.h"

// extern bool glfwInitialized;

window* create_window(
	size_t width,
	size_t height,
	std::string_view title,
	window_initialization_configuration config
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= default_window_initialization_configuration()
#endif
);

wgpu::Surface window_get_surface(
	window* window,
	wgpu::Device webgpu
);

#ifndef WAYLIB_NO_CAMERAS
void window_begin_camera_mode3D(
	wgpu_frame_state& frame,
	window* window,
	camera3D& camera
);

void window_begin_camera_mode2D(
	wgpu_frame_state& frame,
	window* window,
	camera2D& camera
);

void window_begin_camera_mode_identity(
	wgpu_frame_state& frame,
	window* window
);
#endif // WAYLIB_NO_CAMERAS

#ifdef WAYLIB_NAMESPACE_NAME
}
#endif