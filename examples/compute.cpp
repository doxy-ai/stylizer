#include <iostream>

#include "waylib/waylib.hpp"

int main() {
	wl::auto_release state = wl::wgpu_state::create_default();
	
	std::vector<uint32_t> data = {1, 2, 3, 4, 5};
	std::array<wl::gpu_buffer, 2> buffers = {
		wl::gpu_buffer::create<uint32_t>(state, data),
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
	)_", {.compute_entry_point = "compute"});

	wl::computer::dispatch(state, buffers, {}, computerShader, {5, 1, 1});
	buffers[1].download(state);

	auto span = buffers[1].map_cpu_data_span<uint32_t>();
	assert(span[0] == 1);
	assert(span[1] == 4);
	assert(span[2] == 9);
	assert(span[3] == 16);
	assert(span[4] == 25);

	for(auto& buffer: buffers) buffer.release();
}