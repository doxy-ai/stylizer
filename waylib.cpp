#include "waylib.hpp"

#include <chrono>
#include <limits>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#define OPEN_URL_NAMESPACE wl_detail
#include "thirdparty/open.hpp"

#ifdef WAYLIB_NAMESPACE_NAME
namespace WAYLIB_NAMESPACE_NAME {
#endif

// Defined in common.cpp
pipeline_globals& create_pipeline_globals(wgpu_state state);

//////////////////////////////////////////////////////////////////////
// #Miscelanious
//////////////////////////////////////////////////////////////////////

wgpu::Color to_webgpu(const color& color) {
	return {color.r, color.g, color.b, color.a};
}

void time_calculations(time& time) WAYLIB_TRY {
	constexpr static float alpha = .9;
	static std::chrono::system_clock::time_point last = std::chrono::system_clock::now();

	auto now = std::chrono::system_clock::now();
	time.delta = std::chrono::duration_cast<std::chrono::microseconds>(now - last).count() / 1000000.0;
	time.since_start += time.delta;

	if(std::abs(time.average_delta) < 2 * std::numeric_limits<float>::epsilon()) time.average_delta = time.delta;
	time.average_delta = time.average_delta * alpha + time.delta * (1 - alpha);

	last = now;
} WAYLIB_CATCH()
void time_calculations(time* time) { time_calculations(*time); }

void time_upload(wgpu_frame_state& frame, time& time) WAYLIB_TRY {
	static wgpu::Buffer timeBuffer = [&frame] {
		WGPUBufferDescriptor bufferDesc = {
			.label = "Waylib Time Buffer",
			.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
			.size = sizeof(time),
			.mappedAtCreation = false,
		};
		return frame.state.device.createBuffer(bufferDesc);
	}();
	static wgpu::BindGroup bindGroup = [&frame] {
		std::array<WGPUBindGroupEntry, 1> bindings = {WGPUBindGroupEntry{
			.binding = 0,
			.buffer = timeBuffer,
			.offset = 0,
			.size = sizeof(time)
		}};
		wgpu::BindGroupDescriptor bindGroupDesc = wgpu::Default;
		bindGroupDesc.layout = create_pipeline_globals(frame.state).bindGroupLayouts[1]; // Group 1 is time data
		bindGroupDesc.entryCount = bindings.size();
		bindGroupDesc.entries = bindings.data();
		return frame.state.device.createBindGroup(bindGroupDesc);
	}();

	frame.state.device.getQueue().writeBuffer(timeBuffer, 0, &time, sizeof(time));
	frame.render_pass.setBindGroup(1, bindGroup, 0, nullptr);
} WAYLIB_CATCH()
void time_upload(wgpu_frame_state* frame, time* time) {
	time_upload(*frame, *time);
}

bool open_url(const char* url) WAYLIB_TRY {
	return wl_detail::open_url(url);
} WAYLIB_CATCH(false)

void process_wgpu_events([[maybe_unused]] Device device, [[maybe_unused]] bool yieldToWebBrowser = true) WAYLIB_TRY {
#if defined(WEBGPU_BACKEND_DAWN)
    device.tick();
// #elif defined(WEBGPU_BACKEND_WGPU)
//     device.poll(false);
#elif defined(WEBGPU_BACKEND_EMSCRIPTEN)
    if (yieldToWebBrowser) {
        emscripten_sleep(10);
    }
#endif
} WAYLIB_CATCH()

//////////////////////////////////////////////////////////////////////
// #WebGPU Defaults
//////////////////////////////////////////////////////////////////////

template<typename T>
wgpu::IndexFormat calculate_index_format() {
	static_assert(sizeof(T) == 2 || sizeof(T) == 4, "Index types must either be u16 or u32 convertable.");

	if constexpr(sizeof(T) == 2)
		return wgpu::IndexFormat::Uint16;
	else return wgpu::IndexFormat::Uint32;
}

wgpu::TextureFormat surface_prefered_format(wgpu_state state) {
	wgpu::SurfaceCapabilities capabilities;
	state.surface.getCapabilities(state.device.getAdapter(), &capabilities); // TODO: Always returns error?
	return capabilities.formats[0];
}

wgpu::Device create_default_device_from_instance(WGPUInstance instance, WGPUSurface surface /*= nullptr*/, bool prefer_low_power /*= false*/) WAYLIB_TRY {
	wgpu::RequestAdapterOptions adapterOpts = {};
	adapterOpts.compatibleSurface = surface;
	adapterOpts.powerPreference = prefer_low_power ? wgpu::PowerPreference::LowPower : wgpu::PowerPreference::HighPerformance;
	wgpu::Adapter adapter = WAYLIB_C_TO_CPP_CONVERSION(wgpu::instance, instance).requestAdapter(adapterOpts);
	// std::cout << "Got adapter: " << adapter << std::endl;

	// std::cout << "Requesting device..." << std::endl;
	wgpu::DeviceDescriptor deviceDesc = {};
	deviceDesc.label = "Waylib Device";
	deviceDesc.requiredFeatureCount = 0;
	deviceDesc.requiredLimits = nullptr;
	deviceDesc.defaultQueue.nextInChain = nullptr;
	deviceDesc.defaultQueue.label = "Waylib Queue";
	deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void* /* pUserData */) {
		std::cout << "Device lost: reason " << reason;
		if (message) std::cout << " (" << message << ")";
		std::cout << std::endl;
	};
	return adapter.requestDevice(deviceDesc);
} WAYLIB_CATCH(nullptr)

