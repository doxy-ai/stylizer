#pragma once

// #define GLM_FORCE_MESSAGES
#define GLM_FORCE_SWIZZLE
#define GLM_FORCE_SIZE_T_LENGTH
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE // Use a clip space with z in [0, 1]
#define GLM_FORCE_UNRESTRICTED_GENTYPE // Allow more operations on integers
#include <glm/vec2.hpp> // glm::vec2
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp> // Look at, Ortho, Pers

namespace stylizer {
	using namespace glm;

	using int1 = int32_t;
	using int2 = glm::vec<2, int32_t, glm::defaultp>;
	using int3 = glm::vec<3, int32_t, glm::defaultp>;
	using int4 = glm::vec<4, int32_t, glm::defaultp>;

	using uint1 = uint32_t;
	using uint2 = glm::vec<2, uint32_t, glm::defaultp>;
	using uint3 = glm::vec<3, uint32_t, glm::defaultp>;
	using uint4 = glm::vec<4, uint32_t, glm::defaultp>;

	using float1 = float;
	using float2 = glm::vec<2, float, glm::defaultp>;
	using float3 = glm::vec<3, float, glm::defaultp>;
	using float4 = glm::vec<4, float, glm::defaultp>;

	using float4x4 = glm::mat<4, 4, float, glm::defaultp>;

	constexpr static auto identity_matrix = identity<float4x4>();

	inline float4x4 look_at(float3 position, float3 target, float3 up = {0, 1, 0}) {
		return lookAt(position, target, up);
	}

	inline float4x4 perspective(float fov, float width, float height, float zNear, float zFar) {
		return perspectiveFov(fov, width, height, zNear, zFar);
	}
	// inline float4x4 perspective(float fovy, float2 dims, float zNear, float zFar) {
	// 	return perspective(fovy, dims.x, dims.y, zNear, zFar);
	// }

	inline float4x4 orthographic(float left, float right, float bottom, float top, float zNear, float zFar) {
		return ortho(left, right, bottom, top, zNear, zFar);
	}
	// inline float4x4 orthographic(float left, float top, float zNear, float zFar) {
	// 	return orthographic(left, -left, -top, top, zNear, zFar);
	// }

	// // From: https://github.com/g-truc/glm/blob/master/glm/ext/matrix_clip_space.inl#L352
	// inline float4x4 perspective(float fov, float width, float height, float zNear, float zFar) {
	// 	assert(width > 0.f);
	// 	assert(height > 0.f);
	// 	assert(fov > 0.f);
	
	// 	const float h = cos(0.5f * fov) / sin(0.5f * fov);
	// 	const float w = h * height / width; ///todo max(width , Height) / min(width , Height)?

	// 	float4x4 out(0);
	// 	out[0][0] = w;
	// 	out[1][1] = h;
	// 	out[2][2] = zFar / (zNear - zFar);
	// 	out[2][3] = -1.f;
	// 	out[3][2] = -(zFar * zNear) / (zFar - zNear);
	// 	return transpose(out);
	// }
	

	// // From: https://github.com/g-truc/glm/blob/master/glm/ext/matrix_clip_space.inl#L42
	// inline float4x4 orthographic(float left, float right, float bottom, float top, float zNear, float zFar) {
	// 	float4x4 out(0);
	// 	out[0][0] = 2.f / (right - left);
	// 	out[1][1] = 2.f / (top - bottom);
	// 	out[2][2] = - 1.f / (zFar - zNear);
	// 	out[3][0] = - (right + left) / (right - left);
	// 	out[3][1] = - (top + bottom) / (top - bottom);
	// 	out[3][2] = - zNear / (zFar - zNear);
	// 	return transpose(out);
	// }
	



	namespace api {
		// template<>
		// struct vertex_buffer_type_format<float1> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::f32x1; };
		template<>
		struct vertex_buffer_type_format<float2> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::f32x2; };
		template<>
		struct vertex_buffer_type_format<float3> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::f32x3; };
		template<>
		struct vertex_buffer_type_format<float4> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::f32x4; };
		// template<>
		// struct vertex_buffer_type_format<int1> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::i32x1; };
		template<>
		struct vertex_buffer_type_format<int2> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::i32x2; };
		template<>
	    struct vertex_buffer_type_format<int3> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::i32x3; };
		template<>
	    struct vertex_buffer_type_format<int4> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::i32x4; };
		// template<>
		// struct vertex_buffer_type_format<uint1> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::u32x1; };
		template<>
		struct vertex_buffer_type_format<uint2> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::u32x2; };
		template<>
	    struct vertex_buffer_type_format<uint3> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::u32x3; };
		template<>
	    struct vertex_buffer_type_format<uint4> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::u32x4; };


		inline uint2 convert(vec2u v) { return {v.x, v.y}; }
		inline vec2u convert(int2 v) { return {static_cast<size_t>(v.x), static_cast<size_t>(v.y)}; }
		inline vec2u convert(uint2 v) { return {v.x, v.y}; }
		inline uint3 convert(vec3u v) { return {v.x, v.y, v.z}; }
		inline vec3u convert(int3 v) { return {static_cast<size_t>(v.x), static_cast<size_t>(v.y), static_cast<size_t>(v.z)}; }
		inline vec3u convert(uint3 v) { return {v.x, v.y, v.z}; }
		inline float4 convert(color32 v) { return {v.r, v.g, v.b, v.a}; }
		inline color32 convert(float4 v) { return {v.r, v.g, v.b, v.a}; }
	}
}