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

}