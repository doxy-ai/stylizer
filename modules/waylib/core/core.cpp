#define TCPP_IMPLEMENTATION
#define WEBGPU_CPP_IMPLEMENTATION
#define IS_WAYLIB_CORE_CPP
#include "core.hpp"

#include <battery/embed.hpp>
#include <algorithm>
#include <map>

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
		.label = toWGPU("Waylib Device"),
		.requiredFeatureCount = 1,
		.requiredFeatures = &float32filterable,
		.requiredLimits = nullptr,
		.defaultQueue = {
			.label = toWGPU("Waylib Queue")
		},
#ifdef __EMSCRIPTEN__
		.deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void* userdata) {
			std::stringstream s;
			s << "Device lost: reason " << wgpu::DeviceLostReason{reason};
			if (message) s << " (" << message << ")";
#ifdef __cpp_exceptions
			throw exception(s.str());
#else // __cpp_exceptions
			assert(false, s.str().c_str());
#endif // __cpp_exceptions
		},
#else // __EMSCRIPTEN__
		.deviceLostCallbackInfo = {
			.mode = wgpu::CallbackMode::AllowSpontaneous,
			.callback = [](WGPUDevice const* device, WGPUDeviceLostReason reason, WGPUStringView message, void* userdata) {
				std::stringstream s;
				s << "Device " << wgpu::Device{*device} << " lost: reason " << wgpu::DeviceLostReason{reason};
				if (message.length) s << " (" << std::string_view{message.data, message.length} << ")";
#ifdef __cpp_exceptions
				throw exception(s.str());
#else // __cpp_exceptions
				assert(false, s.str().c_str());
#endif // __cpp_exceptions
			}
		},
		.uncapturedErrorCallbackInfo = {
			.callback = [](WGPUErrorType type, WGPUStringView message, void* userdata) {
				std::stringstream s;
				s << "Uncaptured device error: type " << wgpu::ErrorType{type};
				if (message.length) s << " (" << std::string_view{message.data, message.length} << ")";
#ifdef __cpp_exceptions
				throw exception(s.str());
#else // __cpp_exceptions
				errors::set(s.str());
#endif // __cpp_exceptions
			}
		}
#endif // __EMSCRIPTEN__
	});
	if(!device) return unexpected("Failed to create device.");

	return wgpu_stateC{instance, adapter, device, surface, wgpu::TextureFormat::Undefined};
} WAYLIB_CATCH

result<texture> wgpu_state::current_surface_texture() {
	wgpu::SurfaceTexture texture_;
	surface.getCurrentTexture(&texture_);
	if(texture_.status != wgpu::SurfaceGetCurrentTextureStatus::Success) {
		std::stringstream s; s << "Texture error: " << wgpu::SurfaceGetCurrentTextureStatus{texture_.status};
		return unexpected(s.str());
	}

	texture out = textureC{.gpu_data = texture_.texture};
	out.create_view();
	return out;
}

result<struct drawing_state> wgpu_state::begin_drawing_to_surface(WAYLIB_OPTIONAL(colorC) clear_color /* = {} */, WAYLIB_OPTIONAL(gpu_buffer&) utility_buffer /* = {} */){
	auto texture = current_surface_texture();
	if(!texture) return unexpected(texture.error());
	return texture->begin_drawing(*this, clear_color, utility_buffer);
}

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

WAYLIB_OPTIONAL(WAYLIB_PREFIXED_C_CPP_TYPE(texture, textureC)) WAYLIB_PREFIXED(current_surface_texture)(
	WAYLIB_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)* state_
) {
	wgpu_state& state = *static_cast<wgpu_state*>(state_);
	return res2opt<texture, textureC>(state.current_surface_texture());
}

WAYLIB_OPTIONAL(WAYLIB_PREFIXED_C_CPP_TYPE(drawing_state, drawing_stateC)) WAYLIB_PREFIXED(begin_drawing_to_surface)(
	WAYLIB_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)* state_,
	WAYLIB_OPTIONAL(colorC) clear_color /* = {} */
) {
	wgpu_state& state = *static_cast<wgpu_state*>(state_);
	return res2opt<drawing_state, drawing_stateC>(state.begin_drawing_to_surface(clear_color));
}


//////////////////////////////////////////////////////////////////////
// # Image
//////////////////////////////////////////////////////////////////////


result<struct texture> image::upload(wgpu_state& state, texture_create_configuation config /* = {} */, bool take_ownership_of_image /* = true */) {
	config.format = config.format ? *config.format : static_cast<WGPUTextureFormat>(convert_format(format));
	auto out = texture::create(state, size, config);
	if(!out) return out;

	wgpu::ImageCopyTexture destination;
	destination.texture = out->gpu_data;
	destination.mipLevel = 0;
	destination.origin = { 0, 0, 0 }; // equivalent of the offset argument of Queue::writeBuffer
	destination.aspect = wgpu::TextureAspect::All; // only relevant for depth/Stencil textures

	// Arguments telling how the C++ side pixel memory is laid out
	wgpu::TextureDataLayout source;
	source.offset = 0;
	source.bytesPerRow = bytes_per_pixel() * size.x;
	source.rowsPerImage = size.y;

	wgpu::Extent3D extent = {size.x, size.y, static_cast<uint32_t>(frames)};
	state.device.getQueue().writeTexture(destination, *data, source.bytesPerRow * size.y * frames, source, extent);

	if(take_ownership_of_image) out->cpu_data = {true, new image(std::move(*this))};
	else out->cpu_data = this;
	return out;
}


//////////////////////////////////////////////////////////////////////
// # Texture
//////////////////////////////////////////////////////////////////////


