export module stylizer:context;

import std.compat;
export import stylizer.errors;
export import stylizer.auto_release;
export import stylizer.spans;
export import stylizer.graphics;
export import stylizer.graphics.current_backend;

namespace stylizer {

    export struct context : public graphics::current_backend::device {
		using super = graphics::current_backend::device;

		context() {}
		context(const context&) = delete; // Contexts can't be copied!
		context(context&&) = default; // Contexts shouldn't be moved after reactive objects are created from them...
		context& operator=(const context&) = delete;
		context& operator=(context&&) = delete;

		struct event {
			virtual ~event() {}
		};

		signal<void(context&)> process_events;
		signal<void(const event&)> handle_event;

		static context create_default(const graphics::device::create_config& config = {}) {
			context out;
			static_cast<graphics::current_backend::device&>(out) = graphics::current_backend::device::create_default(config);
			out.process_events.connect([](context& ctx) {
				static_cast<graphics::current_backend::device&>(ctx).process_events();
			});
			return out;
		}

		connection_raw register_default_error_handler() {
			auto& errors = get_error_handler();
			return errors.connect([](error::severity severity, std::string_view message, size_t) {
				if (severity >= error::severity::Error)
					throw error(message);
				std::cerr << message << std::endl;
			});
		}

		static context create_default_with_error_handler(const graphics::device::create_config& config = {}) {
			context{}.register_default_error_handler();
			return create_default(config);
		}

		context& per_frame() {
			process_events(*this);
			return *this;
		}

		template<typename... Tothers>
		context& per_frame(Tothers&... args) {
			constexpr static auto per_frame_invoke = [](context& self, auto& arg) {
				if constexpr (requires () { arg.per_frame(self); }) {
					arg.per_frame(self);
				} else arg.per_frame();
			};
			per_frame();
			(per_frame_invoke(*this, args), ...);
			return *this;
		}

		// TODO: Is there a better name than send?
		void send(error_severity severity, std::string_view message, size_t error_tag = 0) {
			get_error_handler()(severity, message, error_tag);
		}
		void send_error(std::string_view message, size_t error_tag = 0) {
			send(error_severity::Error, message, error_tag);
		}
		void send_warning(std::string_view message, size_t error_tag = 0) {
			send(error_severity::Warning, message, error_tag);
		}
		void send_info(std::string_view message, size_t error_tag = 0) {
			send(error_severity::Info, message, error_tag);
		}
		void send_verbose(std::string_view message, size_t error_tag = 0) {
			send(error_severity::Verbose, message, error_tag);
		}

	protected:
		// Hide some of super's methods
		using super::tick;
	};
}