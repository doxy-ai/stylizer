#pragma once

#include "config.hpp"
#include <webgpu/webgpu.hpp>

#include "utility.hpp"
#include "waylib/core/optional.h"
#include "webgpu/webgpu.h"
#include "wgsl_types.hpp"
#include "shader_preprocessor.hpp"

#include <span>
#include <cstdint>
#include <chrono>
#include <sstream>
#include <filesystem>
#include <cstring>
#include <utility>
#include <concepts>

#include <glm/ext/matrix_transform.hpp> // TODO: Factor into .cpp
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_projection.hpp>
#include <glm/trigonometric.hpp>


WAYLIB_BEGIN_NAMESPACE

	#include "interfaces.h"


	static constexpr size_t minimum_utility_buffer_size = 304;


	inline constexpr WGPUStringView toWGPU(std::string_view view) { return WGPUStringView{view.data(), view.size()}; }


//////////////////////////////////////////////////////////////////////
// # thread_pool
//////////////////////////////////////////////////////////////////////

	struct WAYLIB_PREFIXED(thread_pool_future) : std::future<void> {};

	struct thread_pool {
	protected:
		static ZenSepiol::ThreadPool& get_thread_pool(WAYLIB_OPTIONAL(size_t) initial_pool_size = {})
#ifdef IS_WAYLIB_CORE_CPP
		{
			static ZenSepiol::ThreadPool pool(initial_pool_size ? *initial_pool_size : std::thread::hardware_concurrency() - 1);
			return pool;
		}
#else
		;
#endif

	public:
		template <typename F, typename... Args>
		static auto enqueue(F&& function, WAYLIB_OPTIONAL(size_t) initial_pool_size = {}, Args&&... args) {
			return get_thread_pool(initial_pool_size).AddTask(function, args...);
		}
	};


//////////////////////////////////////////////////////////////////////
// # finalizers
//////////////////////////////////////////////////////////////////////


	struct WAYLIB_PREFIXED(finalizer_list) : public std::vector<std::function<void()>> {};


//////////////////////////////////////////////////////////////////////
// # wgpu_state
//////////////////////////////////////////////////////////////////////


	wgpu::TextureFormat determine_best_surface_format(WGPUAdapter adapter, WGPUSurface surface);
	inline wgpu::TextureFormat determine_best_surface_format(wgpu_stateC state) { return determine_best_surface_format(state.adapter, state.surface); }

	wgpu::PresentMode determine_best_presentation_mode(WGPUAdapter adapter, WGPUSurface surface);
	inline wgpu::PresentMode determine_best_presentation_mode(wgpu_stateC state) { return determine_best_presentation_mode(state.adapter, state.surface); }

	struct wgpu_state: public wgpu_stateC {
		WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(wgpu_state)
		wgpu_state(wgpu_state&& other) { *this = std::move(other); }
		wgpu_state& operator=(wgpu_state&& other) {
			instance = std::exchange(other.instance, nullptr);
			adapter = std::exchange(other.adapter, nullptr);
			device = std::exchange(other.device, nullptr);
			surface = std::exchange(other.surface, nullptr);
			return *this;
		}

		static wgpu_state default_from_instance(WGPUInstance instance, WAYLIB_NULLABLE(WGPUSurface) surface = nullptr, bool prefer_low_power = false);

		static wgpu_state create_default(WAYLIB_NULLABLE(WGPUSurface) surface = nullptr, bool prefer_low_power = false) {
#ifndef __EMSCRIPTEN__
			WGPUInstance instance = wgpu::createInstance(wgpu::Default);
			if(!instance) WAYLIB_THROW("Failed to create WebGPU Instance");
#else
			WGPUInstance instance; *((size_t*)&instance) = 1;
#endif
			return default_from_instance(instance, surface, prefer_low_power);
		}

		operator bool() { return instance && adapter && device; } // NOTE: Bool conversion needed for auto_release
		void release(bool adapter_release = true, bool instance_release = true) {
			device.getQueue().release();
#ifndef __EMSCRIPTEN__
			// We expect the device to be lost when it is released... so don't do anything to stop it
			auto callback = device.setDeviceLostCallback([](wgpu::DeviceLostReason reason, wgpu::StringView message) {});
#endif
			if(surface) std::exchange(surface, nullptr).release();
			std::exchange(device, nullptr).release();
			if(adapter_release) std::exchange(adapter, nullptr).release();
			if(instance_release) std::exchange(instance, nullptr).release();
		}

		wgpu_state& configure_surface(vec2u size, surface_configuration config = {}) {
			surface_format = determine_best_surface_format(*this);
			surface.configure(WGPUSurfaceConfiguration {
				.device = device,
				.format = surface_format,
				.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopyDst,
				.alphaMode = config.alpha_mode,
				.width = size.x,
				.height = size.y,
				.presentMode = config.presentation_mode ? static_cast<wgpu::PresentMode>(*config.presentation_mode) : determine_best_presentation_mode(*this)
			});
			return *this;
		}

		struct texture current_surface_texture();

		struct drawing_state begin_drawing_to_surface(WAYLIB_OPTIONAL(colorC) clear_color = {}, WAYLIB_OPTIONAL(struct gpu_buffer&) utility_buffer = {});
	};


//////////////////////////////////////////////////////////////////////
// # Image
//////////////////////////////////////////////////////////////////////


	struct image: public imageC {
		WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(image)
		image(image&& other) { *this = std::move(other); }
		image& operator=(image&& other) {
			format = std::exchange(other.format, image_format::Invalid);
			data = std::exchange(other.data, nullptr);
			size = std::exchange(other.size, {});
			frames = std::exchange(other.frames, 0);
			return *this;
		}
		operator bool() const { return format != image_format::Invalid && *data; }

		inline void release() {
			if(data) delete [] *gray;
		}

		static wgpu::TextureFormat convert_format(image_format format) {
			switch(format){
				case image_format::RGBA: return wgpu::TextureFormat::RGBA32Float;
				case image_format::RGBA8: return wgpu::TextureFormat::RGBA8UnormSrgb;
				case image_format::Gray: return wgpu::TextureFormat::R32Float;
				case image_format::Gray8: return wgpu::TextureFormat::R8Unorm;
				default: return wgpu::TextureFormat::Undefined;
			}
		}

		static size_t bytes_per_pixel(image_format format) {
			switch(format){
				case image_format::RGBA:
					return sizeof(**rgba);
				case image_format::RGBA8:
					return sizeof(**rgba8);
				case image_format::Gray:
					return sizeof(**gray);
				case image_format::Gray8:
					return sizeof(**gray8);
				default: assert(false && "An error has occured finding the number of bytes per pixel!");
			}
		}
		inline size_t bytes_per_pixel() { return bytes_per_pixel(format); }

		struct texture upload(wgpu_state& state, texture_create_configuation config = {}, bool take_ownership_of_image = true);
	};


