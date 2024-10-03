#include <cstdint>
#include <cstring>
#include <iostream>

#include "waylib/waylib.hpp"

int main() {
	wl::auto_release window = wl::window::create({800, 600});
	wl::auto_release state = window.create_default_state().throw_if_error();
	window.reconfigure_surface_on_resize(state);

	// GBuffer that gets its color format from the window
	auto gbuffer = wl::Gbuffer::create_default(state, window.get_size(), {
		.color_format = wgpu::TextureFormat::Undefined
	}).throw_if_error();
	window.raw.sizeEvent.append([&gbuffer, &state](auto& window, int x, int y) {
		gbuffer.resize(state, {x, y}).throw_if_error();
	});

	std::vector<uint32_t> data = {1, 2, 3, 4, 5};
	std::array<wl::gpu_buffer, 2> buffers = {
		wl::gpu_buffer::create<uint32_t>(state, data).throw_if_error(),
		{wl::gpu_bufferC{.size = buffers[0].size}}
	};
	buffers[1].upload(state, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);

	wl::auto_release computerShader = wl::shader::from_wgsl(state, R"_(
@group(0) @binding(0) var<storage, read> in: array<u32>;
@group(0) @binding(1) var<storage, read_write> out: array<u32>;

@compute @workgroup_size(1)
fn compute(@builtin(global_invocation_id) id: vec3<u32>) {
	let i = id.x;
	out[i] = in[i] * in[i];
}
	)_", {.compute_entry_point = "compute"}).throw_if_error();

	wl::computer::dispatch(state, buffers, {}, computerShader, {5, 1, 1}).throw_if_error();
	buffers[1].download(state).throw_if_error();
	auto span = buffers[1].map_cpu_data_span<uint32_t>();
	for(auto& buffer: buffers) buffer.release();

	// WAYLIB_MAIN_LOOP(!window.should_close(),
	while(!window.should_close()) {
		auto draw = gbuffer.begin_drawing(state, {{.7, .1, .2, 1}}).throw_if_error();
		{

		}
		draw.draw().throw_if_error();

		// Blit texture
		auto surface = state.current_surface_texture().throw_if_error();
		gbuffer.color().blit_to(state, surface).throw_if_error();
		state.surface.present();
	}
	// );
}