size_t texture::bytes_per_pixel(WGPUTextureFormat format) { // TODO: complete
	switch(static_cast<wgpu::TextureFormat>(format)) {
		case WGPUTextureFormat_R8Unorm: return sizeof(uint8_t);
		case WGPUTextureFormat_R8Snorm: return sizeof(int8_t);
		case WGPUTextureFormat_R8Uint: return sizeof(uint8_t);
		case WGPUTextureFormat_R8Sint: return sizeof(int8_t);
		case WGPUTextureFormat_R16Uint: return sizeof(uint16_t);
		case WGPUTextureFormat_R16Sint: return sizeof(int16_t);
		case WGPUTextureFormat_R16Float: return sizeof(int16_t);
		// case WGPUTextureFormat_RG8Unorm:
		// case WGPUTextureFormat_RG8Snorm:
		// case WGPUTextureFormat_RG8Uint:
		// case WGPUTextureFormat_RG8Sint:
		// case WGPUTextureFormat_R32Float:
		// case WGPUTextureFormat_R32Uint:
		// case WGPUTextureFormat_R32Sint:
		// case WGPUTextureFormat_RG16Uint:
		// case WGPUTextureFormat_RG16Sint:
		// case WGPUTextureFormat_RG16Float:
		case WGPUTextureFormat_RGBA8Unorm: return sizeof(uint8_t) * 4;
		case WGPUTextureFormat_RGBA8UnormSrgb: return sizeof(uint8_t) * 4;
		case WGPUTextureFormat_RGBA8Snorm: return sizeof(int8_t) * 4;
		case WGPUTextureFormat_RGBA8Uint: return sizeof(uint8_t) * 4;
		case WGPUTextureFormat_RGBA8Sint: return sizeof(int8_t) * 4;
		case WGPUTextureFormat_BGRA8Unorm: return sizeof(uint8_t) * 4;
		case WGPUTextureFormat_BGRA8UnormSrgb: return sizeof(uint8_t) * 4;
		// case WGPUTextureFormat_RGB10A2Uint:
		// case WGPUTextureFormat_RGB10A2Unorm:
		// case WGPUTextureFormat_RG11B10Ufloat:
		// case WGPUTextureFormat_RGB9E5Ufloat:
		// case WGPUTextureFormat_RG32Float:
		// case WGPUTextureFormat_RG32Uint:
		// case WGPUTextureFormat_RG32Sint:
		// case WGPUTextureFormat_RGBA16Uint:
		// case WGPUTextureFormat_RGBA16Sint:
		// case WGPUTextureFormat_RGBA16Float:
		// case WGPUTextureFormat_RGBA32Float:
		// case WGPUTextureFormat_RGBA32Uint:
		// case WGPUTextureFormat_RGBA32Sint:
		// case WGPUTextureFormat_Stencil8:
		// case WGPUTextureFormat_Depth16Unorm:
		// case WGPUTextureFormat_Depth24Plus:
		// case WGPUTextureFormat_Depth24PlusStencil8:
		// case WGPUTextureFormat_Depth32Float:
		// case WGPUTextureFormat_Depth32FloatStencil8:
		// case WGPUTextureFormat_BC1RGBAUnorm:
		// case WGPUTextureFormat_BC1RGBAUnormSrgb:
		// case WGPUTextureFormat_BC2RGBAUnorm:
		// case WGPUTextureFormat_BC2RGBAUnormSrgb:
		// case WGPUTextureFormat_BC3RGBAUnorm:
		// case WGPUTextureFormat_BC3RGBAUnormSrgb:
		// case WGPUTextureFormat_BC4RUnorm:
		// case WGPUTextureFormat_BC4RSnorm:
		// case WGPUTextureFormat_BC5RGUnorm:
		// case WGPUTextureFormat_BC5RGSnorm:
		// case WGPUTextureFormat_BC6HRGBUfloat:
		// case WGPUTextureFormat_BC6HRGBFloat:
		// case WGPUTextureFormat_BC7RGBAUnorm:
		// case WGPUTextureFormat_BC7RGBAUnormSrgb:
		// case WGPUTextureFormat_ETC2RGB8Unorm:
		// case WGPUTextureFormat_ETC2RGB8UnormSrgb:
		// case WGPUTextureFormat_ETC2RGB8A1Unorm:
		// case WGPUTextureFormat_ETC2RGB8A1UnormSrgb:
		// case WGPUTextureFormat_ETC2RGBA8Unorm:
		// case WGPUTextureFormat_ETC2RGBA8UnormSrgb:
		// case WGPUTextureFormat_EACR11Unorm:
		// case WGPUTextureFormat_EACR11Snorm:
		// case WGPUTextureFormat_EACRG11Unorm:
		// case WGPUTextureFormat_EACRG11Snorm:
		// case WGPUTextureFormat_ASTC4x4Unorm:
		// case WGPUTextureFormat_ASTC4x4UnormSrgb:
		// case WGPUTextureFormat_ASTC5x4Unorm:
		// case WGPUTextureFormat_ASTC5x4UnormSrgb:
		// case WGPUTextureFormat_ASTC5x5Unorm:
		// case WGPUTextureFormat_ASTC5x5UnormSrgb:
		// case WGPUTextureFormat_ASTC6x5Unorm:
		// case WGPUTextureFormat_ASTC6x5UnormSrgb:
		// case WGPUTextureFormat_ASTC6x6Unorm:
		// case WGPUTextureFormat_ASTC6x6UnormSrgb:
		// case WGPUTextureFormat_ASTC8x5Unorm:
		// case WGPUTextureFormat_ASTC8x5UnormSrgb:
		// case WGPUTextureFormat_ASTC8x6Unorm:
		// case WGPUTextureFormat_ASTC8x6UnormSrgb:
		// case WGPUTextureFormat_ASTC8x8Unorm:
		// case WGPUTextureFormat_ASTC8x8UnormSrgb:
		// case WGPUTextureFormat_ASTC10x5Unorm:
		// case WGPUTextureFormat_ASTC10x5UnormSrgb:
		// case WGPUTextureFormat_ASTC10x6Unorm:
		// case WGPUTextureFormat_ASTC10x6UnormSrgb:
		// case WGPUTextureFormat_ASTC10x8Unorm:
		// case WGPUTextureFormat_ASTC10x8UnormSrgb:
		// case WGPUTextureFormat_ASTC10x10Unorm:
		// case WGPUTextureFormat_ASTC10x10UnormSrgb:
		// case WGPUTextureFormat_ASTC12x10Unorm:
		// case WGPUTextureFormat_ASTC12x10UnormSrgb:
		// case WGPUTextureFormat_ASTC12x12Unorm:
		// case WGPUTextureFormat_ASTC12x12UnormSrgb:
		// case WGPUTextureFormat_R16Unorm:
		// case WGPUTextureFormat_RG16Unorm:
		// case WGPUTextureFormat_RGBA16Unorm:
		// case WGPUTextureFormat_R16Snorm:
		// case WGPUTextureFormat_RG16Snorm:
		// case WGPUTextureFormat_RGBA16Snorm:
		// case WGPUTextureFormat_R8BG8Biplanar420Unorm:
		// case WGPUTextureFormat_R10X6BG10X6Biplanar420Unorm:
		// case WGPUTextureFormat_R8BG8A8Triplanar420Unorm:
		// case WGPUTextureFormat_R8BG8Biplanar422Unorm:
		// case WGPUTextureFormat_R8BG8Biplanar444Unorm:
		// case WGPUTextureFormat_R10X6BG10X6Biplanar422Unorm:
		// case WGPUTextureFormat_R10X6BG10X6Biplanar444Unorm:
		// case WGPUTextureFormat_External:
		// case WGPUTextureFormat_Force32:
		default: return 0;
	}
}