//////////////////////////////////////////////////////////////////////
// # Texture
//////////////////////////////////////////////////////////////////////


	struct texture: public textureC {
		WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(texture)
		texture(texture&& other) { *this = std::move(other); }
		texture& operator=(texture&& other) {
			mip_levels = std::exchange(other.mip_levels, 1);
			cpu_data = std::exchange(other.cpu_data, nullptr);
			gpu_data = std::exchange(other.gpu_data, nullptr);
			view = std::exchange(other.view, nullptr);
			sampler = std::exchange(other.sampler, nullptr);
			return *this;
		}
		operator bool() const { return gpu_data; }
		wgpu::TextureFormat format() { return gpu_data.getFormat(); }
		vec2u size() { return {gpu_data.getWidth(), gpu_data.getHeight()}; } // TODO: Does this need to return a vec3 for cubemap purposes?

		static wgpu::Sampler create_sampler(wgpu_state& state, WGPUAddressMode address_mode, WGPUFilterMode color_filter, WGPUMipmapFilterMode mipmap_filter) {
			return state.device.createSampler(WGPUSamplerDescriptor {
				.addressModeU = address_mode,
				.addressModeV = address_mode,
				.addressModeW = address_mode,
				.magFilter = color_filter,
				.minFilter = color_filter,
				.mipmapFilter = mipmap_filter,
				.lodMinClamp = 0,
				.lodMaxClamp = 1,
				.compare = wgpu::CompareFunction::Undefined,
				.maxAnisotropy = 1,
			});
		}

		static wgpu::Sampler default_sampler(wgpu_state& state) {
			static auto_release sampler = create_sampler(state, wgpu::AddressMode::Repeat, wgpu::FilterMode::Nearest, wgpu::MipmapFilterMode::Nearest);
			return sampler;
		}

		static wgpu::Sampler trilinear_sampler(wgpu_state& state) {
			static auto_release sampler = create_sampler(state, wgpu::AddressMode::Repeat, wgpu::FilterMode::Linear, wgpu::MipmapFilterMode::Linear);
			return sampler;
		}

		static size_t bytes_per_pixel(WGPUTextureFormat format);
		static wgpu::TextureFormat format_remove_srgb(WGPUTextureFormat format);
		static const char* format_to_string(WGPUTextureFormat format);

		wgpu::TextureView create_mip_view(uint32_t base_mip = 0, uint32_t mip_count = 1, wgpu::TextureAspect aspect = wgpu::TextureAspect::All) {
			assert(base_mip + mip_count <= mip_levels);
			return gpu_data.createView(WGPUTextureViewDescriptor{
				.format = format(),
				.dimension = wgpu::TextureViewDimension::_2D,
				.baseMipLevel = base_mip,
				.mipLevelCount = mip_count,
				.baseArrayLayer = 0,
				.arrayLayerCount = 1,
				.aspect = aspect
			});
		}

		texture& create_view(wgpu::TextureAspect aspect = wgpu::TextureAspect::All) {
			if(view) view.release();
			view = create_mip_view(0, 1, aspect);
			return *this;
		}

		static texture create(wgpu_state& state, vec3u size, texture_create_configuation config = {}) {
			texture out;
			out.mip_levels = config.mip_levels;
			auto format = config.format ? *config.format : wgpu::TextureFormat::RGBA8UnormSrgb;
			out.gpu_data = state.device.createTexture(WGPUTextureDescriptor {
				.label = toWGPU("Waylib Color Buffer"),
				.usage = config.usage,
				.dimension = config.dimension,
				.size = {size.x, size.y, size.z},
				.format = format,
				.mipLevelCount = config.mip_levels,
				.sampleCount = 1,
				.viewFormatCount = 1,
				.viewFormats = &format
			});

			if(config.with_view) out.create_view();
			switch(config.sampler_type) {
			break; case texture_create_sampler_type::None:
			break; case texture_create_sampler_type::Nearest:
				out.sampler = default_sampler(state);
			break; case texture_create_sampler_type::Trilinear:
				out.sampler = trilinear_sampler(state);
			}
			return out;
		}
		static texture create(wgpu_state& state, vec2u size, texture_create_configuation config = {}) {
			return create(state, vec3u(size, 1), config);
		}

		void release() {
			if(cpu_data) static_cast<image&>(**std::exchange(cpu_data, nullptr)).release();
			if(gpu_data) std::exchange(gpu_data, nullptr).release();
			if(view) std::exchange(view, nullptr).release();
			if(sampler) std::exchange(sampler, nullptr).release();
		}

		static WGPUColorTargetState default_color_target_state(wgpu::TextureFormat format) {
			static WGPUBlendState blendState = {
				.color = {
					.operation = wgpu::BlendOperation::Add,
					.srcFactor = wgpu::BlendFactor::SrcAlpha,
					.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha,
				},
				.alpha = {
					.operation = wgpu::BlendOperation::Add,
					.srcFactor = wgpu::BlendFactor::One,
					.dstFactor = wgpu::BlendFactor::Zero,
				},
			};
			return WGPUColorTargetState{
				.nextInChain = nullptr,
				.format = format,
				.blend = &blendState,
				.writeMask = wgpu::ColorWriteMask::All,
			};
		}
		WGPUColorTargetState default_color_target_state() { return default_color_target_state(gpu_data.getFormat()); }

		texture& copy_to_buffer_record_existing(WGPUCommandEncoder encoder, gpu_buffer& buffer, size_t mip_level = 0);
		texture& copy_to_buffer(wgpu_state& state, gpu_buffer& buffer, size_t mip_level = 0) {
			auto_release encoder = state.device.createCommandEncoder(wgpu::Default);
			copy_to_buffer_record_existing(encoder, buffer, mip_level);
			auto_release commands = encoder.finish(wgpu::Default);
			state.device.getQueue().submit(commands);
			return *this;
		}

		image download(wgpu_state& state, image_format format = image_format::RGBA8, size_t mip_level = 0);

		texture& copy_record_existing(WGPUCommandEncoder encoder, texture& source_texture, size_t target_mip_level = 0, size_t source_mip_level = 0) {
			assert(target_mip_level <= mip_levels);
			auto size = this->size() / vec2u(target_mip_level + 1); // TODO: Is this a valid means of compensating for mip level?

			wgpu::ImageCopyTexture source = wgpu::Default;
			source.texture = source_texture.gpu_data;
			source.mipLevel = source_mip_level;
			wgpu::ImageCopyTexture destination = wgpu::Default;
			destination.texture = gpu_data;
			destination.mipLevel = target_mip_level;
			static_cast<wgpu::CommandEncoder>(encoder).copyTextureToTexture(source, destination, {size.x, size.y, 1});
			return *this;
		}
		texture& copy(wgpu_state& state, texture& source_texture, size_t target_mip_level = 0, size_t source_mip_level = 0) {
			auto_release encoder = state.device.createCommandEncoder(wgpu::Default);
			copy_record_existing(encoder, source_texture, target_mip_level, source_mip_level);
			auto_release commands = encoder.finish(wgpu::Default);
			state.device.getQueue().submit(commands);
			return *this;
		}

		texture& generate_mipmaps(wgpu_state& state, uint32_t max_levels = 0);

		struct drawing_state begin_drawing(wgpu_state& state, WAYLIB_OPTIONAL(colorC) clear_color = {}, WAYLIB_OPTIONAL(gpu_buffer&) utility_buffer = {});

		texture& blit(struct drawing_state& draw);
		texture& blit(struct drawing_state& draw, struct shader& blit_shader, bool dirty = false);

		drawing_state blit_to(wgpu_state& state, texture& target, WAYLIB_OPTIONAL(colorC) clear_color = {}, WAYLIB_OPTIONAL(gpu_buffer&) utility_buffer = {});
		drawing_state blit_to(wgpu_state& state, struct shader& blit_shader, texture& target, WAYLIB_OPTIONAL(colorC) clear_color = {}, WAYLIB_OPTIONAL(gpu_buffer&) utility_buffer = {});
	};


