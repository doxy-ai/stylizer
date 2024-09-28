#pragma once

#include "config.hpp"
#include <webgpu/webgpu.hpp>
#include <cstdint>
#include <sstream>

#include "utility.hpp"
#include "wgsl_types.hpp"


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
		wgpu_state(WGPUInstance i, WGPUAdapter a, WGPUDevice d, WGPUSurface s = nullptr) : wgpu_stateC(i, a, d, s) {}
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
			WGPUInstance instance = 1;
#endif
			return default_from_instance(instance, surface, prefer_low_power);
		} WAYLIB_CATCH

		operator bool() { return instance && adapter && device; } // NOTE: Bool conversion needed for auto_release
		void release(bool adapter_release = true, bool instance_release = true) {
			device.getQueue().release();
			// We expect the device to be lost when it is released... so don't do anything to stop it
			auto callback = device.setDeviceLostCallback([](WGPUDeviceLostReason reason, char const* message) {});
			device.release();
			if(surface) surface.release();
			if(adapter_release) adapter.release();
			if(instance_release) instance.release();
		}

		void configure_surface(vec2u size, surface_configuration config = {}) {
			surface.configure(WGPUSurfaceConfiguration {
				.device = device,
				.format = determine_best_surface_format(*this),
				.usage = wgpu::TextureUsage::RenderAttachment,
				.alphaMode = config.alpha_mode,
				.width = size.x,
				.height = size.y,
				.presentMode = config.presentation_mode ? static_cast<wgpu::PresentMode>(*config.presentation_mode) : determine_best_presentation_mode(*this)
			});
		}
	};


	extern "C" {
		#include "core.h"
	}

WAYLIB_END_NAMESPACE