wgpu::Device create_default_device(WGPUSurface surface /*= nullptr*/, bool prefer_low_power /*= false*/) {
	return create_default_device_from_instance(wgpuCreateInstance({}), surface, prefer_low_power);
}

void release_device(WGPUDevice device, bool also_release_adapter /*= true*/, bool also_release_instance /*= true*/) WAYLIB_TRY {
	wgpu::Adapter adapter = WAYLIB_C_TO_CPP_CONVERSION(wgpu::Device, device).getAdapter();
	wgpu::Instance instance = adapter.getInstance();
	wgpuDeviceRelease(device);
	if(also_release_adapter) wgpuAdapterRelease(adapter);
	if(also_release_instance) wgpuInstanceRelease(instance);
} WAYLIB_CATCH()

void release_wgpu_state(wgpu_state state) WAYLIB_TRY {
	state.device.getQueue().release();
	state.surface.release();
	release_device(state.device, true, true);
} WAYLIB_CATCH()

bool configure_surface(wgpu_state state, vec2i size, surface_configuration config /*= {}*/) WAYLIB_TRY {
	// Configure the surface
	wgpu::SurfaceConfiguration descriptor = {};

	wgpu::SurfaceCapabilities capabilities;
	state.surface.getCapabilities(state.device.getAdapter(), &capabilities); // TODO: Always returns error?

	bool found = false;
	for(size_t i = 0; i < capabilities.presentModeCount; ++i)
		if(capabilities.presentModes[i] == config.presentaion_mode) {
			found = true;
			break;
		}
	if(!found) {
		set_error_message_raw("Desired present mode not available... Falling back to First in First out.");
		config.presentaion_mode = wgpu::PresentMode::Fifo; // Fifo
	}

	// Configuration of the textures created for the underlying swap chain
	descriptor.width = size.x;
	descriptor.height = size.y;
	descriptor.usage = wgpu::TextureUsage::RenderAttachment;
	descriptor.format = capabilities.formats[0];

	// And we do not need any particular view format:
	descriptor.viewFormatCount = 0;
	descriptor.viewFormats = nullptr;
	descriptor.device = state.device;
	descriptor.presentMode = config.presentaion_mode;
	descriptor.alphaMode = config.alpha_mode;

	state.surface.configure(descriptor);
	return true;
} WAYLIB_CATCH(false)

//////////////////////////////////////////////////////////////////////
// #Shader (Preprocessor)
//////////////////////////////////////////////////////////////////////

namespace detail {
	// Defined in shader_preprocessor.cpp
	std::optional<std::string> preprocess_shader_from_memory(shader_preprocessor* processor, const std::string& _data, const preprocess_shader_config& config);
	std::optional<std::string> preprocess_shader_from_memory_and_cache(shader_preprocessor* processor, const std::string& _data, const std::filesystem::path& path, preprocess_shader_config config);
	std::optional<std::string> preprocess_shader(shader_preprocessor* processor, const std::filesystem::path& path, const preprocess_shader_config& config);
	void preprocessor_initalize_platform_defines(shader_preprocessor* preprocessor, wgpu::Adapter adapter);
}

shader_preprocessor* create_shader_preprocessor() WAYLIB_TRY {
	return new shader_preprocessor();
} WAYLIB_CATCH(nullptr)

void release_shader_preprocessor(shader_preprocessor* processor) WAYLIB_TRY {
	delete processor;
} WAYLIB_CATCH()

shader_preprocessor* preprocessor_initialize_virtual_filesystem(shader_preprocessor* processor, wgpu_state state, preprocess_shader_config config /*= {}*/) WAYLIB_TRY {
	if(processor->defines.find("WGPU_ADAPTER_TYPE") == std::string::npos)
		preprocessor_initalize_platform_defines(processor, state);

	preprocess_shader_from_memory_and_cache(processor,
#include "shaders/waylib/time.wgsl"
		, "waylib/time", config);
	preprocess_shader_from_memory_and_cache(processor,
#include "shaders/waylib/instance.wgsl"
		, "waylib/instance", config);
	preprocess_shader_from_memory_and_cache(processor,
#include "shaders/waylib/vertex.wgsl"
		, "waylib/vertex", config);
	preprocess_shader_from_memory_and_cache(processor,
#include "shaders/waylib/wireframe.wgsl"
		, "waylib/wireframe", config);
	return processor;
} WAYLIB_CATCH(nullptr)