//////////////////////////////////////////////////////////////////////
// # GPU Buffer (Generic)
//////////////////////////////////////////////////////////////////////


	struct gpu_buffer: public gpu_bufferC {
		WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(gpu_buffer)
		gpu_buffer(gpu_buffer&& other) { *this = std::move(other); }
		gpu_buffer& operator=(gpu_buffer&& other) {
			size = std::exchange(other.size, 0);
			offset = std::exchange(other.offset, 0);
			cpu_data = std::exchange(other.cpu_data, nullptr);
			gpu_data = std::exchange(other.gpu_data, nullptr);
			label = std::exchange(other.label, nullptr);
			return *this;
		}
		operator bool() const { return gpu_data; }

		gpu_buffer& upload(wgpu_state& state, EMSCRIPTEN_FLAGS(WGPUBufferUsage) usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage, bool free_cpu_data = false, bool mapped_at_creation = false) {
			if(gpu_data == nullptr) {
				gpu_data = state.device.createBuffer(WGPUBufferDescriptor {
					.label = toWGPU(*label ? *label : "Generic Waylib Buffer"),
					.usage = usage,
					.size = offset + size,
					.mappedAtCreation = mapped_at_creation,
				});
				if(*cpu_data) state.device.getQueue().writeBuffer(gpu_data, offset, *cpu_data, size);
			} else if(*cpu_data) state.device.getQueue().writeBuffer(gpu_data, offset, *cpu_data, size);

			if(free_cpu_data && cpu_data) delete[] *cpu_data;
			return *this;
		}
		inline std::future<gpu_buffer*> upload_async(wgpu_state& state, EMSCRIPTEN_FLAGS(WGPUBufferUsage) usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage, bool free_cpu_data = false, bool mapped_at_creation = false, WAYLIB_OPTIONAL(size_t) initial_pool_size = {}) {
			return thread_pool::enqueue([this](wgpu_state& state, EMSCRIPTEN_FLAGS(WGPUBufferUsage) usage, bool free, bool map) {
				return &upload(state, usage, free, map);
			}, initial_pool_size, state, usage, free_cpu_data, mapped_at_creation);
		}

		static gpu_buffer create(wgpu_state& state, std::span<std::byte> data, EMSCRIPTEN_FLAGS(WGPUBufferUsage) usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage, WAYLIB_OPTIONAL(std::string_view) label = {}) {
			auto out_ = gpu_bufferC{
				.size = data.size(),
				.offset = 0,
				.cpu_data = (uint8_t*)data.data(),
				.gpu_data = nullptr,
				.label = label.has_value ? cstring_from_view(label.value) : nullptr
			};
			auto out = *(gpu_buffer*)&out_;
			out.upload(state, usage, false);
			return out;
		}
		template<typename T>
		static gpu_buffer create(wgpu_state& state, std::span<T> data, EMSCRIPTEN_FLAGS(WGPUBufferUsage) usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage, WAYLIB_OPTIONAL(std::string_view) label = {}) {
			return create(state, {(std::byte*)data.data(), data.size() * sizeof(T)}, usage, label);
		}

		void release() {
			if(cpu_data) (delete[] *cpu_data, cpu_data = nullptr);
			if(gpu_data) {
				gpu_data.destroy();
				std::exchange(gpu_data, nullptr).release();
			}
			if(label) (delete[] *label, label = nullptr);
		}

		static gpu_buffer zero_buffer(wgpu_state& state, size_t minimum_size = 0, drawing_state* draw = nullptr);

		template<typename T = std::byte>
		gpu_buffer& write(wgpu_state& state, std::span<T> data, size_t offset = 0) {
			assert(data.size() * sizeof(T) + offset <= this->offset + size);
			state.device.getQueue().writeBuffer(gpu_data, this->offset + offset, data.data(), data.size() * sizeof(T));
			return *this;
		}

	protected:
		void map_impl(wgpu_state& state, WGPUMapMode mode) const {
			if(mode != wgpu::MapMode::None) {
				bool done = false;
#ifdef __EMSCRIPTEN__
				wgpuBufferMapAsync(data, mode, offset, size, closure2function_pointer([&done](wgpu::BufferMapAsyncStatus status, void* userdata) {
					done = true;
				}, WGPUBufferMapAsyncStatus{}, (void*)nullptr), nullptr);
#else
				auto future = const_cast<wgpu::Buffer*>(&gpu_data)->mapAsync2(mode, offset, size, WGPUBufferMapCallbackInfo2{
					.mode = wgpu::CallbackMode::AllowSpontaneous,
					.callback = closure2function_pointer([&done](wgpu::MapAsyncStatus status, wgpu::StringView message, void* userdata1, void* userdata2) {
						done = true;
					}, WGPUMapAsyncStatus{}, WGPUStringView{}, (void*)nullptr, (void*)nullptr)
				});
#endif
				while(!done) state.instance.processEvents();
			}
		}

	public:
		void* map_writable(wgpu_state& state, WGPUMapMode mode = wgpu::MapMode::None) {
			map_impl(state, mode);
			return gpu_data.getMappedRange(offset, size);
		}
		const void* map(wgpu_state& state, WGPUMapMode mode = wgpu::MapMode::None) const {
			map_impl(state, mode);
			return const_cast<wgpu::Buffer*>(&gpu_data)->getConstMappedRange(offset, size);
		}


		template<typename T>
		inline T& map_wriable(wgpu_state& state, WGPUMapMode mode = wgpu::MapMode::None) {
			assert(size == sizeof(T));
			return *(T*)map_writable(state, mode);
		}
		template<typename T>
		inline const T& map(wgpu_state& state, WGPUMapMode mode = wgpu::MapMode::None) const {
			assert(size == sizeof(T));
			return *(T*)map(state, mode);
		}

		template<typename T>
		inline std::span<T> map_writable_span(wgpu_state& state, WGPUMapMode mode = wgpu::MapMode::None) {
			assert((size % sizeof(T)) == 0);
			return {(T*)map_writable(state, mode), size / sizeof(T)};
		}
		template<typename T>
		inline std::span<const T> map_span(wgpu_state& state, WGPUMapMode mode = wgpu::MapMode::None) const {
			assert((size % sizeof(T)) == 0);
			return {(const T*)map(state, mode), size / sizeof(T)};
		}

		template<typename T>
		inline T& map_cpu_data() {
			assert(size == sizeof(T));
			assert(cpu_data);
			return *(T*)*cpu_data;
		}
		template<typename T>
		inline std::span<T> map_cpu_data_span() {
			assert((size % sizeof(T)) == 0);
			assert(cpu_data);
			return {(T*)*cpu_data, size / sizeof(T)};
		}

		void unmap() { gpu_data.unmap(); }

		void copy_record_existing(WGPUCommandEncoder encoder, const gpu_buffer& source) {
			static_cast<wgpu::CommandEncoder>(encoder).copyBufferToBuffer(source.gpu_data, source.offset, gpu_data, offset, size);
		}

		gpu_buffer& copy(wgpu_state& state, const gpu_buffer& source) {
			auto_release encoder = state.device.createCommandEncoder();
			copy_record_existing(encoder, source);
			auto_release commands = encoder.finish();
			state.device.getQueue().submit(commands);
			return *this;
		}
		inline std::future<gpu_buffer*> copy_async(wgpu_state& state, const gpu_buffer& source, WAYLIB_OPTIONAL(size_t) initial_pool_size = {}) {
			return thread_pool::enqueue([this](wgpu_state& state, const gpu_buffer& source) {
				return &copy(state, source);
			}, initial_pool_size, state, source);
		}

		gpu_buffer& copy_to_texture_record_existing(WGPUCommandEncoder encoder, texture& target, size_t mip_level = 0) {
			assert(mip_level <= target.mip_levels);
			auto size = target.size() / vec2u(mip_level + 1); // TODO: Is this a valid means of compensating for mip level?

			wgpu::ImageCopyBuffer source = wgpu::Default;
			source.buffer = gpu_data;
			source.layout.bytesPerRow = texture::bytes_per_pixel(target.format()) * size.x;
			source.layout.offset = 0;
			source.layout.rowsPerImage = size.y;
			wgpu::ImageCopyTexture destination = wgpu::Default;
			destination.texture = target.gpu_data;
			destination.mipLevel = mip_level;
			static_cast<wgpu::CommandEncoder>(encoder).copyBufferToTexture(source, destination, {size.x, size.y, 1});
			return *this;
		}
		gpu_buffer& copy_to_texture(wgpu_state& state, texture& target, size_t mip_level = 0) {
			auto_release encoder = state.device.createCommandEncoder();
			copy_to_texture_record_existing(encoder, target, mip_level);
			auto_release commands = encoder.finish();
			state.device.getQueue().submit(commands);
			return *this;
		}

		// You must clear a landing pad for the data (set cpu_data to null) before calling this function
		gpu_buffer& download(wgpu_state& state, bool create_intermediate_gpu_buffer = true) {
			assert(*cpu_data == nullptr);
			if(create_intermediate_gpu_buffer) {
				struct gpu_bufferC out_{
					.size = size,
					.offset = 0,
					.label = "Intermediate"
				};
				auto out = *(gpu_buffer*)&out_;
				out.upload(state, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead);
				out.copy(state, *this);
				out.download(state, false);
				cpu_data = std::exchange(out.cpu_data, nullptr);
				out.release();
			} else {
				cpu_data = {true, new uint8_t[size]};
				auto src = map(state, wgpu::MapMode::Read); // TODO: Failing to map?
				if(!src) WAYLIB_THROW("Failed to map buffer");
				memcpy(*cpu_data, src, size);
				unmap();
			}
			return *this;
		}
		inline std::future<gpu_buffer*> download_async(wgpu_state& state, bool create_intermediate_gpu_buffer = true, WAYLIB_OPTIONAL(size_t) initial_pool_size = {}) {
			return thread_pool::enqueue([this](wgpu_state& state, bool create_intermediate_gpu_buffer) {
				return &download(state, create_intermediate_gpu_buffer);
			}, initial_pool_size, state, create_intermediate_gpu_buffer);
		}

		// TODO: Add gpu_buffer->image and image->gpu_buffer functions
	};


//////////////////////////////////////////////////////////////////////
// # Gbuffer
//////////////////////////////////////////////////////////////////////


	template<typename Tgbuffer>
	concept GbufferProvider = requires(Tgbuffer gbuffer, struct drawing_state draw, struct material mat) {
		{gbuffer.targets()} -> std::convertible_to<std::span<const WGPUColorTargetState>>; // TODO: Why is std::array not convertible to std::span?
		{gbuffer.buffers()} -> std::convertible_to<std::span<const gpu_buffer>>;
		{gbuffer.textures()} -> std::convertible_to<std::span<const texture>>;
		{gbuffer.bind_group(draw, mat)} -> std::convertible_to<wgpu::BindGroup>;
	};

	struct Gbuffer: public GbufferC {
		WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(Gbuffer)
		Gbuffer(Gbuffer&& other) { *this = std::move(other); }
		Gbuffer& operator=(Gbuffer&& other) {
			c().color = std::exchange(other.c().color, {});
			c().depth = std::exchange(other.c().depth, {});
			c().normal = std::exchange(other.c().normal, {});
			return *this;
		}
		texture& color() { return *(texture*)&(c().color); }
		texture& depth() { return *(texture*)&(c().depth); }
		texture& normal() { return *(texture*)&(c().normal); }
		operator bool() { return color() && depth() && normal(); }

		static Gbuffer create_default(wgpu_state& state, vec2u size, Gbuffer_config config = {}) {
			Gbuffer out = {};
			if(config.color_format == wgpu::TextureFormat::Undefined && state.surface_format != wgpu::TextureFormat::Undefined)
				config.color_format = state.surface_format;
			out.color() = texture::create(state, size,
				{.format = config.color_format, .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc, .sampler_type = texture_create_sampler_type::None}
			);
			out.depth() = texture::create(state, size,
				{.format = config.depth_format, .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc, .sampler_type = texture_create_sampler_type::None}
			);
			out.normal() = texture::create(state, size,
				{.format = config.normal_format, .usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc, .sampler_type = texture_create_sampler_type::None}
			);
			out.previous = config.previous;
			return out;
		}

		void release() {
			if(color().gpu_data) color().release();
			if(depth().gpu_data) depth().release();
			if(normal().gpu_data) normal().release();
		}

		std::array<WGPUColorTargetState, 2> targets() {
			return std::array<WGPUColorTargetState, 2>{
				color().default_color_target_state(),
				normal().default_color_target_state(),
			};
		}

		std::span<const gpu_buffer> buffers() { return {(gpu_buffer*)nullptr, 0}; }
		std::span<const texture> textures() { return {&color(), 3}; }

		wgpu::BindGroup bind_group(struct drawing_state& draw, struct material& mat);

		Gbuffer& resize(wgpu_state& state, vec2u size) {
			auto _new = create_default(state, size, {
				.color_format = color().gpu_data.getFormat(),
				.depth_format = depth().gpu_data.getFormat(),
				.normal_format = normal().gpu_data.getFormat(),
			});

			release();
			*this = std::move(_new);
			return *this;
		}

		struct drawing_state begin_drawing(wgpu_state& state, WAYLIB_OPTIONAL(colorC) clear_color = {}, WAYLIB_OPTIONAL(gpu_buffer&) utility_buffer = {});
	};

	template<typename Tgbuffer>
	concept GbufferTargetProvider = requires(Tgbuffer gbuffer) {
		{gbuffer.targets()} -> std::convertible_to<std::span<const WGPUColorTargetState>>; // TODO: Why is std::array not convertible to std::span?
	};


