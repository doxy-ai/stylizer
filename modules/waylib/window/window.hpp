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

		result<wgpu_state> create_default_state(bool prefer_low_power = false) WAYLIB_TRY {
			auto partial = wgpu_state::create_default(nullptr, prefer_low_power);
			if(!partial) return partial;
			partial->surface = get_surface(partial->instance);
			return partial;
		} WAYLIB_CATCH

		inline result<window*> configure_surface(wgpu_state& state, surface_configuration config = {}) const {
			if(auto res = state.configure_surface(get_dimensions(), config); !res) return unexpected(res.error());
			return const_cast<window*>(this);
		}

		result<window*> reconfigure_surface_on_resize(wgpu_state& state, surface_configuration config = {}) {
			raw.sizeEvent.append([&state, config](glfw::Window&, int x, int y){
				state.configure_surface({x, y}, config);
			});
			if(config.automatic_should_configure_now) return configure_surface(state, config);
			return this;
		}

		template<GbufferProvider Tgbuffer>
		window& auto_resize_gbuffer(wgpu_state& state, Tgbuffer& gbuffer) {
			raw.sizeEvent.append([&state, &gbuffer](glfw::Window&, int x, int y){
				gbuffer.resize(state, {x, y});
			});
			return *this;
		}

		result<window*> present(wgpu_state& state, releasable auto... releases) WAYLIB_TRY {
		#ifndef __EMSCRIPTEN__
			surface.present();
		#endif
			(releases.release(), ...);
			return this;
		} WAYLIB_CATCH

		result<window*> present(wgpu_state& state, texture& texture, releasable auto... releases) {
			auto oldSurface = state.surface; state.surface = this->surface;
			auto surfaceTexture = state.current_surface_texture(); if(!surfaceTexture) return unexpected(surfaceTexture.error());
			result<drawing_state> blit = drawing_state{};
			if(surfaceTexture->size() == texture.size()) {
				if(auto res = surfaceTexture->copy(state, texture); !res) return unexpected(res.error());
			} else if(blit = texture.blit_to(state, *surfaceTexture); !blit) return unexpected(blit.error());
			state.surface = oldSurface;
			return present(state, releases..., *blit, *surfaceTexture);
		}

	};

	template<>
	struct auto_release<window> : public window {
		using window::window;
		using window::operator=;
		auto_release(window&& o) : window(std::move(o)) {}
	};

WAYLIB_END_NAMESPACE