#pragma once

struct vertex_output {
	@builtin(position) raw_position: vec4f,
	@location(0) uv: vec2f
};

@group(0) @binding(0) var texture: texture_2d<f32>;
@group(0) @binding(1) var sampler_: sampler;

#ifdef WAYLIB_BLIT_DEFAULT_VERTEX_ENTRY_POINT
@vertex
fn vertex(@builtin(vertex_index) vertex_index: u32) -> vertex_output
#else
fn generate_vertex(vertex_index: u32) -> vertex_output
#endif
{
	switch vertex_index {
		case 0u: {
			return vertex_output(vec4f(-1.0, -3.0, .99, 1.0), vec2f(0.0, 2.0));
		}
		case 1u: {
			return vertex_output(vec4f(3.0, 1.0, .99, 1.0), vec2f(2.0, 0.0));
		}
		case 2u, default: {
			return vertex_output(vec4f(-1.0, 1.0, .99, 1.0), vec2f(0.0));
		}
	}
}