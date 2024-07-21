#pragma once
#include <webgpu/webgpu.hpp>

#include "math.hpp"

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <span>
#include <vector>
#include <string_view>

#ifdef __cpp_exceptions
#include <stdexcept>
#endif

#ifdef WAYLIB_NAMESPACE_NAME
namespace WAYLIB_NAMESPACE_NAME {
#endif

constexpr static WGPUTextureFormat depth_texture_format = wgpu::TextureFormat::Depth24Plus;

#include "common.h"

struct pipeline_globals {
	bool created = false;
	size_t min_buffer_size;
	// Group 0 is Instance/"PBR" Texture Data
	// Group 1 is Utility: Camera (B0) / Light (B1) / Time (B2) Data
	std::array<WGPUBindGroupLayout, 2> bindGroupLayouts;
	wgpu::PipelineLayout layout;
};

#ifndef WAYLIB_NO_CAMERAS
	// Camera, defines position/orientation in 3d space
	// From: raylib.h
	struct camera3D {
		WAYLIB_CAMERA_3D_MEMBERS
	};

	// Camera2D, defines position/orientation in 2d space
	// From: raylib.h
	struct camera2D {
		WAYLIB_CAMERA_2D_MEMBERS
	};

	typedef struct camera_upload_data {
		WAYLIB_CAMERA_UPLOAD_DATA_MEMBERS

	#ifdef WAYLIB_ENABLE_CLASSES
		mat4x4f& get_view_matrix() { return *(mat4x4f*)&view_matrix; }
		mat4x4f& get_projection_matrix() { return *(mat4x4f*)&projection_matrix; }
	#endif
	} camera_upload_data;
#endif // WAYLIB_NO_CAMERAS

#ifndef WAYLIB_NO_LIGHTS
	struct light {
		WAYLIB_LIGHT_MEMBERS
	};
#endif // WAYLIB_NO_LIGHTS

template<size_t extent = std::dynamic_extent>
union image_data_span {
	std::span<color8, extent> data;
	std::span<color, extent> data32;
};

// TODO: Is there an alternative to std::function we can use?
struct wgpu_frame_finalizers: public std::vector<std::function<void()>> { using std::vector<std::function<void()>>::vector; };
#define WAYLIB_RELEASE_AT_FRAME_END(frame, variable) frame.finalizers->emplace_back([variable]() mutable { variable.release(); })
#define WAYLIB_RELEASE_BUFFER_AT_FRAME_END(frame, buffer) frame.finalizers->emplace_back([buffer]() mutable { buffer.destroy(); buffer.release(); })



std::string get_error_message_and_clear();
#ifdef __cpp_exceptions
	struct error: public std::runtime_error {
		using std::runtime_error::runtime_error;
	};

	template<typename T>
	T throw_if_null(const WAYLIB_OPTIONAL(T)& opt) {
		if(opt.has_value) return opt.value;

		auto msg = get_error_message_and_clear();
		throw error(msg.empty() ? "Null value encountered" : msg);
	}
	template<typename T>
	T& throw_if_null(const WAYLIB_NULLABLE(T*) opt) {
		if(opt) return *opt;

		auto msg = get_error_message_and_clear();
		throw error(msg.empty() ? "Null value encountered" : msg);
	}
#else
	template<typename T>
	T throw_if_null(const WAYLIB_OPTIONAL(T)& opt) {
		assert(opt.has_value, get_error_message());
		return opt.value;
	}
	template<typename T>
	T& throw_if_null(const WAYLIB_NULLABLE(T*) opt) {
		assert(opt, get_error_message());
		return *opt;
	}
#endif
void set_error_message(const std::string_view view);
void set_error_message(const std::string& str);

// Creates a c-string from a string view 
// (if the string view doesn't point to a valid cstring a temporary one that 
//		is only valid until the next time this function is called is created)
template<size_t uniqueID = 0>
inline const char* cstring_from_view(const std::string_view view) {
	static std::string tmp;
	if(view.data()[view.size()] == '\0') return view.data();
	tmp = view;
	return tmp.c_str();
}

color convert(const color8& c);
color8 convert(const color& c);

// Not recommended to be used directly!
pipeline_globals& create_pipeline_globals(wgpu_state state);

WAYLIB_NULLABLE(image_data_span<>) image_get_frame(
	image& image, 
	size_t frame, 
	size_t mip_level
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= 0
#endif
);

WAYLIB_OPTIONAL(image) merge_images(
	std::span<image> images, 
	bool free_incoming
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
);

#ifdef WAYLIB_NAMESPACE_NAME
}
#endif