wgpu::TextureFormat texture::format_remove_srgb(WGPUTextureFormat format) {
	switch(static_cast<wgpu::TextureFormat>(format)) {
		case WGPUTextureFormat_RGBA8UnormSrgb: return WGPUTextureFormat_RGBA8Unorm;
		case WGPUTextureFormat_BGRA8UnormSrgb: return WGPUTextureFormat_BGRA8Unorm;
		case WGPUTextureFormat_BC1RGBAUnormSrgb: return WGPUTextureFormat_BC1RGBAUnorm;
		case WGPUTextureFormat_BC2RGBAUnormSrgb: return WGPUTextureFormat_BC2RGBAUnorm;
		case WGPUTextureFormat_BC3RGBAUnormSrgb: return WGPUTextureFormat_BC3RGBAUnorm;
		case WGPUTextureFormat_BC7RGBAUnormSrgb: return WGPUTextureFormat_BC7RGBAUnormSrgb;
		case WGPUTextureFormat_ETC2RGB8UnormSrgb: return WGPUTextureFormat_ETC2RGB8UnormSrgb;
		case WGPUTextureFormat_ETC2RGB8A1UnormSrgb: return WGPUTextureFormat_ETC2RGB8A1UnormSrgb;
		case WGPUTextureFormat_ETC2RGBA8UnormSrgb: return WGPUTextureFormat_ETC2RGBA8UnormSrgb;
		case WGPUTextureFormat_ASTC4x4UnormSrgb: return WGPUTextureFormat_ASTC4x4Unorm;
		case WGPUTextureFormat_ASTC5x4UnormSrgb: return WGPUTextureFormat_ASTC5x4Unorm;
		case WGPUTextureFormat_ASTC5x5UnormSrgb: return WGPUTextureFormat_ASTC5x5Unorm;
		case WGPUTextureFormat_ASTC6x5UnormSrgb: return WGPUTextureFormat_ASTC6x5Unorm;
		case WGPUTextureFormat_ASTC6x6UnormSrgb: return WGPUTextureFormat_ASTC6x6Unorm;
		case WGPUTextureFormat_ASTC8x5UnormSrgb: return WGPUTextureFormat_ASTC8x5Unorm;
		case WGPUTextureFormat_ASTC8x6UnormSrgb: return WGPUTextureFormat_ASTC8x6Unorm;
		case WGPUTextureFormat_ASTC8x8UnormSrgb: return WGPUTextureFormat_ASTC8x8Unorm;
		case WGPUTextureFormat_ASTC10x5UnormSrgb: return WGPUTextureFormat_ASTC10x5Unorm;
		case WGPUTextureFormat_ASTC10x6UnormSrgb: return WGPUTextureFormat_ASTC10x6Unorm;
		case WGPUTextureFormat_ASTC10x8UnormSrgb: return WGPUTextureFormat_ASTC10x8Unorm;
		case WGPUTextureFormat_ASTC10x10UnormSrgb: return WGPUTextureFormat_ASTC10x10Unorm;
		case WGPUTextureFormat_ASTC12x10UnormSrgb: return WGPUTextureFormat_ASTC12x10Unorm;
		case WGPUTextureFormat_ASTC12x12UnormSrgb: return WGPUTextureFormat_ASTC12x12Unorm;
		default: return format;
	}
}

