#define WEBGPU_CPP_IMPLEMENTATION
#define IS_WAYLIB_CORE_CPP
#include "core.hpp"

#include <algorithm>

WAYLIB_BEGIN_NAMESPACE

//////////////////////////////////////////////////////////////////////
// # Errors
//////////////////////////////////////////////////////////////////////

	std::string errors::singleton;

	WAYLIB_NULLABLE(const char*) get_error_message() {
		return errors::get();
	}

	void clear_error_message() {
		errors::clear();
	}

//////////////////////////////////////////////////////////////////////
// # Thread Pool
//////////////////////////////////////////////////////////////////////

	WAYLIB_NULLABLE(WAYLIB_PREFIXED(thread_pool_future)*) WAYLIB_PREFIXED(thread_pool_enqueue)(
		void(*function)(),
		bool return_future /*= false*/,
		WAYLIB_OPTIONAL(size_t) initial_pool_size /*= {}*/
	) {
		auto future = WAYLIB_NAMESPACE::thread_pool::enqueue<void(*)()>(std::move(function), initial_pool_size);
		if(!return_future) return nullptr;
		return new WAYLIB_PREFIXED(thread_pool_future)(std::move(future));
	}

	void WAYLIB_PREFIXED(release_thread_pool_future)(
		WAYLIB_PREFIXED(thread_pool_future)* future
	) {
		delete future;
	}

	void WAYLIB_PREFIXED(thread_pool_future_wait)(
		WAYLIB_PREFIXED(thread_pool_future)* future,
		WAYLIB_OPTIONAL(float) seconds_until_timeout /*= {}*/
	) {
		if(!seconds_until_timeout)
			future->wait();
		else {
			auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::duration<double>(*seconds_until_timeout));
			future->wait_for(microseconds);
		}
	}


//////////////////////////////////////////////////////////////////////
// # wgpu_state
//////////////////////////////////////////////////////////////////////


result<wgpu_state> wgpu_state::default_from_instance(WGPUInstance instance_, WAYLIB_NULLABLE(WGPUSurface) surface_ /* = nullptr */, bool prefer_low_power /* = false */) WAYLIB_TRY {
	wgpu::Instance instance = instance_; wgpu::Surface surface = surface_;

	wgpu::Adapter adapter = instance.requestAdapter(WGPURequestAdapterOptions{
		.compatibleSurface = surface,
		.powerPreference = prefer_low_power ? wgpu::PowerPreference::LowPower : wgpu::PowerPreference::HighPerformance
	});
	if(!adapter) return unexpected("Failed to find adapter.");

	WGPUFeatureName float32filterable = WGPUFeatureName_Float32Filterable;
	wgpu::Device device = adapter.requestDevice(WGPUDeviceDescriptor{
		.label = "Waylib Device",
		.requiredFeatureCount = 1,
		.requiredFeatures = &float32filterable,
		.requiredLimits = nullptr,
		.defaultQueue = {
			.label = "Waylib Queue"
		},
		.deviceLostCallbackInfo = {
			.mode = wgpu::CallbackMode::AllowSpontaneous,
			.callback = [](WGPUDevice const* device, WGPUDeviceLostReason reason, char const* message, void* userdata) {
				std::stringstream s;
				s << "Device " << wgpu::Device{*device} << " lost: reason " << wgpu::DeviceLostReason{reason};
				if (message) s << " (" << message << ")";
#ifdef __cpp_exceptions
				throw exception(s.str());
#else
				assert(false, s.str().c_str());
#endif
			}
		},
		.uncapturedErrorCallbackInfo = {
			.callback = [](WGPUErrorType type, char const* message, void* userdata) {
				std::stringstream s;
				s << "Uncaptured device error: type " << wgpu::ErrorType{type};
				if (message) s << " (" << message << ")";
#ifdef __cpp_exceptions
				throw exception(s.str());
#else
				errros::set(s.str());
#endif
			}
		},

	});
	if(!device) return unexpected("Failed to create device.");

	return wgpu_state(instance, adapter, device, surface);
} WAYLIB_CATCH


WAYLIB_OPTIONAL(WAYLIB_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)) WAYLIB_PREFIXED(default_state_from_instance)(
	WGPUInstance instance,
	WAYLIB_NULLABLE(WGPUSurface) surface /*= nullptr*/,
	bool prefer_low_power /*= false*/
) {
	return res2opt(result<WAYLIB_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)>(
		wgpu_state::default_from_instance(instance, surface, prefer_low_power)
	));
}