//////////////////////////////////////////////////////////////////////
// # Frame State
//////////////////////////////////////////////////////////////////////


	struct drawing_state: public drawing_stateC {
		WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(drawing_state)
		drawing_state(Gbuffer&& other) { *this = std::move(other); }
		drawing_state& operator=(drawing_state&& other) {
			pre_encoder = std::exchange(other.pre_encoder, nullptr);
			render_encoder = std::exchange(other.render_encoder, nullptr);
			render_pass = std::exchange(other.render_pass, nullptr);
			finalizers = std::exchange(other.finalizers, {});
			return *this;
		}
		wgpu_state& state() { return *(wgpu_state*)c().state; }
		Gbuffer& gbuffer() { return *(Gbuffer*)c().gbuffer; }
		operator bool() { return pre_encoder && render_encoder && render_pass; }

		void release() {
			if(pre_encoder) std::exchange(pre_encoder, nullptr).release();
			if(render_encoder) std::exchange(render_encoder, nullptr).release();
			if(render_pass) std::exchange(render_pass, nullptr).release();
			if(finalizers) for(auto& finalizer: std::move(*finalizers))
				finalizer();
		}

		// Runs the provided function when the state is released
		template<typename F>
		void defer(F&& on_release) { finalizers->emplace_back(std::move(on_release)); }

		std::array<wgpu::CommandBuffer, 2> record_draw_commands() {
			assert(render_pass); assert(pre_encoder); assert(render_encoder);

			render_pass.end();

			wgpu::CommandBufferDescriptor cmdBufferDescriptor = {};
			cmdBufferDescriptor.label = toWGPU("Waylib Pre Frame Command Buffer");
			wgpu::CommandBuffer commands = pre_encoder.finish(cmdBufferDescriptor);
			cmdBufferDescriptor.label = toWGPU("Waylib Frame Render Command Buffer");
			wgpu::CommandBuffer render_commands = render_encoder.finish(cmdBufferDescriptor);
			std::exchange(pre_encoder, nullptr).release();
			std::exchange(render_encoder, nullptr).release();

			return std::array<wgpu::CommandBuffer, 2>{commands, render_commands};
		}

		drawing_state& draw_recordered(const std::array<wgpu::CommandBuffer, 2>& commands) {
			state().device.getQueue().submit(commands.size(), commands.data());
			for(auto& command: commands)
				const_cast<wgpu::CommandBuffer&>(command).release();
			return *this;
		}
		inline std::future<drawing_state*> draw_recordered_async(const std::array<wgpu::CommandBuffer, 2>& commands, WAYLIB_OPTIONAL(size_t) initial_pool_size = {}) {
			return thread_pool::enqueue([this](const std::array<wgpu::CommandBuffer, 2>& commands) {
				return &draw_recordered(commands);
			}, initial_pool_size, commands);
		}

		drawing_state& draw() {
			return draw_recordered(record_draw_commands());
		}
		inline std::future<drawing_state*> draw_async(WAYLIB_OPTIONAL(size_t) initial_pool_size = {}) {
			return thread_pool::enqueue([this]() { return &draw(); }, initial_pool_size);
		}
	};


//////////////////////////////////////////////////////////////////////
// # Shader
//////////////////////////////////////////////////////////////////////


	struct shader_preprocessor: public wgsl_preprocess::shader_preprocessor {
		using Super = wgsl_preprocess::shader_preprocessor;

		inline shader_preprocessor& initialize_platform_defines(WGPUAdapter adapter) {
			Super::initialize_platform_defines(adapter);

			defines.insert("#define WAYLIB_LIGHT_TYPE_DIRECTIONAL " "0" "\n");
			defines.insert("#define WAYLIB_LIGHT_TYPE_POINT " "1" "\n");
			defines.insert("#define WAYLIB_LIGHT_TYPE_SPOT " "2" "\n");

			return *this;
		}
		inline shader_preprocessor& initialize_platform_defines(const wgpu_state& state) {
			return initialize_platform_defines(state.adapter);
		}

		shader_preprocessor& initialize_virtual_filesystem(const config &config = {});

		// Wrappers to ensure that the proper type is returned... for chaining purposes
		inline shader_preprocessor& add_define(std::string_view name, std::string_view value) {
			Super::add_define(name, value);
			return *this;
		}
		inline shader_preprocessor& remove_define(std::string_view name) {
			Super::remove_define(name);
			return *this;
		}
	};

	struct shader: public shaderC {
		WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(shader)
		shader(shader&& other) { *this = std::move(other); }
		shader& operator=(shader&& other) {
			compute_entry_point = std::exchange(other.compute_entry_point, nullptr);
			vertex_entry_point = std::exchange(other.vertex_entry_point, nullptr);
			fragment_entry_point = std::exchange(other.fragment_entry_point, nullptr);
			module = std::exchange(other.module, nullptr);
			return *this;
		}
		operator bool() { return module; }

		static shader from_wgsl(wgpu_state& state, std::string_view wgsl_source_code, create_shader_configuration config = {}) {
			wgpu::ShaderModuleDescriptor shaderDesc;
			shaderDesc.label = config.name ? toWGPU(config.name) : WGPUStringView{};

			// We use the extension mechanism to specify the WGSL part of the shader module descriptor
#ifdef __EMSCRIPTEN__
			wgpu::ShaderModuleWGSLDescriptor shaderCodeSource;
			shaderCodeSource.chain.next = nullptr;
			shaderCodeSource.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
#else
			wgpu::ShaderSourceWGSL shaderCodeSource;
			shaderCodeSource.chain.next = nullptr;
			shaderCodeSource.chain.sType = wgpu::SType::ShaderSourceWGSL;
#endif
			shaderDesc.nextInChain = &shaderCodeSource.chain;
			if(config.preprocessor) {
				static std::string keepAlive;
				keepAlive = config.preprocessor->process_from_memory(wgsl_source_code, {
					.remove_comments = config.preprocess_config.remove_comments,
					.remove_whitespace = config.preprocess_config.remove_whitespace,
					.support_pragma_once = config.preprocess_config.support_pragma_once,
					.path = config.preprocess_config.path
				});

				shaderCodeSource.code = toWGPU(keepAlive);
			} else shaderCodeSource.code = toWGPU(wgsl_source_code);

			return {shaderC{
				.compute_entry_point = config.compute_entry_point,
				.vertex_entry_point = config.vertex_entry_point,
				.fragment_entry_point = config.fragment_entry_point,
				.module = state.device.createShaderModule(shaderDesc)
			}};
		}

		void release() {
			if(compute_entry_point.is_managed) delete std::exchange(compute_entry_point.value, nullptr);
			if(vertex_entry_point.is_managed) delete std::exchange(vertex_entry_point.value, nullptr);
			if(fragment_entry_point.is_managed) delete std::exchange(fragment_entry_point.value, nullptr);
			if(module) std::exchange(module, nullptr).release();
		}

		std::pair<wgpu::RenderPipelineDescriptor, WAYLIB_OPTIONAL(wgpu::FragmentState)> configure_render_pipeline_descriptor(
			wgpu_state& state,
			std::span<const WGPUColorTargetState> gbuffer_targets,
			WAYLIB_OPTIONAL(std::span<WGPUVertexBufferLayout>) mesh_layout = {},
			wgpu::RenderPipelineDescriptor pipelineDesc = {}
		);
	};


