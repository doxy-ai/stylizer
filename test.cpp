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

@group(2) @binding(0) var texture: texture_2d<f32>;
@group(2) @binding(1) var textureSampler: sampler;

@vertex
fn vertex(in: vertex_input, @builtin(vertex_index) vertex_index: u32, @builtin(instance_index) instance_index: u32) -> vertex_output {
	if false { ensure_gbuffer_layout(); }
	return unpack_vertex_input_full(in, instances[instance_index].transform, camera_view_projection_matrix(utility.camera));
}

@fragment
fn fragment(vert: vertex_output) -> fragment_output {
	return fragment_output(
		textureSample(texture, textureSampler, vert.uv),
		vec4f(vert.normal, 1.0),
	);
}
	)_", {.vertex_entry_point = "vertex", .fragment_entry_point = "fragment", .preprocessor = &p}).throw_if_error();

	wl::auto_release texture = std::move(*wl::img::load("../resources/test.hdr").throw_if_error()
		.upload(state, {.sampler_type = wl::texture_create_sampler_type::Trilinear}).throw_if_error()
		.generate_mipmaps(state).throw_if_error());

	wl::auto_release model = wl::obj::load("../resources/suzane_highpoly.obj").throw_if_error();
	model.meshes()[0].indices = nullptr;
	model.upload(state, gbuffer).throw_if_error();
	wl::auto_release material = wl::material(wl::materialC{
		.texture_count = 1,
		.textures = &texture,
		.shaders = &shader,
		.shader_count = 1
	});
	material.upload(state, gbuffer, {}, {.double_sided = true});
	model.material_count = 1;
	model.c().materials = &material;
	model.c().mesh_materials = nullptr;
	model.transform = glm::identity<glm::mat4x4>();

	wl::auto_release<wl::gpu_buffer> utility_buffer;
	wl::time time{};
	wl::camera3D camera = wl::camera3DC{.position = {0, 1, -1}, .target_position = wl::vec3f(0)};

	WAYLIB_MAIN_LOOP(!window.should_close(),
	// while(!window.should_close()) {
		utility_buffer = time.calculate().update_utility_buffer(state, utility_buffer).throw_if_error();

		camera.position = wl::vec3f(2 * cos(time.since_start), sin(time.since_start / 4), 2 * sin(time.since_start));
		utility_buffer = camera.calculate_matricies(window.get_size()).update_utility_buffer(state, utility_buffer).throw_if_error();

		wl::auto_release draw = gbuffer.begin_drawing(state, {{.1, .2, .7, 1}}, utility_buffer).throw_if_error();
		{
			model.draw(draw).throw_if_error();
		}
		draw.draw().throw_if_error();

		// Present gbuffer's color
		window.present(state, gbuffer.color()).throw_if_error();
	// }
	);
}