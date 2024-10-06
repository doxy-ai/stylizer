#define TCPP_IMPLEMENTATION
#define WEBGPU_CPP_IMPLEMENTATION
#define IS_WAYLIB_CORE_CPP
#include "core.hpp"

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
		.label = "Waylib Device",
		.requiredFeatureCount = 1,
		.requiredFeatures = &float32filterable,
		.requiredLimits = nullptr,
		.defaultQueue = {
			.label = "Waylib Queue"
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
			.callback = [](WGPUDevice const* device, WGPUDeviceLostReason reason, char const* message, void* userdata) {
				std::stringstream s;
				s << "Device " << wgpu::Device{*device} << " lost: reason " << wgpu::DeviceLostReason{reason};
				if (message) s << " (" << message << ")";
#ifdef __cpp_exceptions
				throw exception(s.str());
#else // __cpp_exceptions
				assert(false, s.str().c_str());
#endif // __cpp_exceptions
			}
		},
		.uncapturedErrorCallbackInfo = {
			.callback = [](WGPUErrorType type, char const* message, void* userdata) {
				std::stringstream s;
				s << "Uncaptured device error: type " << wgpu::ErrorType{type};
				if (message) s << " (" << message << ")";
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

	texture out = textureC{.gpu_texture = texture_.texture};
	out.create_view();
	return out;
}

result<struct drawing_state> wgpu_state::begin_drawing_to_surface(WAYLIB_OPTIONAL(colorC) clear_color /* = {} */){
	auto texture = current_surface_texture();
	if(!texture) return unexpected(texture.error());
	return texture->begin_drawing(*this, clear_color);
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
// # Texture
//////////////////////////////////////////////////////////////////////


result<drawing_state> texture::begin_drawing(wgpu_state& state, WAYLIB_OPTIONAL(colorC) clear_color /* = {} */) WAYLIB_TRY {
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
	encoderDesc.label = "Waylib Render Command Encoder";
	out.render_encoder = state.device.createCommandEncoder(encoderDesc);
	encoderDesc.label = "Waylib Command Encoder";
	out.pre_encoder = state.device.createCommandEncoder(encoderDesc);

	static texture depthTexture = {};
	depthTextureDesc.size = {gpu_texture.getWidth(), gpu_texture.getHeight(), 1};
	if(!depthTexture.gpu_texture || depthTexture.size() != size()) {
		if(depthTexture.gpu_texture) depthTexture.gpu_texture.release();
		depthTexture.gpu_texture = state.device.createTexture(depthTextureDesc);
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
	// setup_default_bindings(out);
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

result<drawing_state> texture::blit_to(wgpu_state& state, texture& target, WAYLIB_OPTIONAL(colorC) clear_color /* = {} */) {
	auto draw = target.begin_drawing(state, {clear_color ? *clear_color : colorC(0, 0, 0, 1)});
	if(!draw) return unexpected(draw.error());
	blit(*draw);
	if(auto res = draw->draw(); !res) return unexpected(res.error());
	return draw;
}

result<drawing_state> texture::blit_to(wgpu_state& state, shader& blit_shader, texture& target, WAYLIB_OPTIONAL(colorC) clear_color /* = {} */) {
	auto draw = target.begin_drawing(state, {clear_color ? *clear_color : colorC(0, 0, 0, 1)});
	if(!draw) return unexpected(draw.error());
	blit(*draw, blit_shader);
	if(auto res = draw->draw(); !res) return unexpected(res.error());
	return draw;
}


//////////////////////////////////////////////////////////////////////
// # Gbuffer
//////////////////////////////////////////////////////////////////////


result<drawing_state> Gbuffer::begin_drawing(wgpu_state& state, WAYLIB_OPTIONAL(colorC) clear_color /* = {} */) WAYLIB_TRY {
	drawing_stateC out {.state = &state, .gbuffer = this};
	// static frame_finalizers finalizers;

	// Create a command encoder for the draw call
	wgpu::CommandEncoderDescriptor encoderDesc = {};
	encoderDesc.label = "Waylib Render Command Encoder";
	out.render_encoder = state.device.createCommandEncoder(encoderDesc);
	encoderDesc.label = "Waylib Command Encoder";
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
	// setup_default_bindings(out);
	return {{std::move(out)}};
} WAYLIB_CATCH


//////////////////////////////////////////////////////////////////////
// # shader
//////////////////////////////////////////////////////////////////////


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
		pipelineDesc.vertex.entryPoint = *vertex_entry_point;
		pipelineDesc.vertex.module = module;
	}

	if(fragment_entry_point.value) {
		static wgpu::FragmentState fragment;
		fragment.constantCount = 0;
		fragment.constants = nullptr;
		fragment.entryPoint = *fragment_entry_point;
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