const char* texture::format_to_string(WGPUTextureFormat format) {
	switch(static_cast<wgpu::TextureFormat>(format)) {
		case WGPUTextureFormat_R8Unorm: return "r8unorm";
		case WGPUTextureFormat_R8Snorm: return "r8snorm";
		case WGPUTextureFormat_R8Uint: return "r8uint";
		case WGPUTextureFormat_R8Sint: return "r8sint";
		case WGPUTextureFormat_R16Uint: return "r16uint";
		case WGPUTextureFormat_R16Sint: return "r16sint";
		case WGPUTextureFormat_R16Float: return "r16float";
		case WGPUTextureFormat_RG8Unorm: return "rg8unorm";
		case WGPUTextureFormat_RG8Snorm: return "rg8snorm";
		case WGPUTextureFormat_RG8Uint: return "rg8uint";
		case WGPUTextureFormat_RG8Sint: return "rg8sint";
		case WGPUTextureFormat_R32Float: return "r32float";
		case WGPUTextureFormat_R32Uint: return "r32uint";
		case WGPUTextureFormat_R32Sint: return "r32sint";
		case WGPUTextureFormat_RG16Uint: return "rg16uint";
		case WGPUTextureFormat_RG16Sint: return "rg16sint";
		case WGPUTextureFormat_RG16Float: return "rg16float";
		case WGPUTextureFormat_RGBA8Unorm: return "rgba8unorm";
		case WGPUTextureFormat_RGBA8UnormSrgb: return "rgba8unorm";
		case WGPUTextureFormat_RGBA8Snorm: return "rgba8snorm";
		case WGPUTextureFormat_RGBA8Uint: return "rgba8uint";
		case WGPUTextureFormat_RGBA8Sint: return "rgba8sint";
		case WGPUTextureFormat_BGRA8Unorm: return "bgra8unorm";
		case WGPUTextureFormat_BGRA8UnormSrgb: return "bgra8unorm";
		case WGPUTextureFormat_RGB10A2Uint: return "rgb10a2uint";
		case WGPUTextureFormat_RGB10A2Unorm: return "rgb10a2unorm";
		case WGPUTextureFormat_RG11B10Ufloat: return "rg11b10ufloat";
		case WGPUTextureFormat_RGB9E5Ufloat: return "rgb9e5ufloat";
		case WGPUTextureFormat_RG32Float: return "rg32float";
		case WGPUTextureFormat_RG32Uint: return "rg32uint";
		case WGPUTextureFormat_RG32Sint: return "rg32sint";
		case WGPUTextureFormat_RGBA16Uint: return "rgba16uint";
		case WGPUTextureFormat_RGBA16Sint: return "rgba16sint";
		case WGPUTextureFormat_RGBA16Float: return "rgba16float";
		case WGPUTextureFormat_RGBA32Float: return "rgba32float";
		case WGPUTextureFormat_RGBA32Uint: return "rgba32uint";
		case WGPUTextureFormat_RGBA32Sint: return "rgba32sint";
		case WGPUTextureFormat_Stencil8: return "stencil8";
		case WGPUTextureFormat_Depth16Unorm: return "depth16unorm";
		case WGPUTextureFormat_Depth24Plus: return "depth24plus";
		case WGPUTextureFormat_Depth24PlusStencil8: return "depth24plusstencil8";
		case WGPUTextureFormat_Depth32Float: return "depth32float";
		case WGPUTextureFormat_Depth32FloatStencil8: return "depth32floatstencil8";
		case WGPUTextureFormat_BC1RGBAUnorm: return "bc1rgbaunorm";
		case WGPUTextureFormat_BC1RGBAUnormSrgb: return "bc1rgbaunorm";
		case WGPUTextureFormat_BC2RGBAUnorm: return "bc2rgbaunorm";
		case WGPUTextureFormat_BC2RGBAUnormSrgb: return "bc2rgbaunorm";
		case WGPUTextureFormat_BC3RGBAUnorm: return "bc3rgbaunorm";
		case WGPUTextureFormat_BC3RGBAUnormSrgb: return "bc3rgbaunorm";
		case WGPUTextureFormat_BC4RUnorm: return "bc4runorm";
		case WGPUTextureFormat_BC4RSnorm: return "bc4rsnorm";
		case WGPUTextureFormat_BC5RGUnorm: return "bc5rgunorm";
		case WGPUTextureFormat_BC5RGSnorm: return "bc5rgsnorm";
		case WGPUTextureFormat_BC6HRGBUfloat: return "bc6hrgbufloat";
		case WGPUTextureFormat_BC6HRGBFloat: return "bc6hrgbfloat";
		case WGPUTextureFormat_BC7RGBAUnorm: return "bc7rgbaunorm";
		case WGPUTextureFormat_BC7RGBAUnormSrgb: return "bc7rgbaunorm";
		case WGPUTextureFormat_ETC2RGB8Unorm: return "etc2rgb8unorm";
		case WGPUTextureFormat_ETC2RGB8UnormSrgb: return "etc2rgb8unorm";
		case WGPUTextureFormat_ETC2RGB8A1Unorm: return "etc2rgb8a1unorm";
		case WGPUTextureFormat_ETC2RGB8A1UnormSrgb: return "etc2rgb8a1unorm";
		case WGPUTextureFormat_ETC2RGBA8Unorm: return "etc2rgba8unorm";
		case WGPUTextureFormat_ETC2RGBA8UnormSrgb: return "etc2rgba8unorm";
		case WGPUTextureFormat_EACR11Unorm: return "eacr11unorm";
		case WGPUTextureFormat_EACR11Snorm: return "eacr11snorm";
		case WGPUTextureFormat_EACRG11Unorm: return "eacrg11unorm";
		case WGPUTextureFormat_EACRG11Snorm: return "eacrg11snorm";
		case WGPUTextureFormat_ASTC4x4Unorm: return "astc4x4unorm";
		case WGPUTextureFormat_ASTC4x4UnormSrgb: return "astc4x4unorm";
		case WGPUTextureFormat_ASTC5x4Unorm: return "astc5x4unorm";
		case WGPUTextureFormat_ASTC5x4UnormSrgb: return "astc5x4unorm";
		case WGPUTextureFormat_ASTC5x5Unorm: return "astc5x5unorm";
		case WGPUTextureFormat_ASTC5x5UnormSrgb: return "astc5x5unorm";
		case WGPUTextureFormat_ASTC6x5Unorm: return "astc6x5unorm";
		case WGPUTextureFormat_ASTC6x5UnormSrgb: return "astc6x5unorm";
		case WGPUTextureFormat_ASTC6x6Unorm: return "astc6x6unorm";
		case WGPUTextureFormat_ASTC6x6UnormSrgb: return "astc6x6unorm";
		case WGPUTextureFormat_ASTC8x5Unorm: return "astc8x5unorm";
		case WGPUTextureFormat_ASTC8x5UnormSrgb: return "astc8x5unorm";
		case WGPUTextureFormat_ASTC8x6Unorm: return "astc8x6unorm";
		case WGPUTextureFormat_ASTC8x6UnormSrgb: return "astc8x6unorm";
		case WGPUTextureFormat_ASTC8x8Unorm: return "astc8x8unorm";
		case WGPUTextureFormat_ASTC8x8UnormSrgb: return "astc8x8unorm";
		case WGPUTextureFormat_ASTC10x5Unorm: return "astc10x5unorm";
		case WGPUTextureFormat_ASTC10x5UnormSrgb: return "astc10x5unorm";
		case WGPUTextureFormat_ASTC10x6Unorm: return "astc10x6unorm";
		case WGPUTextureFormat_ASTC10x6UnormSrgb: return "astc10x6unorm";
		case WGPUTextureFormat_ASTC10x8Unorm: return "astc10x8unorm";
		case WGPUTextureFormat_ASTC10x8UnormSrgb: return "astc10x8unorm";
		case WGPUTextureFormat_ASTC10x10Unorm: return "astc10x10unorm";
		case WGPUTextureFormat_ASTC10x10UnormSrgb: return "astc10x10unorm";
		case WGPUTextureFormat_ASTC12x10Unorm: return "astc12x10unorm";
		case WGPUTextureFormat_ASTC12x10UnormSrgb: return "astc12x10unorm";
		case WGPUTextureFormat_ASTC12x12Unorm: return "astc12x12unorm";
		case WGPUTextureFormat_ASTC12x12UnormSrgb: return "astc12x12unorm";
		case WGPUTextureFormat_R16Unorm: return "r16unorm";
		case WGPUTextureFormat_RG16Unorm: return "rg16unorm";
		case WGPUTextureFormat_RGBA16Unorm: return "rgba16unorm";
		case WGPUTextureFormat_R16Snorm: return "r16snorm";
		case WGPUTextureFormat_RG16Snorm: return "rg16snorm";
		case WGPUTextureFormat_RGBA16Snorm: return "rgba16snorm";
		case WGPUTextureFormat_R8BG8Biplanar420Unorm: return "r8bg8biplanar420unorm";
		case WGPUTextureFormat_R10X6BG10X6Biplanar420Unorm: return "r10x6bg10x6biplanar420unorm";
		case WGPUTextureFormat_R8BG8A8Triplanar420Unorm: return "r8bg8a8triplanar420unorm";
		case WGPUTextureFormat_R8BG8Biplanar422Unorm: return "r8bg8biplanar422unorm";
		case WGPUTextureFormat_R8BG8Biplanar444Unorm: return "r8bg8biplanar444unorm";
		case WGPUTextureFormat_R10X6BG10X6Biplanar422Unorm: return "r10x6bg10x6biplanar422unorm";
		case WGPUTextureFormat_R10X6BG10X6Biplanar444Unorm: return "r10x6bg10x6biplanar444unorm";
		case WGPUTextureFormat_External: return "external";
		case WGPUTextureFormat_Force32: return "force32";
		default: return 0;
	}
}

