#pragma once

#include "glfwpp/event.h"
#include "waylib/core/core.hpp"
#include "waylib/core/utility.hpp"

#include <glfwpp/glfwpp.h>
#include <glfw3webgpu.h>

WAYLIB_BEGIN_NAMESPACE

	extern "C" {
		#include "window.h"
	}

	struct window {
		glfw::Window raw;

		static struct window create(vec2u size, std::string_view name = "Waylib", window_config config = {});

		bool should_close(bool poll_events = true) const {
			if(poll_events) glfw::pollEvents();
			return raw.shouldClose();
		}

		vec2u get_dimensions() const {
			vec2u out; std::tie(out.x, out.y) = raw.getSize();
			return out;
		}

		wgpu::Surface get_surface(WGPUInstance instance) const {
			return glfwCreateWindowWGPUSurface(instance, raw);
		}

		result<wgpu_state> create_default_state(bool prefer_low_power = false) const WAYLIB_TRY {
			auto partial = wgpu_state::create_default(nullptr, prefer_low_power);
			if(!partial) return partial;
			partial->surface = get_surface(partial->instance);
			return partial;
		} WAYLIB_CATCH

		void configure_surface(wgpu_state& state, surface_configuration config = {}) const {
			state.configure_surface(get_dimensions(), config);
		}

		void reconfigure_surface_on_resize(wgpu_state& state, surface_configuration config = {}) {
			raw.sizeEvent.append([&state, config](glfw::Window&, int x, int y){
				state.configure_surface({x, y}, config);
			});
			if(config.automatic_should_configure_now) configure_surface(state, config);
		}

	};

	template<>
	struct auto_release<window> : public window {
		using window::window;
		using window::operator=;
		auto_release(window&& o) : window(std::move(o)) {}
	};

WAYLIB_END_NAMESPACE