//////////////////////////////////////////////////////////////////////
// # Computer
//////////////////////////////////////////////////////////////////////


	struct computer: public computerC {
		WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(computer)
		computer(computer&& other) { *this = std::move(other); }
		computer& operator=(computer&& other) {
			buffer_count = std::exchange(other.buffer_count, 0);
			c().buffers = std::exchange(other.c().buffers, nullptr);
			texture_count = std::exchange(other.texture_count, 0);
			c().textures = std::exchange(other.c().textures, nullptr);
			c().shader = std::exchange(other.c().shader, nullptr);
			pipeline = std::exchange(other.pipeline, nullptr);
			return *this;
		}
		operator bool() const { return pipeline; }
		std::span<gpu_buffer> buffers() { return {(gpu_buffer*)*c().buffers, buffer_count}; }
		std::span<texture> textures() { return {(texture*)*c().textures, texture_count}; }
		struct shader& shader() { assert(c().shader.value); return *(struct shader*)*c().shader; }

		struct recording_state {
			WAYLIB_C_OR_CPP_TYPE(WGPUComputePassEncoder, wgpu::ComputePassEncoder) pass;
			WAYLIB_C_OR_CPP_TYPE(WGPUBindGroup, wgpu::BindGroup) buffer_bind_group, texture_bind_group;

			void release() {
				if(pass) std::exchange(pass, nullptr).release();
				if(buffer_bind_group) std::exchange(buffer_bind_group, nullptr).release();
				if(texture_bind_group) std::exchange(texture_bind_group, nullptr).release();
			}
		};

		computer& upload(wgpu_state& state, WAYLIB_OPTIONAL(std::string_view) label = {}) {
			wgpu::ComputePipelineDescriptor computePipelineDesc = wgpu::Default;
			computePipelineDesc.label = label ? toWGPU(*label) : toWGPU("Waylib Compute Pipeline");
			computePipelineDesc.compute.module = shader().module;
			computePipelineDesc.compute.entryPoint = toWGPU(*shader().compute_entry_point);

			if(pipeline) pipeline.release();
			pipeline = state.device.createComputePipeline(computePipelineDesc);
			return *this;
		}

		void release() {
			if(c().buffers) delete[] *std::exchange(c().buffers, {});
			if(c().textures) delete[] *std::exchange(c().textures, {});
			if(*c().shader) {
				shader().release();
				if(c().shader) delete std::exchange(c().shader, {}).value;
			}
			if(pipeline) std::exchange(pipeline, nullptr).release();
		}

		recording_state dispatch_record_existing(wgpu_state& state, WGPUCommandEncoder encoder, vec3u workgroups, WAYLIB_OPTIONAL(WGPUComputePassEncoder) existing_pass = {}, bool end_pass = true) {
			std::vector<wgpu::BindGroupEntry> entries(buffer_count + texture_count, wgpu::Default);

			if(buffer_count) {
				for(size_t i = 0; i < buffer_count; ++i) {
					entries[i].binding = i;
					entries[i].buffer = buffers()[i].gpu_data;
					entries[i].offset = buffers()[i].offset;
					entries[i].size = buffers()[i].size;
				}
			}
			if(texture_count) {
				for(size_t i = 0; i < texture_count; ++i) {
					entries[i + buffer_count].binding = i;
					entries[i + buffer_count].textureView = textures()[i].view;
				}
			}
			auto bindGroup = state.device.createBindGroup(WGPUBindGroupDescriptor{ // TODO: free when done somehow...
				.label = toWGPU("Waylib Compute Resources Bind Group"),
				.layout = pipeline.getBindGroupLayout(0),
				.entryCount = entries.size(),
				.entries = entries.data()
			});

			wgpu::ComputePassEncoder pass = existing_pass ? static_cast<wgpu::ComputePassEncoder>(*existing_pass) : static_cast<wgpu::CommandEncoder>(encoder).beginComputePass();
			{
				pass.setPipeline(pipeline);
				pass.setBindGroup(0, bindGroup, 0, nullptr);
				pass.dispatchWorkgroups(workgroups.x, workgroups.y, workgroups.z);
			}
			if(end_pass) pass.end();
			return recording_state{pass, bindGroup};
		}

		computer& dispatch(wgpu_state& state, vec3u workgroups) {
			auto encoder = state.device.createCommandEncoder();
			auto record_state = dispatch_record_existing(state, encoder, workgroups);

			state.device.getQueue().submit(encoder.finish());

			record_state.release();
			encoder.release();
			return *this;
		}
		inline std::future<computer*> dispatch_async(wgpu_state& state, vec3u workgroups, WAYLIB_OPTIONAL(size_t) initial_pool_size = {}) {
			return thread_pool::enqueue([this](wgpu_state& state, vec3u workgroups) {
				return &dispatch(state, workgroups);
			}, initial_pool_size, state, workgroups);
		}


		static computer dispatch(wgpu_state& state, std::span<gpu_buffer> gpu_buffers, std::span<texture> textures, struct shader& compute_shader, vec3u workgroups) {
			computerC compute_ {
				.buffer_count = (index_t)gpu_buffers.size(),
				.buffers = gpu_buffers.data(),
				.texture_count = (index_t)textures.size(),
				.textures = textures.data(),
				.shader = &compute_shader
			};
			auto compute = *(computer*)&compute_;
			compute.upload(state);
			compute.dispatch(state, workgroups);
			return compute;
		}
		inline static std::future<computer> dispatch_async(wgpu_state& state, std::span<gpu_buffer> gpu_buffers, std::span<texture> textures, struct shader& compute_shader, vec3u workgroups, WAYLIB_OPTIONAL(size_t) initial_pool_size = {}) {
			return thread_pool::enqueue([](wgpu_state& state, std::span<gpu_buffer> gpu_buffers, std::span<texture> textures, struct shader& compute_shader, vec3u workgroups) {
				return dispatch(state, gpu_buffers, textures, compute_shader, workgroups);
			}, initial_pool_size, state, gpu_buffers, textures, compute_shader, workgroups);
		}
	};


//////////////////////////////////////////////////////////////////////
// # Material
//////////////////////////////////////////////////////////////////////


	struct material: public materialC {
		WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(material)
		material(material&& other) { *this = std::move(other); }
		material& operator=(material&& other) {
			buffer_count = std::exchange(other.buffer_count, 0);
			c().buffers = std::exchange(other.c().buffers, nullptr);
			texture_count = std::exchange(other.texture_count, 0);
			c().textures = std::exchange(other.c().textures, nullptr);
			shader_count = std::exchange(other.shader_count, 0);
			c().shaders = std::exchange(other.c().shaders, nullptr);
			pipeline = std::exchange(other.pipeline, nullptr);
			bind_group = std::exchange(other.bind_group, nullptr);
			return *this;
		}
		std::span<gpu_buffer> buffers() { return {static_cast<gpu_buffer*>(c().buffers.value), buffer_count}; }
		std::span<texture> textures() { return {static_cast<texture*>(c().textures.value), texture_count}; }
		std::span<shader> shaders() { return {static_cast<shader*>(c().shaders.value), shader_count}; }
		operator bool() { return pipeline; }

		material& rebind_bind_group(wgpu_state& state) {
			std::vector<WGPUBindGroupEntry> entries;
			uint32_t binding = 0;
			for(auto& buffer: buffers())
				entries.emplace_back(WGPUBindGroupEntry{
					.binding = binding++,
					.buffer = buffer.gpu_data,
					.offset = buffer.offset,
					.size = buffer.size,
				});
			for(auto& texture: textures()) {
				entries.emplace_back(WGPUBindGroupEntry{
					.binding = binding++,
					.textureView = texture.view
				});
				wgpu::Sampler sampler = texture.sampler ? texture.sampler : texture::default_sampler(state);
				entries.emplace_back(WGPUBindGroupEntry{ // TODO: Failing?
					.binding = binding++,
					.sampler = sampler
				});
			}

			assert(buffer_count + texture_count);
			if(bind_group) bind_group.release();
			bind_group = state.device.createBindGroup(WGPUBindGroupDescriptor {
				.label = toWGPU("Waylib Shader Data Bind Group"),
				.layout = pipeline.getBindGroupLayout(2),
				.entryCount = entries.size(),
				.entries = entries.data()
			});
			return *this;
		}

		// TODO: This function needs to become gbuffer aware
		material& upload(
			wgpu_state& state,
			std::span<const WGPUColorTargetState> gbuffer_targets,
			WAYLIB_OPTIONAL(std::span<WGPUVertexBufferLayout>) mesh_layout = {},
			material_configuration config = {}
		) {
			// Create the render pipeline
			wgpu::RenderPipelineDescriptor pipelineDesc;
			wgpu::FragmentState fragment;
			for(size_t i = shader_count; i--; ) { // Reverse iteration ensures that lower indexed shaders take precedence
				shader& shader = shaders()[i];
				WAYLIB_OPTIONAL(wgpu::FragmentState) fragmentOpt;
				std::tie(pipelineDesc, fragmentOpt) = shader.configure_render_pipeline_descriptor(state, gbuffer_targets, mesh_layout, pipelineDesc);
				if(fragmentOpt.has_value) {
					fragment = fragmentOpt.value;
					pipelineDesc.fragment = &fragment;
				}
			}

			// Each sequence of 3 vertices is considered as a triangle
			pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;

			// We'll see later how to specify the order in which vertices should be
			// connected. When not specified, vertices are considered sequentially.
			pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;

			// The face orientation is defined by assuming that when looking
			// from the front of the face, its corner vertices are enumerated
			// in the counter-clockwise (CCW) order.
			pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW;

			// But the face orientation does not matter much because we do not
			// cull (i.e. "hide") the faces pointing away from us (which is often
			// used for optimization).
			pipelineDesc.primitive.cullMode = config.double_sided ? wgpu::CullMode::None : wgpu::CullMode::Back;

			// We setup a depth buffer state for the render pipeline
			wgpu::DepthStencilState depthStencilState = wgpu::Default;
			// Keep a fragment only if its depth is lower than the previously blended one
			depthStencilState.depthCompare = config.depth_function ? *config.depth_function : wgpu::CompareFunction::Undefined;
			// Each frame_time a fragment is blended into the target, we update the value of the Z-buffer
#ifdef __EMSCRIPTEN__
			depthStencilState.depthWriteEnabled = config.depth_function;
#else
			depthStencilState.depthWriteEnabled = config.depth_function ? wgpu::OptionalBool::True : wgpu::OptionalBool::False;
#endif
			// Store the format in a variable as later parts of the code depend on it
			depthStencilState.format = wgpu::TextureFormat::Depth24Plus; // TODO: make Gbuffer aware
			// Deactivate the stencil altogether
			depthStencilState.stencilReadMask = 0;
			depthStencilState.stencilWriteMask = 0;
			pipelineDesc.depthStencil = &depthStencilState;

			// Samples per pixel
			pipelineDesc.multisample.count = 1;
			// Default value for the mask, meaning "all bits on"
			pipelineDesc.multisample.mask = ~0u;
			// Default value as well (irrelevant for count = 1 anyways)
			pipelineDesc.multisample.alphaToCoverageEnabled = false;
			// Associate with the global layout (auto generate layout)
			pipelineDesc.layout = nullptr;

			if(pipeline) pipeline.release();
			pipeline = state.device.createRenderPipeline(pipelineDesc);

			if(texture_count + buffer_count > 0)
				rebind_bind_group(state);
			return *this;
		}
		template<GbufferTargetProvider Tgbuffer>
		material& upload(
			wgpu_state& state,
			Tgbuffer& gbuffer,
			WAYLIB_OPTIONAL(std::span<WGPUVertexBufferLayout>) mesh_layout = {},
			material_configuration config = {}
		) {
			return upload(state, gbuffer.targets(), mesh_layout, config);
		}

		void release(bool release_shaders = true) {
			if(release_shaders) for(size_t i = 0; i < shader_count; ++i)
				shaders()[i].release();
			if(c().shaders) delete [] std::exchange(c().shaders, {}).value;
			if(pipeline) std::exchange(pipeline, nullptr).release();
			if(bind_group) std::exchange(bind_group, nullptr).release();
		}
	};