result<texture*> texture::copy_to_buffer_record_existing(WGPUCommandEncoder encoder, gpu_buffer& buffer, size_t mip_level /* = 0 */) WAYLIB_TRY {
	assert(mip_level <= mip_levels);
	auto size = this->size() / vec2u(mip_level + 1); // TODO: Is this a valid means of compensating for mip level?

	wgpu::ImageCopyTexture source = wgpu::Default;
	source.texture = gpu_data;
	source.mipLevel = mip_level;
	wgpu::ImageCopyBuffer destination = wgpu::Default;
	destination.buffer = buffer.gpu_data;
	destination.layout.bytesPerRow = bytes_per_pixel(format()) * size.x;
	destination.layout.offset = 0;
	destination.layout.rowsPerImage = size.y;
	static_cast<wgpu::CommandEncoder>(encoder).copyTextureToBuffer(source, destination, { size.x, size.y, 1 });
	return this;
} WAYLIB_CATCH

result<image> texture::download(wgpu_state& state, image_format format /* = image_format::RGBA8 */, size_t mip_level /* = 0 */) WAYLIB_TRY {
	auto size = this->size() / vec2u(mip_level + 1); // TODO: Is this a valid means of compensating for mip level?
	gpu_buffer buffer = gpu_bufferC{.size = bytes_per_pixel(this->format()) * size.x * size.y};
	if(auto res = buffer.upload(state, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead); !res) return unexpected(res.error());
	if(auto res = copy_to_buffer(state, buffer); !res) return unexpected(res.error());
	if(auto res = buffer.download(state, false); !res) return unexpected(res.error());

	image out;
	out.format = format;
	out.gray8 = std::exchange(buffer.cpu_data, nullptr);
	out.size = size;
	out.frames = 1;
	return out;
} WAYLIB_CATCH

