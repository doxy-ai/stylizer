#include "hlsl++/matrix_float_type.h"
#define HLSLPP_FEATURE_TRANSFORM
#include "hlsl++.h"
#include "stylizer/api/api.hpp"

namespace stylizer {
	using namespace hlslpp;

	namespace api {
		template<>
		struct vertex_buffer_type_format<float1> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::f32x1; };
		template<>
		struct vertex_buffer_type_format<float2> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::f32x2; };
		template<>
		struct vertex_buffer_type_format<float3> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::f32x3; };
		template<>
		struct vertex_buffer_type_format<float4> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::f32x4; };
		template<>
		struct vertex_buffer_type_format<int1> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::i32x1; };
		template<>
		struct vertex_buffer_type_format<int2> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::i32x2; };
		template<>
	    struct vertex_buffer_type_format<int3> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::i32x3; };
		template<>
	    struct vertex_buffer_type_format<int4> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::i32x4; };
		template<>
		struct vertex_buffer_type_format<uint1> { static constexpr auto format = render_pipeline::config::vertex_buffer_layout::attribute::format::u32x1; };
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