shader_preprocessor* preprocessor_add_define(shader_preprocessor* processor, const char* name, const char* value) WAYLIB_TRY {
	processor->defines += "#define " + std::string(name) + " " + value + "\n";
	return processor;
} WAYLIB_CATCH(nullptr)

bool preprocessor_add_search_path(shader_preprocessor* processor, const char* _path) WAYLIB_TRY {
	auto path = std::filesystem::absolute(_path);
	if(!std::filesystem::exists(path)) return false;

	processor->search_paths.emplace(path);
	return true;
} WAYLIB_CATCH(false)

shader_preprocessor* preprocessor_initalize_platform_defines(shader_preprocessor* processor, wgpu_state state) WAYLIB_TRY {
	detail::preprocessor_initalize_platform_defines(processor, state.device.getAdapter());
	return processor;
} WAYLIB_CATCH(nullptr)

const char* preprocessor_get_cached_file(shader_preprocessor* processor, const char* path) WAYLIB_TRY {
	static std::string cache;
	if(processor->file_cache.find(path) != processor->file_cache.end())
		return (cache = processor->file_cache[path]).c_str();
	return nullptr;
} WAYLIB_CATCH(nullptr)

const char* preprocess_shader_from_memory(shader_preprocessor* processor, const char* data, preprocess_shader_config config /*= {}*/) WAYLIB_TRY {
	static std::string cache;
	if(auto res = detail::preprocess_shader_from_memory(processor, data, config); res)
		return (cache = std::move(*res)).c_str();
	return nullptr;
} WAYLIB_CATCH(nullptr)

const char* preprocess_shader_from_memory_and_cache(shader_preprocessor* processor, const char* data, const char* path, preprocess_shader_config config /*= {}*/) WAYLIB_TRY {
	static std::string cache;
	if(auto res = detail::preprocess_shader_from_memory_and_cache(processor, data, path, config); res)
		return (cache = std::move(*res)).c_str();
	return nullptr;
} WAYLIB_CATCH(nullptr)

const char* preprocess_shader(shader_preprocessor* processor, const char* path, preprocess_shader_config config /*= {}*/) WAYLIB_TRY {
	static std::string cache;
	if(auto res = detail::preprocess_shader(processor, path, config); res)
		return (cache = std::move(*res)).c_str();
	return nullptr;
} WAYLIB_CATCH(nullptr)

WAYLIB_OPTIONAL(shader) create_shader(wgpu_state state, const char* wgsl_source_code, create_shader_configuration config /*= {}*/) WAYLIB_TRY {
	wgpu::ShaderModuleDescriptor shaderDesc;
	shaderDesc.label = config.name;
#ifdef WEBGPU_BACKEND_WGPU
	shaderDesc.hintCount = 0;
	shaderDesc.hints = nullptr;
#endif

	// We use the extension mechanism to specify the WGSL part of the shader module descriptor
	wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc;
	// Set the chained struct's header
	shaderCodeDesc.chain.next = nullptr;
	shaderCodeDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
	// Connect the chain
	shaderDesc.nextInChain = &shaderCodeDesc.chain;
	if(config.preprocessor) {
		shaderCodeDesc.code = preprocess_shader_from_memory(config.preprocessor, wgsl_source_code, config.preprocess_config);
		if(!shaderCodeDesc.code) return {};
	} else shaderCodeDesc.code = wgsl_source_code;

	return {{
		.compute_entry_point = config.compute_entry_point,
		.vertex_entry_point = config.vertex_entry_point,
		.fragment_entry_point = config.fragment_entry_point,
		.module = state.device.createShaderModule(shaderDesc)
	}};
} WAYLIB_CATCH({})