result<texture*> texture::generate_mipmaps(wgpu_state& state, uint32_t max_levels /* = 0 */) { // 0 -> no maximum
	auto size = this->size();
	if(max_levels == 0) max_levels = std::bit_width(std::min(size.x, size.y));
	mip_levels = max_levels;

	auto format = this->format();
	auto formatStr = std::string(format_to_string(format));
	auto tmpMipShader = shader::from_wgsl(state, R"_(
@group(0) @binding(0) var previousMipLevel: texture_2d<f32>;
@group(0) @binding(1) var nextMipLevel: texture_storage_2d<)_" + formatStr + R"_(, write>;

@compute @workgroup_size(8, 8)
fn compute(@builtin(global_invocation_id) id: vec3<u32>) {
    let offset = vec2<u32>(0, 1);
    let color = (
        textureLoad(previousMipLevel, 2 * id.xy + offset.xx, 0) +
        textureLoad(previousMipLevel, 2 * id.xy + offset.xy, 0) +
        textureLoad(previousMipLevel, 2 * id.xy + offset.yx, 0) +
        textureLoad(previousMipLevel, 2 * id.xy + offset.yy, 0)
    ) * 0.25;
    textureStore(nextMipLevel, id.xy, color);
})_", {.compute_entry_point = "compute"});
	if(!tmpMipShader) return unexpected(tmpMipShader.error());
	wl::auto_release mipShader = std::move(*tmpMipShader);

	auto newTexture = create(state, size, {.format = {format_remove_srgb(format)}, .usage = wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding, .mip_levels = mip_levels});
	if(!newTexture) return unexpected(newTexture.error());
	if(auto res = newTexture->copy(state, *this); !res) return res;

	result<wgpu::TextureView> tmpView;
	if(tmpView = newTexture->create_mip_view(0); !tmpView) return unexpected(tmpView.error());
	auto previousView = *tmpView;
	if(tmpView = newTexture->create_mip_view(1); !tmpView) return unexpected(tmpView.error());
	auto nextView = *tmpView;
	std::array<texture, 2> textures {textureC{.view = previousView}, textureC{.view = nextView}};

	computer compute = computerC{
		.buffer_count = 0,
		.texture_count = textures.size(),
		.textures = textures.data(),
		.shader = &mipShader,
	};
	compute.upload(state, {"Waylib Mipmap Computer"});

	// wgpu::Extent3D mipLevelSize = {size.x, size.y, 1}; // TOOD: do we need a tweak to properly handle cubemaps?
	for (uint32_t level = 1; level < mip_levels; ++level) {
		vec2u invocationCount = size / vec2u(2);
		constexpr uint32_t workgroupSizePerDim = 8;

		compute.dispatch(state, vec3u((invocationCount + vec2u(workgroupSizePerDim) - vec2u(1)) / vec2u(workgroupSizePerDim), 1));

		if(level < mip_levels - 1) {
			textures[0].view = textures[1].view;
			if(tmpView = newTexture->create_mip_view(level + 1); !tmpView) return unexpected(tmpView.error());
			textures[1].view = *tmpView;
			size = size / vec2u(2);
		}
	}

	if(view) newTexture->create_view();
	if(sampler) newTexture->sampler = std::exchange(sampler, nullptr);

	release();
	(*this) = std::move(*newTexture);
	return this;
}

result<drawing_state> texture::begin_drawing(wgpu_state& state, WAYLIB_OPTIONAL(colorC) clear_color /* = {} */, WAYLIB_OPTIONAL(gpu_buffer&) utility_buffer /* = {} */) WAYLIB_TRY {
	static WGPUTextureViewDescriptor viewDesc = {
		// .format = color.getFormat(),
		.dimension = wgpu::TextureViewDimension::_2D,
		.baseMipLevel = 0,
		.mipLevelCount = 1,
		.baseArrayLayer = 0,
		.arrayLayerCount = 1,
		// .aspect = wgpu::TextureAspect::DepthOnly,
	};
	static WGPUTextureFormat depthTextureFormat = wgpu::TextureFormat::Depth24Plus;
	static WGPUTextureDescriptor depthTextureDesc = {
		.usage = wgpu::TextureUsage::RenderAttachment,
		.dimension = wgpu::TextureDimension::_2D,
		.format = depthTextureFormat,
		.mipLevelCount = 1,
		.sampleCount = 1,
		.viewFormatCount = 1,
		.viewFormats = (WGPUTextureFormat*)&depthTextureFormat,
	};

	drawing_stateC out {.state = &state, .gbuffer = nullptr};
	// static frame_finalizers finalizers;

	// Create a command encoder for the draw call
	wgpu::CommandEncoderDescriptor encoderDesc = {};
	encoderDesc.label = toWGPU("Waylib Render Command Encoder");
	out.render_encoder = state.device.createCommandEncoder(encoderDesc);
	encoderDesc.label = toWGPU("Waylib Command Encoder");
	out.pre_encoder = state.device.createCommandEncoder(encoderDesc);

	static texture depthTexture = {};
	depthTextureDesc.size = {gpu_data.getWidth(), gpu_data.getHeight(), 1};
	if(!depthTexture.gpu_data || depthTexture.size() != size()) {
		if(depthTexture.gpu_data) depthTexture.gpu_data.release();
		depthTexture.gpu_data = state.device.createTexture(depthTextureDesc);
		depthTexture.create_view(wgpu::TextureAspect::DepthOnly);
	}


	// Create the render pass that clears the screen with our color
	wgpu::RenderPassDescriptor renderPassDesc = {};

	// The attachment part of the render pass descriptor describes the target texture of the pass
	wgpu::RenderPassColorAttachment colorAttachment = {};
	colorAttachment.view = view;
	colorAttachment.resolveTarget = nullptr;
	colorAttachment.loadOp = clear_color ? wgpu::LoadOp::Clear : wgpu::LoadOp::Load;
	colorAttachment.storeOp = wgpu::StoreOp::Store;
	colorAttachment.clearValue = toWGPU(clear_color ? *clear_color : colorC{0, 0, 0, 1});
	colorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

	// We now add a depth/stencil attachment:
	wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;
	depthStencilAttachment.view = /* support_depth ? */ depthTexture.view /* : nullptr */;
	// The initial value of the depth buffer, meaning "far"
	depthStencilAttachment.depthClearValue = 1.0f;
	// Operation settings comparable to the color attachment
	depthStencilAttachment.depthLoadOp = clear_color.has_value ? wgpu::LoadOp::Clear : wgpu::LoadOp::Load;
	depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
	// we could turn off writing to the depth buffer globally here
	depthStencilAttachment.depthReadOnly = false;

	// Stencil setup, mandatory but unused
	depthStencilAttachment.stencilClearValue = 0;
	// depthStencilAttachment.stencilLoadOp = clear_color.has_value ? wgpu::LoadOp::Clear : wgpu::LoadOp::Load;
	// depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
	depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Undefined;
	depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Undefined;
	depthStencilAttachment.stencilReadOnly = true;

	renderPassDesc.colorAttachmentCount = 1;
	renderPassDesc.colorAttachments = &colorAttachment;
	renderPassDesc.depthStencilAttachment = /* support_depth ? */ &depthStencilAttachment /* : nullptr */;
	renderPassDesc.timestampWrites = nullptr;

	// Create the render pass
	out.render_pass = out.render_encoder.beginRenderPass(renderPassDesc);
	static finalizer_list finalizers = {};
	out.finalizers = &finalizers;

	if(utility_buffer) out.utility_buffer = static_cast<gpu_bufferC*>(&utility_buffer.value);
	return {{std::move(out)}};
} WAYLIB_CATCH


