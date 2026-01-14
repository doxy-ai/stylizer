#pragma once

#include <stylizer/api/api.hpp>
#include <stylizer/api/backends/current_backend.hpp>
#include <reaction/reaction.h>

#include <chrono>
#include <cstddef>
#include <ratio>

namespace stylizer {

	struct context : public api::current_backend::device {
		struct event {
			virtual ~event() {}
		};

		stylizer::signal<void(context&)> process_events;
		stylizer::signal<void(const event&)> handle_event;

		void update() {
			process_events(*this);
		}

		static context create_default(const api::device::create_config& config = {}) {
			context out;
			static_cast<api::current_backend::device&>(out) = api::current_backend::device::create_default(config);
			out.process_events.connect([](context& ctx) {
				static_cast<api::current_backend::device&>(ctx).process_events();
			});
			return out;
		}
	};	

	struct time {
		float total = 0, delta = 0, smoothed_delta = 0;
		size_t frame = 0;

		template<typename Tprecision = std::micro, typename Tclock = std::chrono::system_clock>
		inline time& compute_delta() {
			static auto last_time = Tclock::now();
			auto now = Tclock::now();
			auto dt_precise = std::chrono::duration_cast<std::chrono::duration<int64_t, Tprecision>>(now - last_time).count();
			delta = stdmath::stl::float64_t(dt_precise * Tprecision::num) / Tprecision::den;
			last_time = now;
			return *this;
		}

		inline time& increment_frame() {
			++frame;
			return *this;
		}

		template<typename Tprecision = std::micro, typename Tclock = std::chrono::system_clock>
		inline time& compute_delta_and_frame() {
			compute_delta<Tprecision, Tclock>();
			increment_frame();
			return *this;
		}

		template<typename Tprecision = std::micro, typename Tclock = std::chrono::system_clock>
		inline time& update(float alpha = .9) {
			compute_delta_and_frame<Tprecision, Tclock>();
			total += delta;
			smoothed_delta = alpha * smoothed_delta + (1 - alpha) * delta;
			return *this;
		}
	};

}