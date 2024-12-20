#pragma once

#include <stylizer/core/core.hpp>

struct GLFWwindow;

namespace stylizer {

	struct window_create_config {
		constexpr static int dont_care = -1;

		bool resizable = true;
		bool visible = true;
		bool decorated = true;
		bool focused = true;
		bool auto_iconify = true;
		bool floating = false;
		bool maximized = false;
		bool center_cursor = true;
		bool transparent_framebuffer = false;
		bool focus_on_show = true;
		bool scale_to_monitor = false;

		int red_bits = 8;
		int green_bits = 8;
		int blue_bits = 8;
		int alpha_bits = 8;
		int depth_bits = 24;
		int stencil_bits = 8;
		int accum_red_bits = 0;
		int accum_green_bits = 0;
		int accum_blue_bits = 0;
		int accum_alpha_bits = 0;

		int aux_buffers = 0;
		int samples = 0;
		int refresh_rate = dont_care;
		bool stereo = false;
		bool srgb_capable = true;
		bool double_buffer = true;
	};

	struct window {
		using create_config = window_create_config;

		GLFWwindow* window_ = nullptr;
		event<window&, uint2> resized;

		window() {}
		window(const window&) = default;
		window(window&& o) { *this = std::move(o); }
		window& operator=(const window&) = default;
		window& operator=(window&& o);

		static struct window create(uint2 size, std::string_view name = "Stylizer", create_config config = {});

		bool should_close(bool process_events = true) const;

		inline bool should_close(context& ctx) const { // Override which automatically processes context events as well
			ctx.process_events();
			return should_close(true);
		}

		uint2 get_dimensions() const;
		inline uint2 get_size() const { return get_dimensions(); }

#ifndef STYLIZER_USE_ABSTRACT_API
		context create_context(const api::device::create_config& config = {});
#endif

		api::surface::config determine_optimal_config(context& ctx, stylizer::api::surface& surface) const {
			return surface.determine_optimal_config(ctx.device, api::convert(get_size()));
		}
		api::surface::config determine_optimal_config(context& ctx) const {
			return determine_optimal_config(ctx, ctx.surface);
		}

		inline window& configure_surface(context& ctx, api::surface::config config, stylizer::api::surface& surface) const {
			config.size = api::convert(get_size());
			surface.configure(ctx, config);
			return *const_cast<window*>(this);
		}
		inline window& configure_surface(context& ctx, api::surface::config config = {}) const {
			return configure_surface(ctx, config, ctx.surface);
		}

		window& reconfigure_surface_on_resize(context& ctx, api::surface::config config, stylizer::api::surface& surface) {
			resized.emplace_back([&surface, &ctx, config](window&, uint2 new_size) mutable {
				config.size = api::convert(new_size);
				surface.configure(ctx, config);
			});
			return configure_surface(ctx, config, surface);
		}
		window& reconfigure_surface_on_resize(context& ctx, api::surface::config config = {}) {
			return reconfigure_surface_on_resize(ctx, config, ctx.surface);
		}

		window& auto_resize_geometry_buffer(context& ctx, geometry_buffer& gbuffer) {
			resized.emplace_back([&gbuffer, &ctx](window&, uint2 new_size){
				gbuffer.resize(ctx, new_size);
			});
			return *this;
		}
	};

	template<>
	struct auto_release<window> : public window {
		using window::window;
		using window::operator=;
		auto_release(window&& o) : window(std::move(o)) {}
	};

} // namespace stylizer