result<texture*> texture::blit(drawing_state& draw, shader& blit_shader, bool dirty /* = false */) {
	static std::map<WGPUShaderModule, material> material_map = {};
	material* blitMaterial;
	if(material_map.contains(blit_shader.module) && !dirty)
		blitMaterial = &material_map[blit_shader.module];
	else {
		blitMaterial = &(material_map[blit_shader.module] = materialC{
			.shaders = &blit_shader,
			.shader_count = 1
		});
		auto target = texture::default_color_target_state(format());
		blitMaterial->upload(draw.state(), {&target, 1}, {{}}, {.double_sided = true});
	}
	// TODO: Frame defer release

	WGPUBindGroupEntry x;
	std::array<WGPUBindGroupEntry, 2> bindings = {};
	bindings[0].binding = 0;
	bindings[0].textureView = view;
	bindings[1].binding = 1;
	bindings[1].sampler = sampler ? sampler : default_sampler(draw.state());
	auto bindGroup = draw.state().device.createBindGroup(WGPUBindGroupDescriptor{ // TODO: Frame defer
		.layout = blitMaterial->pipeline.getBindGroupLayout(0),
		.entryCount = bindings.size(),
		.entries = bindings.data()
	});

	draw.render_pass.setPipeline(blitMaterial->pipeline);
	draw.render_pass.setBindGroup(0, bindGroup, 0, nullptr);
	draw.render_pass.draw(3, 1, 0, 0);
	return this;
}

