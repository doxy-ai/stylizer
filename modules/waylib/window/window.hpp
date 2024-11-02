#pragma once

#include "glfwpp/event.h"
#include "waylib/core/core.hpp"

#include <glfwpp/glfwpp.h>
#include <glfw3webgpu.h>

WAYLIB_BEGIN_NAMESPACE

	extern "C" {
		#include "window.h"
	}

	struct window {
		glfw::Window raw;
		wgpu::Surface surface = nullptr;

		static struct window create(vec2u size, std::string_view name = "Waylib", window_config config = {});

		inline bool should_close(bool poll_events = true) const {
			if(poll_events) glfw::pollEvents();
			return raw.shouldClose();
		}

		inline vec2u get_dimensions() const {
			vec2u out; std::tie(out.x, out.y) = raw.getSize();
			return out;
		}
		inline vec2u get_size() const { return get_dimensions(); }

		wgpu::Surface get_surface(WGPUInstance instance) {
			if(!surface) surface = glfwCreateWindowWGPUSurface(instance, raw);
			return surface;
		}

		wgpu_state create_default_state(bool prefer_low_power = false) {
			auto partial = wgpu_state::create_default(nullptr, prefer_low_power);
			partial.surface = get_surface(partial.instance);
			return partial;
		}

		inline window& configure_surface(wgpu_state& state, surface_configuration config = {}) const {
			state.configure_surface(get_dimensions(), config);
			return *const_cast<window*>(this);
		}

		window& reconfigure_surface_on_resize(wgpu_state& state, surface_configuration config = {}) {
			raw.sizeEvent.append([&state, config](glfw::Window&, int x, int y){
				state.configure_surface({x, y}, config);
			});
			if(config.automatic_should_configure_now) return configure_surface(state, config);
			return *this;
		}

		template<GbufferProvider Tgbuffer>
		window& auto_resize_gbuffer(wgpu_state& state, Tgbuffer& gbuffer) {
			raw.sizeEvent.append([&state, &gbuffer](glfw::Window&, int x, int y){
				gbuffer.resize(state, {x, y});
			});
			return *this;
		}

		window& present(wgpu_state& state, releasable auto... releases) {
		#ifndef __EMSCRIPTEN__
			surface.present();
		#endif
			(releases.release(), ...);
			return *this;
		}

		window& present(wgpu_state& state, texture& texture, releasable auto... releases) {
			auto oldSurface = state.surface; state.surface = this->surface;
			auto surfaceTexture = state.current_surface_texture();
			drawing_state blit; blit.zero();
			if(surfaceTexture.size() == texture.size())
				surfaceTexture.copy(state, texture);
			else blit = texture.blit_to(state, surfaceTexture);
			state.surface = oldSurface;
			return present(state, releases..., blit, surfaceTexture);
		}

	};

	template<>
	struct auto_release<window> : public window {
		using window::window;
		using window::operator=;
		auto_release(window&& o) : window(std::move(o)) {}
	};

WAYLIB_END_NAMESPACE