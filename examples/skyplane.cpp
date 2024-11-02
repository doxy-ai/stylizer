#include <iostream>

#include "waylib/waylib.hpp"

int main() {
	wl::auto_release window = wl::window::create({800, 600});
	wl::auto_release state = window.create_default_state();
	window.reconfigure_surface_on_resize(state);

	// GBuffer that gets its color format from the window
	wl::auto_release gbuffer = wl::Gbuffer::create_default(state, window.get_size(), {
		.color_format = wgpu::TextureFormat::Undefined
	});
	window.auto_resize_gbuffer(state, gbuffer);

	wl::auto_release skyShader = wl::shader::from_wgsl(state, R"_(
#define WAYLIB_CAMERA_DATA_IS_3D
#include <waylib/default_gbuffer>
#include <waylib/inverse>

@group(2) @binding(0) var texture: texture_2d<f32>;
@group(2) @binding(1) var texture_sampler: sampler;

@vertex
fn vertex(in: vertex_input, @builtin(vertex_index) vertex_index: u32, @builtin(instance_index) instance_index: u32) -> vertex_output {
	if false { ensure_gbuffer_layout(); }
	return unpack_vertex_input(in); // NOTE: Applying more complex calculations to the vertex breaks the NDC calculations in the fragment shader
}

@fragment
fn fragment(vert: vertex_output) -> fragment_output {
	// Calculate the direction of the pixel
	let clipSpacePos = vec4(vert.position.xy, 1, 1);
	let V = inverse_view_matrix(utility.camera.view_matrix); // NOTE: inverse4x4 could be used as well, but inverse_view_matrix is cheaper (but relies on assumptions about view matricies that aren't true in general)
	let P = inverse4x4(utility.camera.projection_matrix);
	let worldSpacePos = V * P * clipSpacePos;
	let direction = normalize(worldSpacePos.xyz - utility.camera.position);

	// Equirectangularly project the direction
	let longitude = degrees(atan2(direction.z, direction.x)) / 360;
	let latitude = (degrees(acos(direction.y)) + 90) / 360;
	return fragment_output(
		// vec4f(longitude, latitude, 1.0, 1.0),
		// clipSpacePos,
		textureSample(texture, texture_sampler, vec2f(longitude, latitude)),
		vec4f(0.0),
	);
}
	)_", {.vertex_entry_point = "vertex", .fragment_entry_point = "fragment", .preprocessor = &p});
	wl::auto_release skyTexture = wl::img::load("../resources/symmetrical_garden_02_1k.hdr")
		.upload(state, {.sampler_type = wl::texture_create_sampler_type::Trilinear});
	wl::auto_release<wl::material> skyMat{}; skyMat.zero();
	skyMat.c().texture_count = 1;
	skyMat.c().textures = &skyTexture;
	skyMat.c().shader_count = 1;
	skyMat.c().shaders = &skyShader;
	skyMat.upload(state, gbuffer, {}, {.depth_function = {}}); // Note: Passing nullopt in place of a depth function will disable depth testing
	wl::auto_release<wl::model> sky{}; sky.zero();
	sky.c().mesh_count = 1;
	sky.c().meshes = {true, new wl::mesh(wl::mesh::fullscreen_mesh(state))};
	sky.c().material_count = 1;
	sky.c().materials = &skyMat;
	sky.c().mesh_materials = nullptr;

	wl::auto_release<wl::gpu_buffer> utility_buffer;
	wl::time time{};
	wl::camera3D camera = wl::camera3DC{.position = {0, 1, -1}, .target_position = wl::vec3f(0)};

	// WAYLIB_MAIN_LOOP(!window.should_close(),
	while(!window.should_close()) {
		utility_buffer = time.calculate().update_utility_buffer(state, utility_buffer);

		camera.position = wl::vec3f(2 * cos(time.since_start), sin(time.since_start / 4), 2 * sin(time.since_start));
		utility_buffer = camera.calculate_matricies(window.get_size()).update_utility_buffer(state, utility_buffer);

		wl::auto_release draw = gbuffer.begin_drawing(state, {{.1, .2, .7, 1}}, utility_buffer);
		{
			sky.draw(draw);
		}
		draw.draw();

		// Present gbuffer's color
		window.present(state, gbuffer.color());
	}
	// );
}