namespace detail{
	std::pair<wgpu::RenderPipelineDescriptor, WAYLIB_OPTIONAL(wgpu::FragmentState)> shader_configure_render_pipeline_descriptor(wgpu_state state, shader& shader, wgpu::RenderPipelineDescriptor pipelineDesc = {}) {
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
		thread_local static WGPUColorTargetState colorTarget = {
			.nextInChain = nullptr,
			.format = surface_prefered_format(state),
			.blend = &blendState,
			.writeMask = wgpu::ColorWriteMask::All,
		};

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

		static WGPUVertexAttribute texcoordAttribute = {
			.format = wgpu::VertexFormat::Float32x2,
			.offset = 0,
			.shaderLocation = 1,
		};
		static WGPUVertexBufferLayout texcoordLayout = {
			.arrayStride = sizeof(vec2f),
			.stepMode = wgpu::VertexStepMode::Vertex,
			.attributeCount = 1,
			.attributes = &texcoordAttribute,
		};

		static WGPUVertexAttribute normalAttribute = {
			.format = wgpu::VertexFormat::Float32x3,
			.offset = 0,
			.shaderLocation = 2,
		};
		static WGPUVertexBufferLayout normalLayout = {
			.arrayStride = sizeof(vec3f),
			.stepMode = wgpu::VertexStepMode::Vertex,
			.attributeCount = 1,
			.attributes = &normalAttribute,
		};

		static WGPUVertexAttribute colorAttribute = {
			.format = wgpu::VertexFormat::Float32x4,
			.offset = 0,
			.shaderLocation = 3,
		};
		static WGPUVertexBufferLayout colorLayout = {
			.arrayStride = sizeof(color),
			.stepMode = wgpu::VertexStepMode::Vertex,
			.attributeCount = 1,
			.attributes = &colorAttribute,
		};

		static WGPUVertexAttribute tangentAttribute = {
			.format = wgpu::VertexFormat::Float32x4,
			.offset = 0,
			.shaderLocation = 4,
		};
		static WGPUVertexBufferLayout tangentLayout = {
			.arrayStride = sizeof(vec4f),
			.stepMode = wgpu::VertexStepMode::Vertex,
			.attributeCount = 1,
			.attributes = &tangentAttribute,
		};

		static WGPUVertexAttribute texcoord2Attribute = {
			.format = wgpu::VertexFormat::Float32x2,
			.offset = 0,
			.shaderLocation = 5,
		};
		static WGPUVertexBufferLayout texcoord2Layout = {
			.arrayStride = sizeof(vec2f),
			.stepMode = wgpu::VertexStepMode::Vertex,
			.attributeCount = 1,
			.attributes = &texcoord2Attribute,
		};
		static std::array<WGPUVertexBufferLayout, 6> buffers = {positionLayout, texcoordLayout, normalLayout, colorLayout, tangentLayout, texcoord2Layout};

		if(shader.vertex_entry_point) {
			pipelineDesc.vertex.bufferCount = buffers.size();
			pipelineDesc.vertex.buffers = buffers.data();
			pipelineDesc.vertex.constantCount = 0;
			pipelineDesc.vertex.constants = nullptr;
			pipelineDesc.vertex.entryPoint = shader.vertex_entry_point;
			pipelineDesc.vertex.module = shader.module;
		}

		if(shader.fragment_entry_point) {
			wgpu::FragmentState fragment;
			fragment.constantCount = 0;
			fragment.constants = nullptr;
			fragment.entryPoint = shader.fragment_entry_point;
			fragment.module = shader.module;
			fragment.targetCount = 1;
			fragment.targets = &colorTarget;
			pipelineDesc.fragment = &fragment;
			return {pipelineDesc, fragment};
		}
		return {pipelineDesc, {}};
	}
}

//////////////////////////////////////////////////////////////////////
// #Begin/End Drawing
//////////////////////////////////////////////////////////////////////

