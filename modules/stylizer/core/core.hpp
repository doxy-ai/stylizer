#pragma once

#include "config.hpp"
#include "slang_types.hpp"
#include "optional.h"

#include "stylizer/api/api.hpp"
#include "thirdparty/thread_pool.hpp"

namespace stylizer {

//////////////////////////////////////////////////////////////////////
// # Thread Pool
//////////////////////////////////////////////////////////////////////


	struct STYLIZER_PREFIXED(thread_pool_future) : std::future<void> {};

	struct thread_pool {
	protected:
		static ZenSepiol::ThreadPool& get_thread_pool(optional<size_t> initial_pool_size = {})
#ifdef IS_STYLIZER_CORE_CPP
		{
			static ZenSepiol::ThreadPool pool(initial_pool_size ? *initial_pool_size : std::thread::hardware_concurrency() - 1);
			return pool;
		}
#else
		;
#endif

	public:
		template <typename F, typename... Args>
		static auto enqueue(F&& function, optional<size_t> initial_pool_size = {}, Args&&... args) {
			return get_thread_pool(initial_pool_size).AddTask(function, args...);
		}
	};


//////////////////////////////////////////////////////////////////////
// # Drawing State
//////////////////////////////////////////////////////////////////////


	struct drawing_state: public STYLIZER_API_NAMESPACE::render_pass {
		using super = STYLIZER_API_NAMESPACE::render_pass;

		struct context* context;
		STYLIZER_NULLABLE(struct geometry_buffer*) gbuffer = nullptr;

		STYLIZER_API_TYPE(command_buffer) end();
		void one_shot_submit();
	};


//////////////////////////////////////////////////////////////////////
// # Texture
//////////////////////////////////////////////////////////////////////


	struct texture: public STYLIZER_API_NAMESPACE::texture {
		using super = STYLIZER_API_NAMESPACE::texture;

		drawing_state begin_drawing(context& ctx, float4 clear_color, bool one_shot = true);
		drawing_state begin_drawing(context& ctx, optional<float3> clear_color = {}, bool one_shot = true) {
			return begin_drawing(ctx, clear_color ? float4(*clear_color, 1) : float4{0, 0, 0, 1}, one_shot);
		}
	};


//////////////////////////////////////////////////////////////////////
// # Context
//////////////////////////////////////////////////////////////////////


	struct context {
		STYLIZER_API_TYPE(device) device;
		STYLIZER_API_TYPE(surface) surface;
		operator bool() { return device || surface; }
		operator stylizer::api::device&() { return device; } // Automatically convert to an API device!

#ifndef STYLIZER_USE_ABSTRACT_API
		static context create_default(stylizer::api::device::create_config config = {}, optional<stylizer::api::current_backend::surface&> surface = {}) {
			config.compatible_surface = surface ? &surface.value : nullptr;
			context out = {};
			out.device = stylizer::api::current_backend::device::create_default(config);
			if(surface) out.surface = std::move(*surface);
			return out;
		}
#endif

		void process_events() { device.process_events(); }
		void present() { surface.present(device); }

		texture get_surface_texture() {
			auto tmp = surface.next_texture(device);
			return std::move((texture&)tmp);
		}
		drawing_state begin_drawing_to_surface(float4 clear_color, bool one_shot = true) {
			return get_surface_texture().begin_drawing(*this, clear_color, one_shot);
		}
		drawing_state begin_drawing_to_surface(optional<float3> clear_color = {}, bool one_shot = true) {
			return begin_drawing_to_surface(clear_color ? float4(*clear_color, 1) : float4{0, 0, 0, 1}, one_shot);
		}

		void release(bool static_sub_objects = false) {
			device.release(static_sub_objects);
			surface.release();
		}
	};

	inline STYLIZER_API_TYPE(command_buffer) drawing_state::end() {
		assert(context);
		return super::end(context->device);
	}
	inline void drawing_state::one_shot_submit() {
		assert(context);
		super::one_shot_submit(context->device);
	}

	inline drawing_state texture::begin_drawing(context& ctx, float4 clear_color, bool one_shot /* = true */) {
		drawing_state out;
		(STYLIZER_API_NAMESPACE::render_pass&) out = ctx.device.create_render_pass(std::array<api::render_pass::color_attachment, 1>{api::render_pass::color_attachment{
			.texture = this, .clear_value = api::convert(clear_color)
		}}, {}, one_shot);
		out.context = &ctx;
		return out;
	}


//////////////////////////////////////////////////////////////////////
// # Geometry Buffer
//////////////////////////////////////////////////////////////////////


	struct geometry_buffer_create_config {
		STYLIZER_NULLABLE(struct geometry_buffer*) previous = nullptr;
		texture::format color_format = texture::format::BGRA8_SRGB;
		texture::format depth_format = texture::format::Depth24;
	};

	struct geometry_buffer {
		using create_config = geometry_buffer_create_config;

		create_config config;
		STYLIZER_API_TYPE(texture) color, depth;
		operator bool() { return color || depth; }

		static geometry_buffer create_default(context& ctx, uint2 size, create_config config = {}) {
			using namespace api::operators;

			geometry_buffer out;
			out.config = config;
			out.color = ctx.device.create_texture({
				.label = "Stylizer Gbuffer Color Texture",
				.format = config.color_format,
				.usage = api::usage::RenderAttachment | api::usage::TextureBinding,
				.size = api::convert(uint3(size, 1))
			});
			out.color.configure_sampler(ctx);
			out.depth = ctx.device.create_texture({
				.label = "Stylizer Gbuffer Depth Texture",
				.format = config.depth_format,
				.usage = api::usage::RenderAttachment | api::usage::TextureBinding,
				.size = api::convert(uint3(size, 1))
			});
			return out;
		}

		virtual geometry_buffer& resize(context& ctx, uint2 size) {
			auto new_ = create_default(ctx, size, config);			
			release();
			return *this = std::move(new_);
		}

		virtual std::span<api::render_pass::color_attachment> color_attachments() {
			static std::array<api::render_pass::color_attachment, 1> out;
			return out = {api::render_pass::color_attachment{ .texture = &color }};
		}
		virtual std::optional<api::render_pass::depth_stencil_attachment> depth_attachment() {
			return {api::render_pass::depth_stencil_attachment{
				.texture = &depth,
			}};
		}

		drawing_state begin_drawing(context& ctx, float4 clear_color, optional<float> clear_depth = {}, bool one_shot = true) {
			auto color_attachments = this->color_attachments();
			color_attachments[0].clear_value = api::convert(clear_color);
			auto depth_attachment = this->depth_attachment();
			depth_attachment->depth_clear_value = clear_depth ? *clear_depth : 1;

			drawing_state out;			
			(STYLIZER_API_NAMESPACE::render_pass&)out = ctx.device.create_render_pass(color_attachments, depth_attachment, one_shot);
			out.context = &ctx;
			out.gbuffer = this;
			return out;
		}
		drawing_state begin_drawing(context& ctx, optional<float3> clear_color = {}, optional<float> clear_depth = {}, bool one_shot = true) {
			return begin_drawing(ctx, clear_color ? float4(*clear_color, 1) : float4{0, 0, 0, 1}, clear_depth, one_shot);
		}

		void release() {
			color.release();
			depth.release();
		}
	};
	using gbuffer = geometry_buffer;


} // namespace stylizer