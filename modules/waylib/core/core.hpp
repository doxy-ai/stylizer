#pragma once

#include "config.hpp"
#include <webgpu/webgpu.hpp>
#include <glm/ext/matrix_transform.hpp> // TODO: Factor into .cpp

#include "utility.hpp"
#include "waylib/core/optional.h"
#include "wgsl_types.hpp"
#include "shader_preprocessor.hpp"

#include <span>
#include <cstdint>
#include <sstream>
#include <filesystem>


WAYLIB_BEGIN_NAMESPACE

	#include "interfaces.h"

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

		static result<wgpu_state> default_from_instance(WGPUInstance instance, WAYLIB_NULLABLE(WGPUSurface) surface = nullptr, bool prefer_low_power = false);

		static result<wgpu_state> create_default(WAYLIB_NULLABLE(WGPUSurface) surface = nullptr, bool prefer_low_power = false) WAYLIB_TRY {
#ifndef __EMSCRIPTEN__
			WGPUInstance instance = wgpu::createInstance(wgpu::Default);
			if(!instance) return unexpected("Failed to create WebGPU Instance");
#else
			WGPUInstance instance; *((size_t*)&instance) = 1;
#endif
			return default_from_instance(instance, surface, prefer_low_power);
		} WAYLIB_CATCH

		operator bool() { return instance && adapter && device; } // NOTE: Bool conversion needed for auto_release
		void release(bool adapter_release = true, bool instance_release = true) {
			device.getQueue().release();
#ifndef __EMSCRIPTEN__
			// We expect the device to be lost when it is released... so don't do anything to stop it
			auto callback = device.setDeviceLostCallback([](WGPUDeviceLostReason reason, char const* message) {});
#endif
			device.release();
			if(surface) surface.release();
			if(adapter_release) adapter.release();
			if(instance_release) instance.release();
		}

		result<void> configure_surface(vec2u size, surface_configuration config = {}) {
			surface_format = determine_best_surface_format(*this);
			surface.configure(WGPUSurfaceConfiguration {
				.device = device,
				.format = surface_format,
				.usage = wgpu::TextureUsage::RenderAttachment,
				.alphaMode = config.alpha_mode,
				.width = size.x,
				.height = size.y,
				.presentMode = config.presentation_mode ? static_cast<wgpu::PresentMode>(*config.presentation_mode) : determine_best_presentation_mode(*this)
			});
			return result<void>::success;
		}

		result<struct texture> current_surface_texture();

		result<struct drawing_state> begin_drawing_to_surface(WAYLIB_OPTIONAL(colorC) clear_color = {});
	};