WAYLIB_OPTIONAL(wgpu_frame_state) begin_drawing_render_texture(wgpu_state state, WGPUTextureView render_texture, vec2i render_texture_dimensions, WAYLIB_OPTIONAL(color) clear_color /*= {}*/) WAYLIB_TRY {
	static WGPUTextureViewDescriptor depthTextureViewDesc = {
		.format = depth_texture_format,
		.dimension = wgpu::TextureViewDimension::_2D,
		.baseMipLevel = 0,
		.mipLevelCount = 1,
		.baseArrayLayer = 0,
		.arrayLayerCount = 1,
		.aspect = wgpu::TextureAspect::DepthOnly,
	};
	static WGPUTextureDescriptor depthTextureDesc = {
		.usage = TextureUsage::RenderAttachment,
		.dimension = wgpu::TextureDimension::_2D,
		.format = depth_texture_format,
		.mipLevelCount = 1,
		.sampleCount = 1,
		.viewFormatCount = 1,
		.viewFormats = (WGPUTextureFormat*)&depth_texture_format,
	};

	static wgpu::Texture depthTexture;
	static wgpu::TextureView depthTextureView;

	// Create a command encoder for the draw call
	wgpu::CommandEncoderDescriptor encoderDesc = {};
	encoderDesc.label = "Waylib Command Encoder";
	wgpu::CommandEncoder encoder = state.device.createCommandEncoder(encoderDesc);

	// Update the depth texture (if nessicary)
	if(!depthTexture || depthTextureDesc.size.width != render_texture_dimensions.x || depthTextureDesc.size.height != render_texture_dimensions.y) {
		depthTextureDesc.size = {(uint32_t)render_texture_dimensions.x, (uint32_t)render_texture_dimensions.y, 1};
		if(depthTextureView) depthTextureView.release();
		if(depthTexture) {
			depthTexture.destroy();
			depthTexture.release();
		}
		depthTexture = state.device.createTexture(depthTextureDesc);
		depthTextureView = depthTexture.createView(depthTextureViewDesc);
	}

	// Create the render pass that clears the screen with our color
	wgpu::RenderPassDescriptor renderPassDesc = {};

	// The attachment part of the render pass descriptor describes the target texture of the pass
	wgpu::RenderPassColorAttachment renderPassColorAttachment = {};
	renderPassColorAttachment.view = render_texture;
	renderPassColorAttachment.resolveTarget = nullptr;
	renderPassColorAttachment.loadOp = clear_color.has_value ? wgpu::LoadOp::Clear : wgpu::LoadOp::Load;
	renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
	renderPassColorAttachment.clearValue = to_webgpu(clear_color.has_value ? clear_color.value : color{0, 0, 0, 0});
	renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;

	// We now add a depth/stencil attachment:
	wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;
	depthStencilAttachment.view = depthTextureView;
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
	renderPassDesc.colorAttachments = &renderPassColorAttachment;
	renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
	renderPassDesc.timestampWrites = nullptr;

	// Create the render pass and end it immediately (we only clear the screen but do not draw anything)
	wgpu::RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

	wgpu_frame_state out = {state, render_texture, depthTexture, depthTextureView, encoder, renderPass};
	out.get_current_view_projection_matrix() = glm::identity<glm::mat4x4>();
	return out;
} WAYLIB_CATCH({})

WAYLIB_OPTIONAL(wgpu_frame_state) begin_drawing(wgpu_state state, WAYLIB_OPTIONAL(color) clear_color /*= {}*/) WAYLIB_TRY {
	// Get the surface texture
	wgpu::SurfaceTexture surfaceTexture;
	state.surface.getCurrentTexture(&surfaceTexture);
	if (surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success)
		return {};
	wgpu::Texture texture = surfaceTexture.texture;

	// Create a view for this surface texture
	wgpu::TextureViewDescriptor viewDescriptor;
	viewDescriptor.label = "Waylib Surface Texture View";
	viewDescriptor.format = texture.getFormat();
	viewDescriptor.dimension = wgpu::TextureViewDimension::_2D;
	viewDescriptor.baseMipLevel = 0;
	viewDescriptor.mipLevelCount = 1;
	viewDescriptor.baseArrayLayer = 0;
	viewDescriptor.arrayLayerCount = 1;
	viewDescriptor.aspect = wgpu::TextureAspect::All;

	return begin_drawing_render_texture(state, texture.createView(viewDescriptor), {texture.getWidth(), texture.getHeight()}, clear_color);
} WAYLIB_CATCH({})

void end_drawing(wgpu_frame_state& frame) WAYLIB_TRY {
	frame.render_pass.end();
	frame.render_pass.release();

	// Finally encode and submit the render pass
	wgpu::CommandBufferDescriptor cmdBufferDescriptor = {};
	cmdBufferDescriptor.label = "Waylib Frame Command buffer";
	wgpu::CommandBuffer command = frame.encoder.finish(cmdBufferDescriptor);
	frame.encoder.release();

	// std::cout << "Submitting command..." << std::endl;
	frame.state.device.getQueue().submit(1, &command);
	command.release();
	// std::cout << "Command submitted." << std::endl;

#ifndef __EMSCRIPTEN__
	frame.state.surface.present();
#endif

	// Process callbacks
	frame.state.device.tick();
} WAYLIB_CATCH()

#ifndef WAYLIB_NO_CAMERAS
mat4x4f camera3D_get_matrix(camera3D& camera, vec2i window_dimensions) {
	mat4x4f V = glm::lookAt(camera.position, camera.target, camera.up);
	mat4x4f P;
	if(camera.orthographic) {
		float right = camera.field_of_view / 2;
		float top = right * window_dimensions.y / window_dimensions.x;
		P = glm::ortho<float>(-right, right, -top, top, camera.near_clip_distance, camera.far_clip_distance);
	} else
		P = glm::perspectiveFov<float>(camera.field_of_view, window_dimensions.x, window_dimensions.y, camera.near_clip_distance, camera.far_clip_distance);
	return P * V;
}
mat4x4f camera3D_get_matrix(camera3D* camera, vec2i window_dimensions) { return camera3D_get_matrix(*camera, window_dimensions); }

