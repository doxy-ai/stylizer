#pragma once

#include <stylizer/core/api.hpp>

namespace stylizer {

	struct window : public stylizer::surface {
		window() = default;
		window(const window&) = delete;
		window(window&& o) { *this = std::move(o); }
		window& operator=(const window&) = delete;
		window& operator=(window&& o) {
			*reinterpret_cast<surface*>(this) = std::move(o);

			title = std::move(o.title);
			visible = std::move(o.visible);
			maximized = std::move(o.maximized);
			minimized = std::move(o.minimized);
			fullscreen = std::move(o.fullscreen);
			borderless = std::move(o.borderless);
			close_requested = std::move(o.close_requested);
			position = std::move(o.position);

			return *this;
		}

		// Windows should not be moved after registering event listeners!
		virtual void register_event_listener(context& ctx) = 0;

		reaction::Var<std::string> title;
		reaction::Var<bool> visible, maximized, minimized, fullscreen, borderless, close_requested;
		reaction::Var<stdmath::vector<int64_t, 2>> position;
		
		virtual void focus() = 0;
		virtual float content_scaling() = 0;
	};

	template<typename T>
	concept window_concept = std::derived_from<T, window> && requires(T t, context& ctx, std::string_view title, stdmath::vector<size_t, 2> size) {
		{ T::create(ctx, title, size) } -> std::convertible_to<T>;
		{ t.type } -> std::convertible_to<size_t>;
	};
	
}