#ifndef WAYLIB_INTERFACES_f9641d712fbea9630f1ce0a0613269d7
#define WAYLIB_INTERFACES_f9641d712fbea9630f1ce0a0613269d7

#ifndef __cplusplus
	#include "config.h"
	#include "optional.h"
	#include <stdint.h>

	#include <webgpu/webgpu.h>
#endif

#include "interfaces.allocatable.h"

//////////////////////////////////////////////////////////////////////
// # Math
//////////////////////////////////////////////////////////////////////

#ifndef WAYLIB_NO_SCALAR_ALIASES
	typedef uint8_t WAYLIB_PREFIXED(u8);
	typedef uint16_t WAYLIB_PREFIXED(u16);
	typedef uint32_t WAYLIB_PREFIXED(u32);
	typedef uint64_t WAYLIB_PREFIXED(u64);
	typedef int8_t WAYLIB_PREFIXED(i8);
	typedef int16_t WAYLIB_PREFIXED(i16);
	typedef int32_t WAYLIB_PREFIXED(i32);
	typedef int64_t WAYLIB_PREFIXED(i64);
	typedef float WAYLIB_PREFIXED(f32);
	typedef double WAYLIB_PREFIXED(f64);
	typedef uint32_t WAYLIB_PREFIXED(bool32);
#endif

typedef struct {
	uint32_t x, y;
} WAYLIB_PREFIXED_C_CPP_TYPE(vec2u, vec2uC);

#ifdef __cplusplus
	inline vec2uC toC(const vec2u& v) { return {v.x, v.y}; }
	inline vec2u fromC(const vec2uC& v) { return {v.x, v.y}; }
#endif

typedef struct {
	int32_t x, y;
} WAYLIB_PREFIXED_C_CPP_TYPE(vec2i, vec2iC);

#ifdef __cplusplus
	inline vec2iC toC(const vec2i& v) { return {v.x, v.y}; }
	inline vec2i fromC(const vec2iC& v) { return {v.x, v.y}; }
#endif

typedef struct {
	float x, y;
} WAYLIB_PREFIXED_C_CPP_TYPE(vec2f, vec2fC);

#ifdef __cplusplus
	inline vec2fC toC(const vec2f& v) { return {v.x, v.y}; }
	inline vec2f fromC(const vec2fC& v) { return {v.x, v.y}; }
#endif

typedef struct {
	uint32_t x, y, z;
} WAYLIB_PREFIXED_C_CPP_TYPE(vec3u, vec3uC);

#ifdef __cplusplus
	inline vec3uC toC(const vec3u& v) { return {v.x, v.y, v.x}; }
	inline vec3u fromC(const vec3uC& v) { return {v.x, v.y, v.x}; }
#endif

typedef struct {
	int32_t x, y, z;
} WAYLIB_PREFIXED_C_CPP_TYPE(vec3i, vec3iC);

#ifdef __cplusplus
	inline vec3iC toC(const vec3i& v) { return {v.x, v.y, v.x}; }
	inline vec3i fromC(const vec3iC& v) { return {v.x, v.y, v.x}; }
#endif

typedef struct {
	float x, y, z;
} WAYLIB_PREFIXED_C_CPP_TYPE(vec3f, vec3fC);

#ifdef __cplusplus
	inline vec3fC toC(const vec3f& v) { return {v.x, v.y, v.x}; }
	inline vec3f fromC(const vec3fC& v) { return {v.x, v.y, v.x}; }
#endif

typedef struct {
	uint32_t x, y, z, w;
} WAYLIB_PREFIXED_C_CPP_TYPE(vec4u, vec4uC);

#ifdef __cplusplus
	inline vec4uC toC(const vec4u& v) { return {v.x, v.y, v.z, v.w}; }
	inline vec4u fromC(const vec4uC& v) { return {v.x, v.y, v.z, v.w}; }
#endif

typedef struct {
	int32_t x, y, z, w;
} WAYLIB_PREFIXED_C_CPP_TYPE(vec4i, vec4iC);

#ifdef __cplusplus
	inline vec4iC toC(const vec4i& v) { return {v.x, v.y, v.z, v.w}; }
	inline vec4i fromC(const vec4iC& v) { return {v.x, v.y, v.z, v.w}; }
#endif

typedef struct {
	float x, y, z, w;
} WAYLIB_PREFIXED_C_CPP_TYPE(vec4f, vec4fC);

#ifdef __cplusplus
	inline vec4fC toC(const vec4f& v) { return {v.x, v.y, v.z, v.w}; }
	inline vec4f fromC(const vec4fC& v) { return {v.x, v.y, v.z, v.w}; }
#endif

typedef struct {
	float m0, m4, m8, m12;      // Matrix first row (4 components)
    float m1, m5, m9, m13;      // Matrix second row (4 components)
    float m2, m6, m10, m14;     // Matrix third row (4 components)
    float m3, m7, m11, m15;     // Matrix fourth row (4 components)
} WAYLIB_PREFIXED_C_CPP_TYPE(mat4x4f, mat4x4fC);

// #ifdef __cplusplus
// 	mat4x4fC toC(mat4x4f v) { return {v.x, v.y, v.z, v.w}; }
// 	mat4x4f fromC(mat4x4fC v) { return {v.x, v.y, v.z, v.w}; }
// #endif


//////////////////////////////////////////////////////////////////////
// # Opaque Types
//////////////////////////////////////////////////////////////////////


struct WAYLIB_PREFIXED(window);


//////////////////////////////////////////////////////////////////////
// # Interfaces
//////////////////////////////////////////////////////////////////////


typedef struct {
	WAYLIB_C_CPP_TYPE(WGPUInstance, wgpu::Instance) instance;
	WAYLIB_C_CPP_TYPE(WGPUAdapter, wgpu::Adapter) adapter;
	WAYLIB_C_CPP_TYPE(WGPUDevice, wgpu::Device) device;
	WAYLIB_NULLABLE(WAYLIB_C_CPP_TYPE(WGPUSurface, wgpu::Surface)) surface
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	;
} WAYLIB_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC);


//////////////////////////////////////////////////////////////////////
// # Core Configuration
//////////////////////////////////////////////////////////////////////


typedef struct {
	WAYLIB_OPTIONAL(WGPUPresentMode) presentation_mode
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= {}
#endif
	; WGPUCompositeAlphaMode alpha_mode
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= wgpu::CompositeAlphaMode::Auto
#endif
	; bool automatic_should_configure_now
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	;
} WAYLIB_PREFIXED(surface_configuration);

#endif // WAYLIB_INTERFACES_f9641d712fbea9630f1ce0a0613269d7