mat4x4f camera2D_get_matrix(camera2D& camera, vec2i window_dimensions) {
	window_dimensions /= std::abs(camera.zoom) < .01 ? 2 : 2 * camera.zoom;
	vec3f position = {camera.target.x, camera.target.y, -.1};
	vec4f up = {0, 1, 0, 0};
	up = glm::rotate(glm::identity<glm::mat4x4>(), camera.rotation.radian_value(), vec3f{0, 0, 1}) * up;
	auto P = glm::ortho<float>(-window_dimensions.x, window_dimensions.x, -window_dimensions.y, window_dimensions.y, camera.near_clip_distance, camera.far_clip_distance);
	auto V = glm::lookAt(position, position + vec3f{0, 0, 1}, up.xyz());
	return P * V;
}
mat4x4f camera2D_get_matrix(camera2D* camera, vec2i window_dimensions) { return camera2D_get_matrix(*camera, window_dimensions); }

void begin_camera_mode3D(wgpu_frame_state& frame, camera3D& camera, vec2i window_dimensions) {
	frame.get_current_view_projection_matrix() = camera3D_get_matrix(camera, window_dimensions);
}
void begin_camera_mode3D(wgpu_frame_state* frame, camera3D* camera, vec2i window_dimensions) {
	begin_camera_mode3D(*frame, *camera, window_dimensions);
}

void begin_camera_mode2D(wgpu_frame_state& frame, camera2D& camera, vec2i window_dimensions) {
	frame.get_current_view_projection_matrix() = camera2D_get_matrix(camera, window_dimensions);
}
void begin_camera_mode2D(wgpu_frame_state* frame, camera2D* camera, vec2i window_dimensions) {
	begin_camera_mode2D(*frame, *camera, window_dimensions);
}

void end_camera_mode(wgpu_frame_state& frame) {
	frame.get_current_view_projection_matrix() = glm::identity<glm::mat4x4>();
}
void end_camera_mode(wgpu_frame_state* frame) {
	end_camera_mode(*frame);
}
#endif // WAYLIB_NO_CAMERAS

//////////////////////////////////////////////////////////////////////
// #Mesh
//////////////////////////////////////////////////////////////////////

void mesh_upload(wgpu_state state, mesh& mesh) WAYLIB_TRY {
	size_t biggest = std::max(mesh.vertexCount * sizeof(vec4f), mesh.triangleCount * sizeof(index_t) * 3);
	std::vector<std::byte> zeroBuffer(biggest, std::byte{});

	wgpu::BufferDescriptor bufferDesc;
	bufferDesc.label = "Waylib Vertex Buffer";
	bufferDesc.size = mesh.vertexCount * sizeof(vec2f) * 2
		+ mesh.vertexCount * sizeof(vec3f) * 2
		+ mesh.vertexCount * sizeof(vec4f) * 1
		+ mesh.vertexCount * sizeof(color) * 1;
	bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex; // Vertex usage here!
	bufferDesc.mappedAtCreation = false;
	if(mesh.buffer) mesh.buffer.release();
	mesh.buffer = state.device.createBuffer(bufferDesc);

	// Upload geometry data to the buffer
	size_t currentOffset = 0;
	wgpu::Queue queue = state.device.getQueue();
	{ // Position
		void* data = mesh.positions ? (void*)mesh.positions : (void*)zeroBuffer.data();
		queue.writeBuffer(mesh.buffer, currentOffset, data, mesh.vertexCount * sizeof(vec3f));
		currentOffset += mesh.vertexCount * sizeof(vec3f);
	}
	{ // Texcoords
		void* data = mesh.texcoords ? (void*)mesh.texcoords : (void*)zeroBuffer.data();
		queue.writeBuffer(mesh.buffer, currentOffset, data, mesh.vertexCount * sizeof(vec2f));
		currentOffset += mesh.vertexCount * sizeof(vec2f);
	}
	{ // Texcoords2
		void* data = mesh.texcoords2 ? (void*)mesh.texcoords2 : (void*)zeroBuffer.data();
		queue.writeBuffer(mesh.buffer, currentOffset, data, mesh.vertexCount * sizeof(vec2f));
		currentOffset += mesh.vertexCount * sizeof(vec2f);
	}
	{ // Normals
		void* data = mesh.normals ? (void*)mesh.normals : (void*)zeroBuffer.data();
		queue.writeBuffer(mesh.buffer, currentOffset, data, mesh.vertexCount * sizeof(vec3f));
		currentOffset += mesh.vertexCount * sizeof(vec3f);
	}
	{ // Tangents
		void* data = mesh.tangents ? (void*)mesh.tangents : (void*)zeroBuffer.data();
		queue.writeBuffer(mesh.buffer, currentOffset, data, mesh.vertexCount * sizeof(vec4f));
		currentOffset += mesh.vertexCount * sizeof(vec4f);
	}
	{ // Colors
		void* data = mesh.colors ? (void*)mesh.colors : (void*)zeroBuffer.data();
		queue.writeBuffer(mesh.buffer, currentOffset, data, mesh.vertexCount * sizeof(color));
		currentOffset += mesh.vertexCount * sizeof(color);
	}
	if(mesh.indices) {
		wgpu::BufferDescriptor bufferDesc;
		bufferDesc.label = "Vertex Position Buffer";
		bufferDesc.size = mesh.triangleCount * sizeof(index_t) * 3;
		bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index;
		bufferDesc.mappedAtCreation = false;
		if(mesh.indexBuffer) mesh.indexBuffer.release();
		mesh.indexBuffer = state.device.createBuffer(bufferDesc);

		queue.writeBuffer(mesh.indexBuffer, 0, mesh.indices, mesh.triangleCount * sizeof(index_t) * 3);
	} else if(mesh.indexBuffer) mesh.indexBuffer.release();
} WAYLIB_CATCH()
void mesh_upload(wgpu_state state, mesh* mesh) {
	mesh_upload(state, *mesh);
}

