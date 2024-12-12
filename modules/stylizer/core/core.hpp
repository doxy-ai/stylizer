#pragma once

#include "config.hpp"
#include "core.h"

#include "stylizer/api/api.hpp"
#include "thirdparty/thread_pool.hpp"

namespace stylizer {


//////////////////////////////////////////////////////////////////////
// # Thread Pool
//////////////////////////////////////////////////////////////////////


	struct STYLIZER_PREFIXED(thread_pool_future) : std::future<void> {};

	struct thread_pool {
	protected:
		static ZenSepiol::ThreadPool& get_thread_pool(STYLIZER_OPTIONAL(size_t) initial_pool_size = {})
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
		static auto enqueue(F&& function, STYLIZER_OPTIONAL(size_t) initial_pool_size = {}, Args&&... args) {
			return get_thread_pool(initial_pool_size).AddTask(function, args...);
		}
	};


//////////////////////////////////////////////////////////////////////
// # Context
//////////////////////////////////////////////////////////////////////


	struct context: public context_c {
		STYLIZER_API_TYPE(device) device;
		STYLIZER_API_TYPE(surface) surface;
		operator bool() { return device || surface; }
		operator stylizer::api::device&() { return device; } // Automatically convert to an API device!

		context_c& c() { return *(context_c*)this; }
		static context& set_c_pointers(context& cpp) {
			auto& c = cpp.c();
#ifdef STYLIZER_USE_ABSTRACT_API
			c.device = (stylizer_api_device*)cpp.device;
			c.surface = (stylizer_api_surface*)cpp.surface;
#else
			c.device = (stylizer_api_device*)&cpp.device;
			c.surface = (stylizer_api_surface*)&cpp.surface;
#endif
			return cpp;
		}
		static context& set_c_pointers(context&& ctx) { return set_c_pointers(ctx); }

#ifndef STYLIZER_USE_ABSTRACT_API
		static context create_default(stylizer::api::device::create_config config = {}, optional<stylizer::api::current_backend::surface&> surface = {}) {
			config.compatible_surface = surface ? &surface.value : nullptr;
			context out = {};
			out.device = stylizer::api::current_backend::device::create_default(config);
			if(surface) out.surface = std::move(*surface);
			return set_c_pointers(out);
		}
#endif

		void process_events() { device.process_events(); }


		void release(bool static_sub_objects = false) {
			device.release(static_sub_objects);
			surface.release();
		}
	};
} // namespace stylizer