WAYLIB_OPTIONAL(WAYLIB_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)) WAYLIB_PREFIXED(create_state)(
	WAYLIB_NULLABLE(WGPUSurface) surface /*= nullptr*/,
	bool prefer_low_power /*= false*/
) {
	return res2opt(result<WAYLIB_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)>(
		wgpu_state::create_default(surface, prefer_low_power)
	));
}

void WAYLIB_PREFIXED(release_state)(
	WAYLIB_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)* state_,
	bool adapter_release /*= true*/,
	bool instance_release /*= true*/
) {
	wgpu_state& state = *static_cast<wgpu_state*>(state_);
	state.release(adapter_release, instance_release);
}

WAYLIB_PREFIXED(surface_configuration) WAYLIB_PREFIXED(default_surface_configuration)();

void WAYLIB_PREFIXED(state_configure_surface)(
	WAYLIB_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)* state_,
	WAYLIB_PREFIXED_C_CPP_TYPE(vec2u, vec2uC) size,
	WAYLIB_PREFIXED(surface_configuration) config /*= {}*/
) {
	wgpu_state& state = *static_cast<wgpu_state*>(state_);
	state.configure_surface(fromC(size), config);
}


//////////////////////////////////////////////////////////////////////
// # Miscilanious
//////////////////////////////////////////////////////////////////////

bool format_is_srgb(WGPUTextureFormat format) {
	switch(static_cast<wgpu::TextureFormat>(format)) {
	case WGPUTextureFormat_RGBA8UnormSrgb:
	case WGPUTextureFormat_BGRA8UnormSrgb:
	case WGPUTextureFormat_BC1RGBAUnormSrgb:
	case WGPUTextureFormat_BC2RGBAUnormSrgb:
	case WGPUTextureFormat_BC3RGBAUnormSrgb:
	case WGPUTextureFormat_BC7RGBAUnormSrgb:
	case WGPUTextureFormat_ETC2RGB8UnormSrgb:
	case WGPUTextureFormat_ETC2RGB8A1UnormSrgb:
	case WGPUTextureFormat_ETC2RGBA8UnormSrgb:
	case WGPUTextureFormat_ASTC4x4UnormSrgb:
	case WGPUTextureFormat_ASTC5x4UnormSrgb:
	case WGPUTextureFormat_ASTC5x5UnormSrgb:
	case WGPUTextureFormat_ASTC6x5UnormSrgb:
	case WGPUTextureFormat_ASTC6x6UnormSrgb:
	case WGPUTextureFormat_ASTC8x5UnormSrgb:
	case WGPUTextureFormat_ASTC8x6UnormSrgb:
	case WGPUTextureFormat_ASTC8x8UnormSrgb:
	case WGPUTextureFormat_ASTC10x5UnormSrgb:
	case WGPUTextureFormat_ASTC10x6UnormSrgb:
	case WGPUTextureFormat_ASTC10x8UnormSrgb:
	case WGPUTextureFormat_ASTC10x10UnormSrgb:
	case WGPUTextureFormat_ASTC12x10UnormSrgb:
	case WGPUTextureFormat_ASTC12x12UnormSrgb:
		return true;
	default: return false;
	}
}

wgpu::TextureFormat determine_best_surface_format(WGPUAdapter adapter, WGPUSurface surface_) {
	wgpu::Surface surface = surface_;

	wgpu::SurfaceCapabilities caps;
	surface.getCapabilities(adapter, &caps);

	for(size_t i = 0; i < caps.formatCount; ++i)
		if(format_is_srgb(caps.formats[i]))
			return caps.formats[i];

	errors::set("Surface does not support SRGB!");
	return caps.formats[0];
}

wgpu::PresentMode determine_best_presentation_mode(WGPUAdapter adapter, WGPUSurface surface_) {
	wgpu::Surface surface = surface_;

	wgpu::SurfaceCapabilities caps;
	surface.getCapabilities(adapter, &caps);

	auto begin = caps.presentModes; auto end = caps.presentModes + caps.presentModeCount;
	if(std::find(begin, end, wgpu::PresentMode::Mailbox) != end)
		return wgpu::PresentMode::Mailbox;
	if(std::find(begin, end, wgpu::PresentMode::Immediate) != end)
		return wgpu::PresentMode::Immediate;
#ifndef __EMSCRIPTEN__
	if(std::find(begin, end, wgpu::PresentMode::FifoRelaxed) != end)
		return wgpu::PresentMode::FifoRelaxed;
#endif
	return wgpu::PresentMode::Fifo; // Always supported
}

WAYLIB_END_NAMESPACE