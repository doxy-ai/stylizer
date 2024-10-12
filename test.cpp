#include <iostream>

#include "waylib/waylib.hpp"

int main() {
	wl::auto_release window = wl::window::create({800, 600});
	wl::auto_release state = window.create_default_state().throw_if_error();
	window.reconfigure_surface_on_resize(state).throw_if_error();

	// GBuffer that gets its color format from the window
	wl::auto_release gbuffer = wl::Gbuffer::create_default(state, window.get_size(), {
		.color_format = wgpu::TextureFormat::Undefined
	}).throw_if_error();
	window.auto_resize_gbuffer(state, gbuffer);

	auto p = wl::shader_preprocessor{}.initialize_platform_defines(state).initialize_virtual_filesystem();
	wl::auto_release shader = wl::shader::from_wgsl(state, R"_(
#define WAYLIB_CAMERA_DATA_IS_3D
#include <waylib/default_gbuffer>

@vertex
fn vertex(in: vertex_input, @builtin(vertex_index) in_vertex_index: u32) -> vertex_output {
	if false { ensure_pipeline_layout(); }
	return unpack_input(in);
}

@fragment
fn fragment(vert: vertex_output) -> fragment_output {
	return fragment_output(
		vec4f(vert.color.rgb, 1.0),
		vec4f(vert.normal, 1.0),
	);
}
	)_", {.vertex_entry_point = "vertex", .fragment_entry_point = "fragment", .preprocessor = &p}).throw_if_error();

	wl::auto_release model = wl::obj::load("../resources/tri.obj").throw_if_error();
	model.meshes()[0].indices = nullptr;
	model.upload(state, gbuffer).throw_if_error();
	wl::auto_release material = wl::material(wl::materialC{
		.shaders = &shader,
		.shader_count = 1
	});
	material.upload(state, gbuffer, {}, {.double_sided = true});
	model.material_count = 1;
	model.c().materials = &material;
	model.c().mesh_materials = nullptr;

	wl::auto_release<wl::gpu_buffer> utility_buffer;
	wl::time time{};
	wl::camera3D camera = wl::camera3DC{.position = {1, 2, 3}, .target_position = wl::vec3f(0), .up = {0, 1, 0}};

	// WAYLIB_MAIN_LOOP(!window.should_close(),
	while(!window.should_close()) {
		utility_buffer = time.calculate().update_utility_buffer(state, utility_buffer).throw_if_error();
		utility_buffer = camera.calculate_matricies(window.get_size()).update_utility_buffer(state, utility_buffer).throw_if_error();

		wl::auto_release draw = gbuffer.begin_drawing(state, {{.1, .2, .7, 1}}, utility_buffer).throw_if_error();
		{
			model.draw_instanced(draw, {}).throw_if_error();
		}
		draw.draw().throw_if_error();

		// Present gbuffer's color
		window.present(state, gbuffer.color()).throw_if_error();
	}
	// );
}