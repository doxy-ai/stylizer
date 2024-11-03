#include <iostream>

#include "stylizer/stylizer.hpp"

int main() {
	sl::auto_release window = sl::window::create({800, 600});
	sl::auto_release state = window.create_default_state();
	window.reconfigure_surface_on_resize(state);

	// GBuffer that gets its color format from the window
	sl::auto_release gbuffer = sl::geometry_buffer::create_default(state, window.get_size(), {
		.color_format = wgpu::TextureFormat::Undefined
	});
	window.auto_resize_gbuffer(state, gbuffer);

	auto p = sl::shader_preprocessor{}.initialize_platform_defines(state).initialize_virtual_filesystem();
	sl::auto_release shader = sl::shader::from_wgsl(state, R"_(
#define STYLIZER_CAMERA_DATA_IS_3D
#include <stylizer/default_gbuffer>

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
	)_", {.vertex_entry_point = "vertex", .fragment_entry_point = "fragment", .preprocessor = &p});

	sl::auto_release texture = std::move(sl::img::load("../resources/test.hdr")
		.upload(state, {.sampler_type = sl::texture_create_sampler_type::Nearest})
		.generate_mipmaps(state));

	sl::auto_release model = sl::obj::load("../resources/suzane_highpoly.obj");
	model.meshes()[0].indices = nullptr;
	model.upload(state, gbuffer);
	sl::auto_release material = sl::material(sl::materialC{
		.texture_count = 1,
		.textures = &texture,
		.shaders = &shader,
		.shader_count = 1
	});
	material.upload(state, gbuffer, {}, {.double_sided = true});
	model.c().material_count = 1;
	model.c().materials = &material;
	model.c().mesh_materials = nullptr;
	model.transform = glm::identity<glm::mat4x4>();

	// From: https://learnopengl.com/Advanced-OpenGL/Cubemaps

	sl::auto_release skyTexture = sl::img::load_frames(std::array<std::filesystem::path, 6>{
		"../resources/skybox/right.jpg", "../resources/skybox/left.jpg",
		"../resources/skybox/top.jpg", "../resources/skybox/bottom.jpg",
		"../resources/skybox/front.jpg", "../resources/skybox/back.jpg"
	}).upload_frames_as_cube(state, {.sampler_type = sl::texture_create_sampler_type::Trilinear});
	sl::auto_release skyShader = sl::shader::from_wgsl(state, R"_(
#define STYLIZER_CAMERA_DATA_IS_3D
#include <stylizer/default_gbuffer>
#include <stylizer/inverse>

@group(2) @binding(0) var texture: texture_cube<f32>;
@group(2) @binding(1) var texture_sampler: sampler;

@vertex
fn vertex(in: vertex_input, @builtin(vertex_index) vertex_index: u32, @builtin(instance_index) instance_index: u32) -> vertex_output {
	if false { ensure_gbuffer_layout(); }
	return unpack_vertex_input(in);
}

@fragment
fn fragment(vert: vertex_output) -> fragment_output {
	// Calculate the direction of the pixel
	let clipSpacePos = vec4(vert.position.xy, 1, 1);
	let V = inverse_view_matrix(utility.camera.view_matrix);
	let P = inverse4x4(utility.camera.projection_matrix);
	let worldSpacePos = V * P * clipSpacePos;
	let direction = normalize(worldSpacePos.xyz - utility.camera.position);

	return fragment_output(
		textureSample(texture, texture_sampler, direction),
		vec4f(0.0),
	);
}
	)_", {.vertex_entry_point = "vertex", .fragment_entry_point = "fragment", .preprocessor = &p});
	sl::auto_release<sl::material> skyMat{}; skyMat.zero();
	skyMat.c().texture_count = 1;
	skyMat.c().textures = &skyTexture;
	skyMat.c().shader_count = 1;
	skyMat.c().shaders = &skyShader;
	skyMat.upload(state, gbuffer, {}, {.depth_function = {}});
	sl::auto_release<sl::model> sky{}; sky.zero();
	sky.c().mesh_count = 1;
	sky.c().meshes = {true, new sl::mesh(sl::mesh::fullscreen_mesh(state))};
	sky.c().material_count = 1;
	sky.c().materials = &skyMat;
	sky.c().mesh_materials = nullptr;

	sl::auto_release<sl::gpu_buffer> utility_buffer;
	sl::time time{};
	sl::camera3D camera = sl::camera3DC{.position = {0, 1, -1}, .target_position = sl::vec3f(0)};

	STYLIZER_MAIN_LOOP(!window.should_close(),
	// while(!window.should_close()) {
		utility_buffer = time.calculate().update_utility_buffer(state, utility_buffer);

		camera.position = sl::vec3f(2 * cos(time.since_start), sin(time.since_start / 4), 2 * sin(time.since_start));
		utility_buffer = camera.calculate_matrices(window.get_size()).update_utility_buffer(state, utility_buffer);

		sl::auto_release draw = gbuffer.begin_drawing(state, {{.1, .2, .7, 1}}, utility_buffer);
		{
			sky.draw(draw);

			model.draw(draw);
		}
		draw.draw();

		// Present gbuffer's color
		window.present(state, gbuffer.color());
	// }
	);
}