//////////////////////////////////////////////////////////////////////
// # Mesh
//////////////////////////////////////////////////////////////////////


	struct mesh: public meshC {
		WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(mesh)
		mesh(mesh&& other) { *this = std::move(other); }
		mesh& operator=(mesh&& other) {
			triangle_count = std::exchange(other.triangle_count, 0);
			vertex_count = std::exchange(other.vertex_count, 0);
			positions = std::exchange(other.positions, nullptr);
			normals = std::exchange(other.normals, nullptr);
			tangents = std::exchange(other.tangents, nullptr);
			uvs = std::exchange(other.uvs, nullptr);
			cta = std::exchange(other.cta, nullptr);
			colors = std::exchange(other.colors, nullptr);
			bones = std::exchange(other.bones, nullptr);
			bone_weights = std::exchange(other.bone_weights, nullptr);
			indices = std::exchange(other.indices, nullptr);
			vertex_buffer = std::exchange(other.vertex_buffer, nullptr);
			index_buffer = std::exchange(other.index_buffer, nullptr);
			transformed_vertex_buffer = std::exchange(other.transformed_vertex_buffer, nullptr);
			transformed_index_buffer = std::exchange(other.transformed_index_buffer, nullptr);
			return *this;
		}

		static mesh fullscreen_mesh(wgpu_state& state, bool upload = true) {
			mesh out{}; out.zero();
			out.vertex_count = 3;
			out.triangle_count = 1;
			out.positions = {true, new vec4f[]{vec4f(-1, -3, .99, 1), vec4f(3, 1, .99, 1), vec4f(-1, 1, .99, 1)}};
			out.uvs = {true, new vec4f[]{vec4f(0, 2, 0, 0), vec4f(2, 0, 0, 0), vec4f(0)}};
			if(upload) out.upload(state);
			return out;
		}

		static const std::array<WGPUVertexBufferLayout, 8>& mesh_layout() {
			static WGPUVertexAttribute positionAttribute = {
				.format = wgpu::VertexFormat::Float32x4,
				.offset = 0,
				.shaderLocation = 0,
			};
			static WGPUVertexBufferLayout positionLayout = {
				.arrayStride = sizeof(vec4f),
				.stepMode = wgpu::VertexStepMode::Vertex,
				.attributeCount = 1,
				.attributes = &positionAttribute,
			};

			static WGPUVertexAttribute normalsAttribute = {
				.format = wgpu::VertexFormat::Float32x4,
				.offset = 0,
				.shaderLocation = 1,
			};
			static WGPUVertexBufferLayout normalsLayout = {
				.arrayStride = sizeof(vec4f),
				.stepMode = wgpu::VertexStepMode::Vertex,
				.attributeCount = 1,
				.attributes = &normalsAttribute,
			};

			static WGPUVertexAttribute tangentsAttribute = {
				.format = wgpu::VertexFormat::Float32x4,
				.offset = 0,
				.shaderLocation = 2,
			};
			static WGPUVertexBufferLayout tangentsLayout = {
				.arrayStride = sizeof(vec4f),
				.stepMode = wgpu::VertexStepMode::Vertex,
				.attributeCount = 1,
				.attributes = &tangentsAttribute,
			};

			static WGPUVertexAttribute uvsAttribute = {
				.format = wgpu::VertexFormat::Float32x4,
				.offset = 0,
				.shaderLocation = 3,
			};
			static WGPUVertexBufferLayout uvsLayout = {
				.arrayStride = sizeof(vec4f),
				.stepMode = wgpu::VertexStepMode::Vertex,
				.attributeCount = 1,
				.attributes = &uvsAttribute,
			};

			static WGPUVertexAttribute ctaAttribute = {
				.format = wgpu::VertexFormat::Float32x4,
				.offset = 0,
				.shaderLocation = 4,
			};
			static WGPUVertexBufferLayout ctaLayout = {
				.arrayStride = sizeof(vec4f),
				.stepMode = wgpu::VertexStepMode::Vertex,
				.attributeCount = 1,
				.attributes = &ctaAttribute,
			};

			static WGPUVertexAttribute colorsAttribute = {
				.format = wgpu::VertexFormat::Float32x4,
				.offset = 0,
				.shaderLocation = 5,
			};
			static WGPUVertexBufferLayout colorsLayout = {
				.arrayStride = sizeof(vec4f),
				.stepMode = wgpu::VertexStepMode::Vertex,
				.attributeCount = 1,
				.attributes = &colorsAttribute,
			};

			static WGPUVertexAttribute bonesAttribute = {
				.format = wgpu::VertexFormat::Uint32x4,
				.offset = 0,
				.shaderLocation = 6,
			};
			static WGPUVertexBufferLayout bonesLayout = {
				.arrayStride = sizeof(vec4u),
				.stepMode = wgpu::VertexStepMode::Vertex,
				.attributeCount = 1,
				.attributes = &bonesAttribute,
			};

			static WGPUVertexAttribute boneWeightsAttribute = {
				.format = wgpu::VertexFormat::Float32x4,
				.offset = 0,
				.shaderLocation = 7,
			};
			static WGPUVertexBufferLayout boneWeightsLayout = {
				.arrayStride = sizeof(vec4f),
				.stepMode = wgpu::VertexStepMode::Vertex,
				.attributeCount = 1,
				.attributes = &boneWeightsAttribute,
			};

			static std::array<WGPUVertexBufferLayout, 8> layouts = {positionLayout, normalsLayout, tangentsLayout, uvsLayout, ctaLayout, colorsLayout, bonesLayout, boneWeightsLayout};
			return layouts;
		}

		mesh_metadata metadata() {
			mesh_metadata meta = {
				.is_indexed = (bool)index_buffer,
				.vertex_count = vertex_count,
				.triangle_count = triangle_count,

				.position_start = sizeof(mesh_metadata), // Not accounting for metadata size... on gpu this is correct, on cpu need to add sizeof(mesh_metadata)
				// Others should default to zero :)
			};
			size_t offset = vertex_count * sizeof(vec4f) + sizeof(mesh_metadata); // Account for position and metadata
			if(*normals) (meta.normals_start = offset, offset += vertex_count * sizeof(vec4f));
			if(*tangents) (meta.tangents_start = offset, offset += vertex_count * sizeof(vec4f));
			if(*uvs) (meta.uvs_start = offset, offset += vertex_count * sizeof(vec4f));
			if(*cta) (meta.cta_start = offset, offset += vertex_count * sizeof(vec4f));
			if(*colors) (meta.colors_start = offset, offset += vertex_count * sizeof(vec4f));
			if(*bones) (meta.bones_start = offset, offset += vertex_count * sizeof(vec4u));
			if(*bone_weights) (meta.bone_weights_start = offset, offset += vertex_count * sizeof(vec4f));
			meta.vertex_buffer_size = offset;
			return meta;
		}

		mesh_metadata gpu_metadata() {
			auto meta = metadata();
			meta.position_start -= sizeof(mesh_metadata);
			if(meta.normals_start > 0) meta.normals_start -= sizeof(mesh_metadata);
			if(meta.tangents_start > 0) meta.tangents_start -= sizeof(mesh_metadata);
			if(meta.uvs_start > 0) meta.uvs_start -= sizeof(mesh_metadata);
			if(meta.cta_start > 0) meta.cta_start -= sizeof(mesh_metadata);
			if(meta.colors_start > 0) meta.colors_start -= sizeof(mesh_metadata);
			if(meta.bones_start > 0) meta.bones_start -= sizeof(mesh_metadata);
			if(meta.bone_weights_start > 0) meta.bone_weights_start -= sizeof(mesh_metadata);
			return meta;
		}

		mesh& upload(wgpu_state& state) {
			auto metadata = this->metadata();
			auto gpuMetadata = this->gpu_metadata();
			size_t attribute_size = metadata.vertex_count * sizeof(vec4f);

			gpu_buffer buffer = gpu_bufferC{
				.size = metadata.vertex_buffer_size,
				.label = "Waylib Vertex Buffer"
			};
			buffer.upload(state, wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Storage, false, true);
			std::byte* mapped = (std::byte*)buffer.map_writable(state);

			// Upload geometry data to the buffer
			memcpy(mapped, &gpuMetadata, sizeof(metadata));
			memcpy(mapped + metadata.position_start, *positions, attribute_size);
			if(*normals) memcpy(mapped + metadata.normals_start, *normals, attribute_size);
			if(*tangents) memcpy(mapped + metadata.tangents_start, *tangents, attribute_size);
			if(*uvs) memcpy(mapped + metadata.uvs_start, *uvs, attribute_size);
			if(*cta) memcpy(mapped + metadata.cta_start, *cta, attribute_size);
			if(*colors) memcpy(mapped + metadata.colors_start, *colors, attribute_size);
			if(*bones) memcpy(mapped + metadata.bones_start, *bones, attribute_size);
			if(*bone_weights) memcpy(mapped + metadata.bone_weights_start, *bone_weights, attribute_size);
			buffer.unmap();

			gpu_buffer indexBuffer{};
			if(*indices)
				indexBuffer = gpu_buffer::create(state, {(std::byte*)*indices, triangle_count * sizeof(index_t) * 3}, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index, {"Waylib Index Buffer"});

			if(vertex_buffer) vertex_buffer.release();
			vertex_buffer = buffer.gpu_data;
			if(index_buffer) index_buffer.release();
			index_buffer = indexBuffer.gpu_data;
			return *this;
		}
	};