//////////////////////////////////////////////////////////////////////
// #Material
//////////////////////////////////////////////////////////////////////

pipeline_globals& create_pipeline_globals(wgpu_state state); // Declared in waylib.cpp

void material_upload(wgpu_state state, material& material, material_configuration config /*= {}*/) WAYLIB_TRY {
	// Create the render pipeline
	wgpu::RenderPipelineDescriptor pipelineDesc;
	wgpu::FragmentState fragment;
	for(size_t i = material.shaderCount; i--; ) { // Reverse itteration ensures that lower indexed shaders take precedence
		WAYLIB_OPTIONAL(wgpu::FragmentState) fragmentOpt;
		std::tie(pipelineDesc, fragmentOpt) = detail::shader_configure_render_pipeline_descriptor(state, material.shaders[i], pipelineDesc);
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
	pipelineDesc.primitive.cullMode = wgpu::CullMode::None; // = wgpu::CullMode::Back;

	// We setup a depth buffer state for the render pipeline
	wgpu::DepthStencilState depthStencilState = wgpu::Default;
	// Keep a fragment only if its depth is lower than the previously blended one
	depthStencilState.depthCompare = config.depth_function.has_value ? config.depth_function.value : wgpu::CompareFunction::Undefined;
	// Each time a fragment is blended into the target, we update the value of the Z-buffer
	depthStencilState.depthWriteEnabled = config.depth_function.has_value;
	// Store the format in a variable as later parts of the code depend on it
	depthStencilState.format = depth_texture_format;
	// Deactivate the stencil alltogether
	depthStencilState.stencilReadMask = 0;
	depthStencilState.stencilWriteMask = 0;
	pipelineDesc.depthStencil = &depthStencilState;

	// Samples per pixel
	pipelineDesc.multisample.count = 1;
	// Default value for the mask, meaning "all bits on"
	pipelineDesc.multisample.mask = ~0u;
	// Default value as well (irrelevant for count = 1 anyways)
	pipelineDesc.multisample.alphaToCoverageEnabled = false;
	// Associate with the global layout
	pipelineDesc.layout = create_pipeline_globals(state).layout;

	if(material.pipeline) material.pipeline.release();
	material.pipeline = state.device.createRenderPipeline(pipelineDesc);
} WAYLIB_CATCH()
void material_upload(wgpu_state state, material* material, material_configuration config /*= {}*/) {
	material_upload(state, *material, config);
}

material create_material(wgpu_state state, shader* shaders, size_t shader_count, material_configuration config /*= {}*/) {
	material out {.shaderCount = (index_t)shader_count, .shaders = shaders};
	material_upload(state, out, config);
	return out;
}
material create_material(wgpu_state state, std::span<shader> shaders, material_configuration config /*= {}*/) {
	return create_material(state, shaders.data(), shaders.size(), config);
}
material create_material(wgpu_state state, shader& shader, material_configuration config /*= {}*/) {
	return create_material(state, &shader, 1, config);
}

//////////////////////////////////////////////////////////////////////
// #Model
//////////////////////////////////////////////////////////////////////

void model_upload(wgpu_state state, model& model) {
	for(size_t i = 0; i < model.mesh_count; ++i)
		mesh_upload(state, model.meshes[i]);
	for(size_t i = 0; i < model.material_count; ++i)
		material_upload(state, model.materials[i]);
}
void model_upload(wgpu_state state, model* model) {
	model_upload(state, *model);
}

void model_draw_instanced(wgpu_frame_state& frame, model& model, std::span<model_instance_data> instances) WAYLIB_TRY {
	static WGPUBufferDescriptor bufferDesc = {
		.label = "Waylib Instance Buffer",
		.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Storage,
		.size = sizeof(model_instance_data),
		.mappedAtCreation = false,
	};
	static wgpu::Buffer instanceBuffer;
	static wgpu::BindGroup bindGroup;

	if(instances.size() > 0) {
#ifndef WAYLIB_NO_CAMERAS
		for(auto& data: instances)
			data.get_transform() = frame.get_current_view_projection_matrix() * data.get_transform();
#endif // WAYLIB_NO_CAMERAS

		if(!instanceBuffer || instanceBuffer.getSize() < sizeof(model_instance_data) * instances.size()) {
			if(instanceBuffer) instanceBuffer.release();
			bufferDesc.size = sizeof(model_instance_data) * instances.size();
			instanceBuffer = frame.state.device.createBuffer(bufferDesc);
		}
		frame.state.device.getQueue().writeBuffer(instanceBuffer, 0, instances.data(), sizeof(model_instance_data) * instances.size());

		std::array<WGPUBindGroupEntry, 1> bindings = {WGPUBindGroupEntry{
			.binding = 0,
			.buffer = instanceBuffer,
			.offset = 0,
			.size = sizeof(model_instance_data) * instances.size(),
		}};
		wgpu::BindGroupDescriptor bindGroupDesc = wgpu::Default;
		bindGroupDesc.layout = create_pipeline_globals(frame.state).bindGroupLayouts[0]; // Group 0 is instance data
		bindGroupDesc.entryCount = bindings.size();
		bindGroupDesc.entries = bindings.data();
		
		if(bindGroup) bindGroup.release();
		bindGroup = frame.state.device.createBindGroup(bindGroupDesc);
		frame.render_pass.setBindGroup(0, bindGroup, 0, nullptr);
	}

	for(size_t i = 0; i < model.mesh_count; ++i) {
		// Select which render pipeline to use
		frame.render_pass.setPipeline(model.materials[model.get_material_index_for_mesh(i)].pipeline);

		size_t currentOffset = 0;
		auto& mesh = model.meshes[i];
		{ // Position
			frame.render_pass.setVertexBuffer(0, mesh.buffer, currentOffset, mesh.vertexCount * sizeof(vec3f));
			currentOffset += mesh.vertexCount * sizeof(vec3f);
		}
		{ // Texcoord
			frame.render_pass.setVertexBuffer(1, mesh.buffer, currentOffset, mesh.vertexCount * sizeof(vec2f));
			currentOffset += mesh.vertexCount * sizeof(vec2f);
		}
		{ // Texcoord 2
			frame.render_pass.setVertexBuffer(5, mesh.buffer, currentOffset, mesh.vertexCount * sizeof(vec2f));
			currentOffset += mesh.vertexCount * sizeof(vec2f);
		}
		{ // Normals
			frame.render_pass.setVertexBuffer(2, mesh.buffer, currentOffset, mesh.vertexCount * sizeof(vec3f));
			currentOffset += mesh.vertexCount * sizeof(vec3f);
		}
		{ // Tangents
			frame.render_pass.setVertexBuffer(4, mesh.buffer, currentOffset, mesh.vertexCount * sizeof(vec4f));
			currentOffset += mesh.vertexCount * sizeof(vec4f);
		}
		{ // Colors
			frame.render_pass.setVertexBuffer(3, mesh.buffer, currentOffset, mesh.vertexCount * sizeof(color));
			currentOffset += mesh.vertexCount * sizeof(color);
		}

		if(mesh.indexBuffer) {
			frame.render_pass.setIndexBuffer(mesh.indexBuffer, calculate_index_format<index_t>(), 0, mesh.indexBuffer.getSize());
			frame.render_pass.drawIndexed(mesh.triangleCount * 3, std::max<size_t>(instances.size(), 1), 0, 0, 0);
		} else
			frame.render_pass.draw(model.meshes[i].vertexCount, std::max<size_t>(instances.size(), 1), 0, 0);
	}
} WAYLIB_CATCH()
void model_draw_instanced(wgpu_frame_state* frame, model* model, model_instance_data* instances, size_t instance_count) {
	model_draw_instanced(*frame, *model, {instances, instance_count});
}

void model_draw(wgpu_frame_state& frame, model& model) {
	model_instance_data instance = {model.transform, {1, 1, 1, 1}};
	model_draw_instanced(frame, model, {&instance, 1});
}
void model_draw(wgpu_frame_state* frame, model* model) {
	model_draw(*frame, *model);
}

#ifdef WAYLIB_NAMESPACE_NAME
}
#endif