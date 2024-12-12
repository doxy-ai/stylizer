#pragma once

#include "stylizer/core/core.hpp"
struct GLFWwindow;

namespace stylizer {

	extern "C" {
		#include "window.h"
	}

	struct window {
		GLFWwindow* window_ = nullptr;
		event<window&, size_t, size_t> resized;

		window() {}
		window(const window&) = default;
		window(window&& o) { *this = std::move(o); }
		window& operator=(const window&) = default;
		window& operator=(window&& o);

		static struct window create(api::vec2u size, std::string_view name = "Stylizer", window_config config = {});

		bool should_close(bool process_events = true) const;

		inline bool should_close(context& ctx) const { // Override which automatically processes context events as well
			ctx.process_events();
			return should_close(true);
		}

		api::vec2u get_dimensions() const;
		inline api::vec2u get_size() const { return get_dimensions(); }

#ifndef STYLIZER_USE_ABSTRACT_API
		context create_context(const api::device::create_config& config = {});
#endif

		api::surface::config determine_optimal_config(context& ctx, stylizer::api::surface& surface) const {
			return surface.determine_optimal_config(ctx.device, get_size());
		}
		api::surface::config determine_optimal_config(context& ctx) const {
			return determine_optimal_config(ctx, ctx.surface);
		}

		inline window& configure_surface(context& ctx, api::surface::config config, stylizer::api::surface& surface) const {
			config.size = get_size();
			surface.configure(ctx, config);
			return *const_cast<window*>(this);
		}
		inline window& configure_surface(context& ctx, api::surface::config config = {}) const {
			return configure_surface(ctx, config, ctx.surface);
		}

		window& reconfigure_surface_on_resize(context& ctx, api::surface::config config, stylizer::api::surface& surface) {
			resized.emplace_back([&surface, &ctx, config](window&, size_t x, size_t y) mutable {
				config.size = {x, y};
				surface.configure(ctx, config);
			});
			return configure_surface(ctx, config, surface);
		}
		window& reconfigure_surface_on_resize(context& ctx, api::surface::config config = {}) {
			return reconfigure_surface_on_resize(ctx, config, ctx.surface);
		}

		// template<GbufferProvider Tgbuffer>
		// window& auto_resize_gbuffer(wgpu_state& state, Tgbuffer& gbuffer) {
		// 	raw.sizeEvent.append([&state, &gbuffer](glfw::Window&, int x, int y){
		// 		gbuffer.resize(state, {x, y});
		// 	});
		// 	return *this;
		// }

		// window& present(wgpu_state& state, releasable auto... releases) {
		// #ifndef __EMSCRIPTEN__
		// 	surface.present();
		// #endif
		// 	(releases.release(), ...);
		// 	return *this;
		// }

		// window& present(wgpu_state& state, texture& texture, releasable auto... releases) {
		// 	auto oldSurface = state.surface; state.surface = this->surface;
		// 	auto surfaceTexture = state.current_surface_texture();
		// 	drawing_state blit; blit.zero();
		// 	if(surfaceTexture.size() == texture.size())
		// 		surfaceTexture.copy(state, texture);
		// 	else blit = texture.blit_to(state, surfaceTexture);
		// 	state.surface = oldSurface;
		// 	return present(state, releases..., blit, surfaceTexture);
		// }

	};

	template<>
	struct auto_release<window> : public window {
		using window::window;
		using window::operator=;
		auto_release(window&& o) : window(std::move(o)) {}
	};

} // namespace stylizer