//////////////////////////////////////////////////////////////////////
// # Model
//////////////////////////////////////////////////////////////////////


	struct model: public modelC {
		WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(model)
		model(model&& other) { *this = std::move(other); }
		model& operator=(model&& other) {
			transform = std::exchange(other.transform, glm::identity<glm::mat4x4>());
			mesh_count = std::exchange(other.mesh_count, 0);
			c().meshes = std::exchange(other.c().meshes, nullptr);
			material_count = std::exchange(other.material_count, 0);
			c().materials = std::exchange(other.c().materials, nullptr);
			c().mesh_materials = std::exchange(other.c().mesh_materials, nullptr);
			return *this;
		}
		operator bool() { return *c().meshes && *c().materials; }
		std::span<mesh> meshes() { return {static_cast<mesh*>(*c().meshes), mesh_count}; }
		std::span<material> materials() { return {static_cast<material*>(*c().materials), material_count}; }
		std::span<index_t> mesh_materials() { return {static_cast<index_t*>(*c().mesh_materials), mesh_count}; }

		index_t get_material_index_for_mesh(index_t i) {
			if(*c().mesh_materials == nullptr) return i;
			return mesh_materials()[i];
		}
		material& get_material_for_mesh(index_t i) {
			i = get_material_index_for_mesh(i);
			assert(i < material_count);
			return materials()[i];
		}

		model& upload(
			wgpu_state& state,
			std::span<const WGPUColorTargetState> gbuffer_targets,
			WAYLIB_OPTIONAL(std::span<WGPUVertexBufferLayout>) mesh_layout = {},
			material_configuration material_config = {}
		) {
			for(auto& mesh: meshes())
				mesh.upload(state);
			for(auto& material: materials())
				material.upload(state, gbuffer_targets, mesh_layout, material_config);
			return *this;
		}
		template<GbufferTargetProvider Tgbuffer>
		model& upload(
			wgpu_state& state,
			Tgbuffer& gbuffer,
			WAYLIB_OPTIONAL(std::span<WGPUVertexBufferLayout>) mesh_layout = {},
			material_configuration material_config = {}
		) { return upload(state, gbuffer.targets(), mesh_layout, material_config); }

		void release() {
			// TODO: Implement
		}

		model& draw_instanced(drawing_state& draw, std::span<model_instance> instances) {
			std::array<WGPUBindGroupEntry, 3> bindings = {
				WGPUBindGroupEntry{ // Mesh (metadata + vertex attributes)
					.binding = 0,
					// .buffer = mesh.vertex_buffer,
					.offset = 0,
					// .size = metadata.vertex_buffer_size
				}, WGPUBindGroupEntry{ // Indicies
					.binding = 1,
					// .buffer = metadata.is_indexed ? mesh.index_buffer : zeroBuffer->data,
					.offset = 0,
					// .size = index_size
				}, WGPUBindGroupEntry{ // Instances
					.binding = 2,
					// .buffer = instanceBuffer,
					.offset = 0,
					// .size = instanceBuffer.size
				}
			};

			if(instances.size() > 0) {
				gpu_buffer instanceBuffer = gpu_bufferC{.size = std::max<size_t>(instances.size() * sizeof(model_instance), 144), .label = "Waylib Instance Buffer"};
				instanceBuffer.upload(draw.state());
				instanceBuffer.write(draw.state(), instances);
				draw.defer([instanceBuffer]{ const_cast<gpu_buffer&>(instanceBuffer).release(); });

				bindings[2].buffer = instanceBuffer.gpu_data;
				bindings[2].size = instanceBuffer.size;
			} else {
				auto zeroBuffer = gpu_buffer::zero_buffer(draw.state(), sizeof(model_instance), &draw);

				bindings[2].buffer = zeroBuffer.gpu_data;
				bindings[2].size = sizeof(model_instance);
			}

			for(size_t i = 0; i < mesh_count; ++i) {
				// Select which render pipeline to use
				auto& mat = get_material_for_mesh(i);
				draw.render_pass.setPipeline(mat.pipeline);

				auto& mesh = meshes()[i];
				auto metadata = mesh.metadata();
				size_t attribute_size = metadata.vertex_count * sizeof(vec4f);
				size_t index_size = metadata.triangle_count * 3 * sizeof(index_t);
				auto zeroBuffer = gpu_buffer::zero_buffer(draw.state(), std::max<size_t>(attribute_size, index_size), &draw); // TODO: Why is the min index size 144?

				auto drawTimeBindGroup = draw.gbuffer().bind_group(draw, mat);
				draw.render_pass.setBindGroup(0, drawTimeBindGroup, 0, nullptr);
				// draw.defer([group = *drawTimeBindGroup]{ /* const_cast<wgpu::BindGroup&>(group).release(); */ });

				bindings[0].buffer = mesh.vertex_buffer;
				bindings[0].size = metadata.vertex_buffer_size;
				bindings[1].buffer = metadata.is_indexed ? mesh.index_buffer : zeroBuffer.gpu_data;
				bindings[1].size = metadata.is_indexed ? index_size : zeroBuffer.size;

				auto perMeshBindGroup = draw.state().device.createBindGroup(WGPUBindGroupDescriptor {
					.label = toWGPU("Waylib Mesh Data Bind Group"),
					.layout = mat.pipeline.getBindGroupLayout(1),
					.entryCount = bindings.size(),
					.entries = bindings.data()
				});
				draw.render_pass.setBindGroup(1, perMeshBindGroup, 0, nullptr);
				// draw.defer([perMeshBindGroup]{ /* const_cast<wgpu::BindGroup&>(perMeshBindGroup).release(); */ });

				if(mat.bind_group)
					draw.render_pass.setBindGroup(2, mat.bind_group, 0, nullptr);

				// Position
				draw.render_pass.setVertexBuffer(0, mesh.vertex_buffer, metadata.position_start, attribute_size);
				// Normals
				draw.render_pass.setVertexBuffer(1, mesh.normals ? mesh.vertex_buffer : zeroBuffer.gpu_data, metadata.normals_start, attribute_size);
				// Tangents
				draw.render_pass.setVertexBuffer(2, mesh.tangents ? mesh.vertex_buffer : zeroBuffer.gpu_data, metadata.tangents_start, attribute_size);
				// UVs
				draw.render_pass.setVertexBuffer(3, mesh.uvs ? mesh.vertex_buffer : zeroBuffer.gpu_data, metadata.uvs_start, attribute_size);
				// CTA
				draw.render_pass.setVertexBuffer(4, mesh.cta ? mesh.vertex_buffer : zeroBuffer.gpu_data, metadata.cta_start, attribute_size);
				// Colors
				draw.render_pass.setVertexBuffer(5, mesh.colors ? mesh.vertex_buffer : zeroBuffer.gpu_data, metadata.colors_start, attribute_size);
				// Bones
				draw.render_pass.setVertexBuffer(6, mesh.bones ? mesh.vertex_buffer : zeroBuffer.gpu_data, metadata.bones_start, attribute_size);
				// Bone Weights
				draw.render_pass.setVertexBuffer(7, mesh.bone_weights ? mesh.vertex_buffer : zeroBuffer.gpu_data, metadata.bone_weights_start, attribute_size);

				if(mesh.index_buffer) {
					draw.render_pass.setIndexBuffer(mesh.index_buffer, wgpu::IndexFormat::Uint32, 0, index_size);
					draw.render_pass.drawIndexed(metadata.triangle_count * 3, std::max<size_t>(instances.size(), 1), 0, 0, 0);
				} else
					draw.render_pass.draw(metadata.vertex_count, std::max<size_t>(instances.size(), 1), 0, 0);
			}
			return *this;
		}

		model& draw(drawing_state& draw) {
			model_instance instance{
				.transform = transform,
				.tint = {}
			};
			return draw_instanced(draw, {&instance, 1});
		}
	};


