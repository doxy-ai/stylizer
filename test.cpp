#include <iostream>

#include "waylib/waylib.hpp"

int main() {
	wl::auto_release window = wl::window::create({800, 600});
	wl::auto_release state = window.create_default_state().throw_if_error();
	window.reconfigure_surface_on_resize(state).throw_if_error();

	// GBuffer that gets its color format from the window
	auto gbuffer = wl::Gbuffer::create_default(state, window.get_size(), {
		.color_format = wgpu::TextureFormat::Undefined
	}).throw_if_error();
	window.auto_resize_gbuffer(state, gbuffer);

	auto p = wl::shader_preprocessor{}.initialize_platform_defines(state);
	wl::auto_release shader = wl::shader::from_wgsl(state, R"_(
struct vertex_input {
	@location(0) position: vec4f,
	@location(1) normal: vec4f,
	@location(2) tangent: vec4f,
	@location(3) uv: vec4f,
	@location(4) cta: vec4f,
	@location(5) color: vec4f,
	@location(6) bones: vec4u,
	@location(7) bone_weights: vec4f,
};

struct vertex_output {
	@builtin(position) raw_position: vec4f,
	@location(0) normal: vec3f,
	@location(1) color: vec4f,
};

struct fragment_output {
	@location(0) color: vec4f,
	@location(1) normal: vec4f,
};

@vertex
fn vertex(in: vertex_input, @builtin(vertex_index) in_vertex_index: u32) -> vertex_output {
	return vertex_output(in.position, in.normal.xyz, in.color);
}

@fragment
fn fragment(vert: vertex_output) -> fragment_output {
	return fragment_output(
		vec4f(vert.color.rgb, 1.0),
		vec4f(vert.normal, 1.0),
	);
}
	)_", {.vertex_entry_point = "vertex", .fragment_entry_point = "fragment", .preprocessor = &p}).throw_if_error();

	auto model = wl::obj::load("../resources/tri.obj").throw_if_error();
	model.meshes()[0].indices = nullptr;
	model.upload(state, gbuffer).throw_if_error();
	auto material = wl::material(wl::materialC{
		.shaders = &shader,
		.shader_count = 1
	});
	material.upload(state, gbuffer, {}, {.double_sided = true});
	model.material_count = 1;
	model.c().materials = &material;
	model.c().mesh_materials = nullptr;


	// WAYLIB_MAIN_LOOP(!window.should_close(),
	while(!window.should_close()) {
		wl::auto_release draw = gbuffer.begin_drawing(state, {{.1, .2, .7, 1}}).throw_if_error();
		{
			model.draw_instanced(draw, {}).throw_if_error();
		}
		draw.draw().throw_if_error();

		// Present gbuffer's color
		window.present(state, gbuffer.color()).throw_if_error();
	}
	// );
}