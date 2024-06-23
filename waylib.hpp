#pragma once
#include <cstdint>
#include <cstddef>
#include <webgpu/webgpu.hpp>

#include "wgsl_types.hpp"

#ifdef WAYLIB_NAMESPACE_NAME
namespace WAYLIB_NAMESPACE_NAME {
inline namespace wgpu {
#else
namespace wgpu {
#endif

	using namespace ::wgpu;

	using instance = Instance;
	using adapter = Adapter;
	using device = Device;
	using surface = Surface;
	using queue = Queue;
}

#include "waylib.h"

wgpu::Color to_webgpu(const wl::color8bit& color);
wgpu::Color to_webgpu(const wl::color32bit& color);

#ifdef WAYLIB_NAMESPACE_NAME
}
#endif