//////////////////////////////////////////////////////////////////////
// # Utility Data
//////////////////////////////////////////////////////////////////////


	struct time: public timeC {
		WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(time)
		time(model&& other) { *this = std::move(other); }
		time& operator=(time&& other) {
			since_start = std::exchange(since_start, 0);
			delta = std::exchange(delta, 0);
			average_delta = std::exchange(average_delta, 0);
			return *this;
		}

		inline time& calculate_only_delta() {
			static auto last = std::chrono::steady_clock::now();

			auto now = std::chrono::steady_clock::now();
			delta = std::chrono::duration_cast<std::chrono::microseconds>(now - last).count() / 1'000'000.0;

			last = now;
			return *this;
		}

		inline time& calculate() {
			calculate_only_delta();

			since_start += delta;

			constexpr static float alpha = .9;
			if(std::abs(average_delta) < 2 * std::numeric_limits<float>::epsilon()) average_delta = delta;
			average_delta = average_delta * alpha + delta * (1 - alpha);
			return *this;
		}

		gpu_buffer update_utility_buffer(wgpu_state& state, gpu_buffer utility_buffer = {}) {
			// struct utility_data {
			// 	frame_time_data time; // at byte offset 0
			// 	float _pad0;
			// 	camera3D_data camera; // at byte offset 16
			// 	std::vector<light_data> lights; // at byte offset 208
			// };

			if(!utility_buffer) {
				std::vector<std::byte> zeroData(minimum_utility_buffer_size, std::byte{0});
				utility_buffer = gpu_buffer::create(state, zeroData, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage, {"Waylib Utility Data Buffer"});
			}

			utility_buffer.write<time>(state, {this, 1}, 0);
			return utility_buffer;
		}
	};

	struct camera3D: public camera3DC {
		WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(camera3D)
		camera3D(camera3D&& other) { *this = std::move(other); }
		camera3D& operator=(camera3D&& other) {
			view_matrix = std::exchange(other.view_matrix, {});
			projection_matrix = std::exchange(other.projection_matrix, {});
			position = std::exchange(other.position, {});
			target_position = std::exchange(other.target_position, {});
			up = std::exchange(other.up, {});
			field_of_view = std::exchange(other.field_of_view, 0);
			near_clip_distance = std::exchange(other.near_clip_distance, 0);
			far_clip_distance = std::exchange(other.far_clip_distance, 0);
			orthographic = std::exchange(other.orthographic, 0);
			return *this;
		}

		camera3D& calculate_matricies(vec2u window_size) {
			view_matrix = glm::lookAt(position, target_position, up);
			if(orthographic) {
				float right = field_of_view / 2;
				float top = right * window_size.y / window_size.x;
				projection_matrix = glm::ortho<float>(-right, right, -top, top, near_clip_distance, far_clip_distance);
			} else
				projection_matrix = glm::perspectiveFov<float>(field_of_view, window_size.x, window_size.y, near_clip_distance, far_clip_distance);
			return *this;
		}

		gpu_buffer update_utility_buffer(wgpu_state& state, gpu_buffer utility_buffer = {}) {
			// struct utility_data {
			// 	frame_time_data time; // at byte offset 0
			// 	float _pad0;
			// 	camera3D_data camera; // at byte offset 16
			// 	std::vector<light_data> lights; // at byte offset 208
			// };

			if(!utility_buffer) {
				std::vector<std::byte> zeroData(minimum_utility_buffer_size, std::byte{0});
				utility_buffer = gpu_buffer::create(state, zeroData, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage, {"Waylib Utility Data Buffer"});
			}

			utility_buffer.write<camera3D>(state, {this, 1}, 16);
			return utility_buffer;
		}
	};

	static_assert(sizeof(camera3DC) == sizeof(camera2DC), "The two camera classes need to have the same size in bytes");

	struct camera2D: public camera2DC {
		WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(camera2D)
		camera2D(camera2D&& other) { *this = std::move(other); }
		camera2D& operator=(camera2D&& other) {
			view_matrix = std::exchange(other.view_matrix, {});
			projection_matrix = std::exchange(other.projection_matrix, {});
			offset = std::exchange(other.offset, {});
			target_position = std::exchange(other.target_position, {});
			rotation = std::exchange(other.rotation, 0);
			near_clip_distance = std::exchange(other.near_clip_distance, 0);
			far_clip_distance = std::exchange(other.far_clip_distance, 0);
			zoom = std::exchange(other.zoom, 0);
			pixel_perfect = std::exchange(other.pixel_perfect, 0);
			return *this;
		}

		camera2D& calculate_matricies(vec2u window_size) {
			vec3f position = {target_position.x, target_position.y, -1};
			vec4f up = {0, 1, 0, 0};
			up = glm::rotate(glm::identity<glm::mat4x4>(), glm::radians(rotation)/* .radian_value() */, vec3f{0, 0, 1}) * up;
			if(pixel_perfect) projection_matrix = glm::ortho<float>(0, window_size.x, window_size.y, 0, near_clip_distance, far_clip_distance);
			else projection_matrix = glm::ortho<float>(0, 1/zoom, 1/zoom, 0, near_clip_distance, far_clip_distance);
			view_matrix = glm::lookAt<float>(position, position + vec3f{0, 0, 1}, up.xyz());
			return *this;
		}

		gpu_buffer update_utility_buffer(wgpu_state& state, gpu_buffer utility_buffer = {}) {
			// struct utility_data {
			// 	frame_time_data time; // at byte offset 0
			// 	float _pad0;
			// 	camera3D_data camera; // at byte offset 16
			// 	std::vector<light_data> lights; // at byte offset 208
			// };

			if(!utility_buffer) {
				std::vector<std::byte> zeroData(minimum_utility_buffer_size, std::byte{0});
				utility_buffer = gpu_buffer::create(state, zeroData, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage, {"Waylib Utility Data Buffer"});
			}

			utility_buffer.write<camera2D>(state, {this, 1}, 16);
			return utility_buffer;
		}
	};

	struct light: public lightC {
		WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(light)
		light(light&& other) { *this = std::move(other); }
		light& operator=(light&& other) {
			light_type = std::exchange(other.light_type, {});
			intensity = std::exchange(other.intensity, 0);
			cutoff_start_angle = std::exchange(other.cutoff_start_angle, 0);
			cutoff_end_angle = std::exchange(other.cutoff_end_angle, 0);
			falloff = std::exchange(other.falloff, 0);
			position = std::exchange(other.position, {});
			direction = std::exchange(other.direction, {});
			color = std::exchange(other.color, {});
			return *this;
		}

		static gpu_buffer update_utility_buffer(wgpu_state& state, std::span<light> lights, gpu_buffer utility_buffer = {}, bool skip_resize_copy = false) {
			// struct utility_data {
			// 	frame_time_data time; // at byte offset 0
			// 	float _pad0;
			// 	camera3D_data camera; // at byte offset 16
			// 	std::vector<light_data> lights; // at byte offset 208
			// };

			if(!utility_buffer || (utility_buffer.size - 208) / sizeof(light) != lights.size()) {
				std::vector<std::byte> zeroData(208 + sizeof(light) * lights.size(), std::byte{0});
				auto res = gpu_buffer::create(state, zeroData, wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage, {"Waylib Utility Data Buffer"});
				if(!skip_resize_copy)
					res.copy(state, utility_buffer);
				if(utility_buffer) utility_buffer.release();
				utility_buffer = res;
			}

			utility_buffer.write<light>(state, lights, 208);
			return utility_buffer;
		}
		gpu_buffer update_utility_buffer(wgpu_state& state, gpu_buffer utility_buffer = {}, bool skip_resize_copy = false) {
			return update_utility_buffer(state, {this, 1}, utility_buffer, skip_resize_copy);
		}
	};


	extern "C" {
		#include "core.h"
	}

WAYLIB_END_NAMESPACE