result<texture*> texture::blit(drawing_state& draw) {
	static auto_release<shader> blitShader = {};
	if(!blitShader) {
		auto tmp = shader::from_wgsl(draw.state(), R"_(
struct vertex_output {
	@builtin(position) raw_position: vec4f,
	@location(0) uv: vec2f
};

@group(0) @binding(0) var texture: texture_2d<f32>;
@group(0) @binding(1) var sampler_: sampler;

@vertex
fn vertex(@builtin(vertex_index) vertex_index: u32) -> vertex_output {
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

@fragment
fn fragment(vert: vertex_output) -> @location(0) vec4f {
	return textureSample(texture, sampler_, vert.uv);
}
		)_", {.vertex_entry_point = "vertex", .fragment_entry_point = "fragment"});
		if(!tmp) return unexpected(tmp.error());
		blitShader = std::move(*tmp);
	}

	return blit(draw, blitShader, false);
}

result<drawing_state> texture::blit_to(wgpu_state& state, texture& target, WAYLIB_OPTIONAL(colorC) clear_color /* = {} */, WAYLIB_OPTIONAL(gpu_buffer&) utility_buffer /* = {} */) {
	auto draw = target.begin_drawing(state, {clear_color ? *clear_color : colorC(0, 0, 0, 1)}, utility_buffer);
	if(!draw) return unexpected(draw.error());
	blit(*draw);
	if(auto res = draw->draw(); !res) return unexpected(res.error());
	return draw;
}

result<drawing_state> texture::blit_to(wgpu_state& state, shader& blit_shader, texture& target, WAYLIB_OPTIONAL(colorC) clear_color /* = {} */, WAYLIB_OPTIONAL(gpu_buffer&) utility_buffer /* = {} */) {
	auto draw = target.begin_drawing(state, {clear_color ? *clear_color : colorC(0, 0, 0, 1)}, utility_buffer);
	if(!draw) return unexpected(draw.error());
	blit(*draw, blit_shader);
	if(auto res = draw->draw(); !res) return unexpected(res.error());
	return draw;
}


//////////////////////////////////////////////////////////////////////
// # Gbuffer
//////////////////////////////////////////////////////////////////////


result<wgpu::BindGroup> Gbuffer::bind_group(struct drawing_state& draw, struct material& mat) {
	auto zeroBuffer = gpu_buffer::zero_buffer(draw.state(), minimum_utility_buffer_size);
	if(!zeroBuffer) return unexpected(zeroBuffer.error());

	std::array<WGPUBindGroupEntry, 1> entries = {
		WGPUBindGroupEntry{
			.binding = 0,
			.buffer = zeroBuffer->gpu_data,
			.offset = zeroBuffer->offset,
			.size = zeroBuffer->size,
		}
	};
	if(draw.utility_buffer) {
		entries[0].buffer = draw.utility_buffer->gpu_data;
		entries[0].offset = draw.utility_buffer->offset;
		entries[0].size = draw.utility_buffer->size;
	}
	return draw.state().device.createBindGroup(WGPUBindGroupDescriptor{ // TODO: free when done somehow...
		.label = toWGPU("Waylib Compute Texture Bind Group"),
		.layout = mat.pipeline.getBindGroupLayout(0),
		.entryCount = entries.size(),
		.entries = entries.data()
	});
}

result<drawing_state> Gbuffer::begin_drawing(wgpu_state& state, WAYLIB_OPTIONAL(colorC) clear_color /* = {} */, WAYLIB_OPTIONAL(gpu_buffer&) utility_buffer /* = {} */) WAYLIB_TRY {
	drawing_stateC out {.state = &state, .gbuffer = this};
	// static frame_finalizers finalizers;

	// Create a command encoder for the draw call
	wgpu::CommandEncoderDescriptor encoderDesc = {};
	encoderDesc.label = toWGPU("Waylib Render Command Encoder");
	out.render_encoder = state.device.createCommandEncoder(encoderDesc);
	encoderDesc.label = toWGPU("Waylib Command Encoder");
	out.pre_encoder = state.device.createCommandEncoder(encoderDesc);


	// Create the render pass that clears the screen with our color
	wgpu::RenderPassDescriptor renderPassDesc = {};

	// The attachment part of the render pass descriptor describes the target texture of the pass
	std::array<wgpu::RenderPassColorAttachment, 2> colorAttachments = {};
	// Color Attachment
	colorAttachments[0].view = color().view;
	colorAttachments[0].resolveTarget = nullptr;
	colorAttachments[0].loadOp = clear_color ? wgpu::LoadOp::Clear : wgpu::LoadOp::Load;
	colorAttachments[0].storeOp = wgpu::StoreOp::Store;
	colorAttachments[0].clearValue = toWGPU(clear_color ? *clear_color : colorC{0, 0, 0, 0});
	colorAttachments[0].depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
	// Normal Attachment
	colorAttachments[1].view = normal().view;
	colorAttachments[1].resolveTarget = nullptr;
	colorAttachments[1].loadOp = clear_color ? wgpu::LoadOp::Clear : wgpu::LoadOp::Load;
	colorAttachments[1].storeOp = wgpu::StoreOp::Store;
	colorAttachments[1].clearValue = {0, 0, 0, 0};
	colorAttachments[1].depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

	// We now add a depth/stencil attachment:
	wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;
	depthStencilAttachment.view = depth().view;
	// The initial value of the depth buffer, meaning "far"
	depthStencilAttachment.depthClearValue = 1.0f;
	// Operation settings comparable to the color attachment
	depthStencilAttachment.depthLoadOp = clear_color.has_value ? wgpu::LoadOp::Clear : wgpu::LoadOp::Load;
	depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
	// we could turn off writing to the depth buffer globally here
	depthStencilAttachment.depthReadOnly = false;

	// Stencil setup, mandatory but unused
	depthStencilAttachment.stencilClearValue = 0;
	// depthStencilAttachment.stencilLoadOp = clear_color.has_value ? wgpu::LoadOp::Clear : wgpu::LoadOp::Load;
	// depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
	depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Undefined;
	depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Undefined;
	depthStencilAttachment.stencilReadOnly = true;

	renderPassDesc.colorAttachmentCount = colorAttachments.size();
	renderPassDesc.colorAttachments = colorAttachments.data();
	renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
	renderPassDesc.timestampWrites = nullptr;

	// Create the render pass
	out.render_pass = out.render_encoder.beginRenderPass(renderPassDesc);
	static finalizer_list finalizers = {};
	out.finalizers = &finalizers;

	if(utility_buffer) out.utility_buffer = static_cast<gpu_bufferC*>(&utility_buffer.value);
	return {{std::move(out)}};
} WAYLIB_CATCH


//////////////////////////////////////////////////////////////////////
// # shader
//////////////////////////////////////////////////////////////////////


shader_preprocessor& shader_preprocessor::initialize_virtual_filesystem(const config &config /* = {} */) {
	process_from_memory_and_cache(b::embed<"shaders/embeded/mesh_data.wgsl">().str(), "waylib/mesh_data", config);
	process_from_memory_and_cache(b::embed<"shaders/embeded/utility_data.wgsl">().str(), "waylib/utility_data", config);
	process_from_memory_and_cache(b::embed<"shaders/embeded/vertex_data.wgsl">().str(), "waylib/vertex_data", config);
	process_from_memory_and_cache(b::embed<"shaders/embeded/default_gbuffer_data.wgsl">().str(), "waylib/default_gbuffer_data", config);
	process_from_memory_and_cache(b::embed<"shaders/embeded/default_gbuffer.wgsl">().str(), "waylib/default_gbuffer", config);
	process_from_memory_and_cache(b::embed<"shaders/embeded/default_blit.wgsl">().str(), "waylib/default_blit", config);
	return *this;
}


std::pair<wgpu::RenderPipelineDescriptor, WAYLIB_OPTIONAL(wgpu::FragmentState)> shader::configure_render_pipeline_descriptor(
	wgpu_state& state,
	std::span<const WGPUColorTargetState> gbuffer_targets,
	WAYLIB_OPTIONAL(std::span<WGPUVertexBufferLayout>) mesh_layout /* = {} */,
	wgpu::RenderPipelineDescriptor pipelineDesc /* = {} */
) {
	// static WGPUBlendState blendState = {
	// 	.color = {
	// 		.operation = wgpu::BlendOperation::Add,
	// 		.srcFactor = wgpu::BlendFactor::SrcAlpha,
	// 		.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha,
	// 	},
	// 	.alpha = {
	// 		.operation = wgpu::BlendOperation::Add,
	// 		.srcFactor = wgpu::BlendFactor::One,
	// 		.dstFactor = wgpu::BlendFactor::Zero,
	// 	},
	// };
	// thread_local static WGPUColorTargetState colorTarget = {
	// 	.nextInChain = nullptr,
	// 	.format = state.surface_format,
	// 	.blend = &blendState,
	// 	.writeMask = wgpu::ColorWriteMask::All,
	// };


	std::span<WGPUVertexBufferLayout> bufferLayouts;
	if(mesh_layout) bufferLayouts = *mesh_layout;
	else bufferLayouts = {(WGPUVertexBufferLayout*)mesh::mesh_layout().data(), mesh::mesh_layout().size()};

	if(vertex_entry_point.value) {
		pipelineDesc.vertex.bufferCount = bufferLayouts.size();
		pipelineDesc.vertex.buffers = bufferLayouts.data();
		pipelineDesc.vertex.constantCount = 0;
		pipelineDesc.vertex.constants = nullptr;
		pipelineDesc.vertex.entryPoint = toWGPU(*vertex_entry_point);
		pipelineDesc.vertex.module = module;
	}

	if(fragment_entry_point.value) {
		static wgpu::FragmentState fragment;
		fragment.constantCount = 0;
		fragment.constants = nullptr;
		fragment.entryPoint = toWGPU(*fragment_entry_point);
		fragment.module = module;
		fragment.targetCount = gbuffer_targets.size();
		fragment.targets = gbuffer_targets.data();
		pipelineDesc.fragment = &fragment;
		return {pipelineDesc, fragment};
	}
	return {pipelineDesc, {}};
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