//////////////////////////////////////////////////////////////////////
// # Texture
//////////////////////////////////////////////////////////////////////


	struct texture: public textureC {
		WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(texture)
		texture(texture&& other) { *this = std::move(other); }
		texture& operator=(texture&& other) {
			gpu_texture = std::exchange(other.gpu_texture, nullptr);
			view = std::exchange(other.view, nullptr);
			sampler = std::exchange(other.sampler, nullptr);
			return *this;
		}
		operator bool() const { return gpu_texture; }
		wgpu::TextureFormat format() { return gpu_texture.getFormat(); }

		void release() {
			if(gpu_texture) gpu_texture.release();
			if(view) view.release();
			if(sampler) sampler.release();
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
		WGPUColorTargetState default_color_target_state() { return default_color_target_state(gpu_texture.getFormat()); }

		static wgpu::Sampler default_sampler(wgpu_state& state) {
			static auto_release sampler = state.device.createSampler(WGPUSamplerDescriptor {
				.addressModeU = wgpu::AddressMode::Repeat,
				.addressModeV = wgpu::AddressMode::Repeat,
				.addressModeW = wgpu::AddressMode::Repeat,
				.magFilter = wgpu::FilterMode::Nearest,
				.minFilter = wgpu::FilterMode::Nearest,
				.mipmapFilter = wgpu::MipmapFilterMode::Nearest,
				.lodMinClamp = 0,
				.lodMaxClamp = 1,
				.compare = wgpu::CompareFunction::Undefined,
				.maxAnisotropy = 1,
			});
			return sampler;
		}

		static wgpu::Sampler trilinear_sampler(wgpu_state& state) {
			static auto_release sampler = state.device.createSampler(WGPUSamplerDescriptor {
				.addressModeU = wgpu::AddressMode::Repeat,
				.addressModeV = wgpu::AddressMode::Repeat,
				.addressModeW = wgpu::AddressMode::Repeat,
				.magFilter = wgpu::FilterMode::Linear,
				.minFilter = wgpu::FilterMode::Linear,
				.mipmapFilter = wgpu::MipmapFilterMode::Linear,
				.lodMinClamp = 0,
				.lodMaxClamp = 1,
				.compare = wgpu::CompareFunction::Undefined,
				.maxAnisotropy = 1,
			});
			return sampler;
		}

		result<void> maybe_create_view(wgpu::TextureAspect aspect = wgpu::TextureAspect::All) WAYLIB_TRY {
			if(view) return result<void>::success;

			view = gpu_texture.createView(WGPUTextureViewDescriptor{
				.format = format(),
				.dimension = wgpu::TextureViewDimension::_2D,
				.baseMipLevel = 0,
				.mipLevelCount = 1,
				.baseArrayLayer = 0,
				.arrayLayerCount = 1,
				.aspect = aspect
			});
			return result<void>::success;
		} WAYLIB_CATCH

		result<struct drawing_state> begin_drawing(wgpu_state& state, WAYLIB_OPTIONAL(colorC) clear_color = {});

		result<texture*> blit(struct drawing_state& draw);
		result<texture*> blit(struct drawing_state& draw, struct shader& blit_shader, bool dirty = false);

		result<void> blit_to(wgpu_state& state, texture& target, WAYLIB_OPTIONAL(colorC) clear_color = {});
		result<void> blit_to(wgpu_state& state, struct shader& blit_shader, texture& target, WAYLIB_OPTIONAL(colorC) clear_color = {});
	};


//////////////////////////////////////////////////////////////////////
// # Gbuffer
//////////////////////////////////////////////////////////////////////


	struct Gbuffer : public GbufferC {
		WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(Gbuffer)
		Gbuffer(Gbuffer&& other) { *this = std::move(other); }
		Gbuffer& operator=(Gbuffer&& other) {
			GbufferC::color = std::exchange(other.GbufferC::color, {});
			GbufferC::depth = std::exchange(other.GbufferC::depth, {});
			GbufferC::normal = std::exchange(other.GbufferC::normal, {});
			GbufferC::ctl = std::exchange(other.GbufferC::ctl, {});
			GbufferC::masks = std::exchange(other.GbufferC::masks, {});
			GbufferC::barycentric = std::exchange(other.GbufferC::barycentric, {});
			return *this;
		}
		texture& color() { return *(texture*)&(GbufferC::color); }
		texture& depth() { return *(texture*)&(GbufferC::depth); }
		texture& normal() { return *(texture*)&(GbufferC::normal); }
		texture& ctl() { return *(texture*)&(GbufferC::ctl); }
		texture& masks() { return *(texture*)&(GbufferC::masks); }
		texture& barycentric() { return *(texture*)&(GbufferC::barycentric); }
		operator bool() { return color() && depth() && normal(); }

		static result<Gbuffer> create_default(wgpu_state& state, vec2u size, Gbuffer_config config = {}) WAYLIB_TRY {
			Gbuffer out = {};
			if(config.color_format == wgpu::TextureFormat::Undefined && state.surface_format != wgpu::TextureFormat::Undefined)
				config.color_format = state.surface_format;
			out.color().gpu_texture = state.device.createTexture(WGPUTextureDescriptor {
				.label = "Waylib Color Buffer",
				.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc,
				.dimension = wgpu::TextureDimension::_2D,
				.size = {size.x, size.y, 1},
				.format = config.color_format,
				.mipLevelCount = 1,
				.sampleCount = 1,
				.viewFormatCount = 1,
				.viewFormats = &config.color_format
			});
			out.color().maybe_create_view();
			out.depth().gpu_texture = state.device.createTexture(WGPUTextureDescriptor {
				.label = "Waylib Depth Buffer",
				.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc,
				.dimension = wgpu::TextureDimension::_2D,
				.size = {size.x, size.y, 1},
				.format = config.depth_format,
				.mipLevelCount = 1,
				.sampleCount = 1,
				.viewFormatCount = 1,
				.viewFormats = &config.depth_format
			});
			out.depth().maybe_create_view(wgpu::TextureAspect::DepthOnly);
			out.normal().gpu_texture = state.device.createTexture(WGPUTextureDescriptor {
				.label = "Waylib Normal Buffer",
				.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc,
				.dimension = wgpu::TextureDimension::_2D,
				.size = {size.x, size.y, 1},
				.format = config.normal_format,
				.mipLevelCount = 1,
				.sampleCount = 1,
				.viewFormatCount = 1,
				.viewFormats = &config.normal_format
			});
			out.normal().maybe_create_view();
			if(config.ctl_format != wgpu::TextureFormat::Undefined) {
				out.ctl().gpu_texture = state.device.createTexture(WGPUTextureDescriptor {
					.label = "Waylib Curvature, Thickness, Lighting Buffer",
					.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc,
					.dimension = wgpu::TextureDimension::_2D,
					.size = {size.x, size.y, 1},
					.format = config.ctl_format,
					.mipLevelCount = 1,
					.sampleCount = 1,
					.viewFormatCount = 1,
					.viewFormats = &config.ctl_format
				});
				out.ctl().maybe_create_view();
			}
			if(config.mask_format != wgpu::TextureFormat::Undefined) {
				out.masks().gpu_texture = state.device.createTexture(WGPUTextureDescriptor {
					.label = "Waylib Mask Buffer",
					.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc,
					.dimension = wgpu::TextureDimension::_2D,
					.format = config.mask_format,
					.mipLevelCount = 1,
					.sampleCount = 1,
					.viewFormatCount = 1,
					.viewFormats = &config.mask_format
				});
				out.masks().maybe_create_view();
			}
			if(config.barycentric_format != wgpu::TextureFormat::Undefined) {
				out.barycentric().gpu_texture = state.device.createTexture(WGPUTextureDescriptor {
					.label = "Waylib Barycentric Buffer",
					.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc,
					.dimension = wgpu::TextureDimension::_2D,
					.size = {size.x, size.y, 1},
					.format = config.barycentric_format,
					.mipLevelCount = 1,
					.sampleCount = 1,
					.viewFormatCount = 1,
					.viewFormats = &config.barycentric_format
				});
				out.barycentric().maybe_create_view();
			}
			return out;
		// } WAYLIB_CATCH
		} catch(const std::exception& e) { return unexpected(std::string(e.what())); }


		void release() {
			if(color().gpu_texture) color().release();
			if(depth().gpu_texture) depth().release();
			if(normal().gpu_texture) normal().release();
			if(ctl().gpu_texture) ctl().release();
			if(masks().gpu_texture) masks().release();
			if(barycentric().gpu_texture) barycentric().release();
		}

		result<void> resize(wgpu_state& state, vec2u size) {
			wgpu::TextureFormat ctlFormat = wgpu::TextureFormat::Undefined;
			if(ctl().gpu_texture) ctlFormat = ctl().gpu_texture.getFormat();
			wgpu::TextureFormat masksFormat = wgpu::TextureFormat::Undefined;
			if(masks().gpu_texture) masksFormat = masks().gpu_texture.getFormat();
			wgpu::TextureFormat barycentricFormat = wgpu::TextureFormat::Undefined;
			if(barycentric().gpu_texture) barycentricFormat = barycentric().gpu_texture.getFormat();
			auto _new = create_default(state, size, {
				.color_format = color().gpu_texture.getFormat(),
				.depth_format = depth().gpu_texture.getFormat(),
				.normal_format = normal().gpu_texture.getFormat(),
				.ctl_format = ctlFormat,
				.mask_format = masksFormat,
				.barycentric_format = barycentricFormat
			});
			if(!_new) return unexpected(_new.error());

			release();
			static_cast<GbufferC>(*this) = static_cast<GbufferC>(*_new);
			return result<void>::success;
		}

		result<struct drawing_state> begin_drawing(wgpu_state& state, WAYLIB_OPTIONAL(colorC) clear_color = {});
	};


//////////////////////////////////////////////////////////////////////
// # Frame State
//////////////////////////////////////////////////////////////////////


	struct drawing_state : public drawing_stateC {
		WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(drawing_state)
		drawing_state(Gbuffer&& other) { *this = std::move(other); }
		drawing_state& operator=(drawing_state&& other) {
			pre_encoder = std::exchange(other.pre_encoder, nullptr);
			render_encoder = std::exchange(other.render_encoder, nullptr);
			render_pass = std::exchange(other.render_pass, nullptr);
			return *this;
		}
		wgpu_state& state() { return *(wgpu_state*)drawing_stateC::state; }
		Gbuffer& gbuffer() { return *(Gbuffer*)drawing_stateC::gbuffer; }
		operator bool() { return pre_encoder && render_encoder && render_pass; }

		void release() {
			if(pre_encoder) pre_encoder.release();
			if(render_encoder) render_encoder.release();
			if(render_pass) render_pass.release();
		}

		result<std::array<wgpu::CommandBuffer, 2>> record_draw_commands() WAYLIB_TRY {
			render_pass.end();

			wgpu::CommandBufferDescriptor cmdBufferDescriptor = {};
			cmdBufferDescriptor.label = "Waylib Pre Frame Command Buffer";
			wgpu::CommandBuffer commands = pre_encoder.finish(cmdBufferDescriptor);
			cmdBufferDescriptor.label = "Waylib Frame Render Command Buffer";
			wgpu::CommandBuffer render_commands = render_encoder.finish(cmdBufferDescriptor);
			release();

			return std::array<wgpu::CommandBuffer, 2>{commands, render_commands};
		// } WAYLIB_CATCH
		} catch(const std::exception& e) { return unexpected(std::string(e.what())); }

		std::future<result<void>> draw_recordered(const std::array<wgpu::CommandBuffer, 2>& commands) {
			return thread_pool::enqueue([&commands, this]() -> result<void> {
				WAYLIB_TRY {
					state().device.getQueue().submit(commands.size(), commands.data());
					for(auto& command: commands)
						const_cast<wgpu::CommandBuffer*>(&command)->release();
					return result<void>::success;
				// } WAYLIB_CATCH
				} catch(const std::exception& e) { return unexpected(std::string(e.what())); }
			});
		}

		std::future<result<void>> draw() {
			auto commands = record_draw_commands();
			if(!commands) return value2future<result<void>>(unexpected(commands.error()));
			return draw_recordered(*commands);
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

		static result<shader> from_wgsl(wgpu_state& state, std::string_view wgsl_source_code, create_shader_configuration config = {}) WAYLIB_TRY {
			wgpu::ShaderModuleDescriptor shaderDesc;
			shaderDesc.label = config.name;

			// We use the extension mechanism to specify the WGSL part of the shader module descriptor
			wgpu::ShaderSourceWGSL shaderCodeSource;
			shaderCodeSource.chain.next = nullptr;
			shaderCodeSource.chain.sType = wgpu::SType::ShaderSourceWGSL;
			shaderDesc.nextInChain = &shaderCodeSource.chain;
			if(config.preprocessor) {
				auto res = config.preprocessor->process_from_memory(wgsl_source_code, {
					.remove_comments = config.preprocess_config.remove_comments,
					.remove_whitespace = config.preprocess_config.remove_whitespace,
					.support_pragma_once = config.preprocess_config.support_pragma_once,
					.path = config.preprocess_config.path
				});
				if(!res) return unexpected(res.error());

				static std::string keepAlive; keepAlive = *res;
				shaderCodeSource.code = keepAlive.c_str();
			} else shaderCodeSource.code = cstring_from_view(wgsl_source_code);

			return {shaderC{
				.compute_entry_point = config.compute_entry_point,
				.vertex_entry_point = config.vertex_entry_point,
				.fragment_entry_point = config.fragment_entry_point,
				.module = state.device.createShaderModule(shaderDesc)
			}};
		} WAYLIB_CATCH

		void release() {
			if(compute_entry_point.is_managed) delete compute_entry_point.value;
			if(vertex_entry_point.is_managed) delete vertex_entry_point.value;
			if(fragment_entry_point.is_managed) delete fragment_entry_point.value;
			if(module) module.release();
		}

		std::pair<wgpu::RenderPipelineDescriptor, WAYLIB_OPTIONAL(wgpu::FragmentState)> configure_render_pipeline_descriptor(
			wgpu_state& state,
			std::span<WGPUColorTargetState> gbuffer_targets,
			WAYLIB_OPTIONAL(std::span<WGPUVertexBufferLayout>) mesh_layout = {},
			wgpu::RenderPipelineDescriptor pipelineDesc = {}
		);
	};


//////////////////////////////////////////////////////////////////////
// # Material
//////////////////////////////////////////////////////////////////////


	struct material: public materialC {
		WAYLIB_GENERIC_AUTO_RELEASE_SUPPORT(material)
		material(material&& other) { *this = std::move(other); }
		material& operator=(material&& other) {
			shader_count = std::exchange(other.shader_count, 0);
			shaders = std::exchange(other.shaders, nullptr);
			pipeline = std::exchange(other.pipeline, nullptr);
			return *this;
		}
		std::span<shader> shaders_span() { return {static_cast<shader*>(shaders.value), shader_count}; }

		// TODO: This function needs to become gbuffer aware
		result<void> upload(
			wgpu_state& state,
			std::span<WGPUColorTargetState> gbuffer_targets,
			WAYLIB_OPTIONAL(std::span<WGPUVertexBufferLayout>) mesh_layout = {},
			material_configuration config = {}
		) {
			// Create the render pipeline
			wgpu::RenderPipelineDescriptor pipelineDesc;
			wgpu::FragmentState fragment;
			for(size_t i = shader_count; i--; ) { // Reverse iteration ensures that lower indexed shaders take precedence
				shader& shader = *static_cast<struct shader*>(*shaders + i);
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
			depthStencilState.depthWriteEnabled = config.depth_function ? wgpu::OptionalBool::True : wgpu::OptionalBool::False;
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
			return result<void>::success;
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
			uvs2 = std::exchange(other.uvs2, nullptr);
			colors = std::exchange(other.colors, nullptr);
			ctl = std::exchange(other.ctl, nullptr);
			masks = std::exchange(other.masks, nullptr);
			indices = std::exchange(other.indices, nullptr);
			buffer = std::exchange(other.buffer, nullptr);
			index_buffer = std::exchange(other.index_buffer, nullptr);
			return *this;
		}

		static const std::array<WGPUVertexBufferLayout, 8>& mesh_layout() {
			static WGPUVertexAttribute positionAttribute = {
				.format = wgpu::VertexFormat::Float32x3,
				.offset = 0,
				.shaderLocation = 0,
			};
			static WGPUVertexBufferLayout positionLayout = {
				.arrayStride = sizeof(vec3f),
				.stepMode = wgpu::VertexStepMode::Vertex,
				.attributeCount = 1,
				.attributes = &positionAttribute,
			};

			static WGPUVertexAttribute normalsAttribute = {
				.format = wgpu::VertexFormat::Float32x3,
				.offset = 0,
				.shaderLocation = 1,
			};
			static WGPUVertexBufferLayout normalsLayout = {
				.arrayStride = sizeof(vec3f),
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
				.format = wgpu::VertexFormat::Float32x2,
				.offset = 0,
				.shaderLocation = 3,
			};
			static WGPUVertexBufferLayout uvsLayout = {
				.arrayStride = sizeof(vec2f),
				.stepMode = wgpu::VertexStepMode::Vertex,
				.attributeCount = 1,
				.attributes = &uvsAttribute,
			};

			static WGPUVertexAttribute uvs2Attribute = {
				.format = wgpu::VertexFormat::Float32x2,
				.offset = 0,
				.shaderLocation = 4,
			};
			static WGPUVertexBufferLayout uvs2Layout = {
				.arrayStride = sizeof(vec2f),
				.stepMode = wgpu::VertexStepMode::Vertex,
				.attributeCount = 1,
				.attributes = &uvs2Attribute,
			};

			static WGPUVertexAttribute colorsAttribute = {
				.format = wgpu::VertexFormat::Float32x4,
				.offset = 0,
				.shaderLocation = 5,
			};
			static WGPUVertexBufferLayout colorsLayout = {
				.arrayStride = sizeof(colorC),
				.stepMode = wgpu::VertexStepMode::Vertex,
				.attributeCount = 1,
				.attributes = &colorsAttribute,
			};

			static WGPUVertexAttribute ctlAttribute = {
				.format = wgpu::VertexFormat::Float32x4,
				.offset = 0,
				.shaderLocation = 6,
			};
			static WGPUVertexBufferLayout ctlLayout = {
				.arrayStride = sizeof(vec4f),
				.stepMode = wgpu::VertexStepMode::Vertex,
				.attributeCount = 1,
				.attributes = &ctlAttribute,
			};

			static WGPUVertexAttribute masksAttribute = {
				.format = wgpu::VertexFormat::Float32x4,
				.offset = 0,
				.shaderLocation = 7,
			};
			static WGPUVertexBufferLayout masksLayout = {
				.arrayStride = sizeof(vec4f),
				.stepMode = wgpu::VertexStepMode::Vertex,
				.attributeCount = 1,
				.attributes = &masksAttribute,
			};

			static std::array<WGPUVertexBufferLayout, 8> layouts = {positionLayout, normalsLayout, tangentsLayout, uvsLayout, uvs2Layout, colorsLayout, ctlLayout, masksLayout};
			return layouts;
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
			meshes = std::exchange(other.meshes, nullptr);
			material_count = std::exchange(other.material_count, 0);
			materials = std::exchange(other.materials, nullptr);
			mesh_materials = std::exchange(other.mesh_materials, nullptr);
			return *this;
		}
		std::span<mesh> meshes_span() { return {static_cast<mesh*>(meshes.value), mesh_count}; }
		std::span<material> materials_span() { return {static_cast<material*>(materials.value), material_count}; }
		std::span<index_t> mesh_materials_span() { return {static_cast<index_t*>(mesh_materials.value), mesh_count}; }
	};


	extern "C" {
		#include "core.h"
	}

WAYLIB_END_NAMESPACE