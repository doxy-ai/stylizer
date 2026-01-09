#pragma once

#include <stylizer/core/api.hpp>

namespace stylizer {

	struct window : public stylizer::surface {
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