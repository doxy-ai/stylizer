#pragma once

#include <stylizer/core/api.hpp>

namespace stylizer {

	/**
	 * @brief Represents a system window.
	 *
	 * Extends stylizer::surface, providing a target for rendering and a way to handle window-specific events and properties.
	 *
	 * @code{.cpp}
	 * // Example: Creating a window and setting its title
	 * auto& win = stylizer::current_backend::window::create(ctx, "My Stylizer App", {800, 600});
	 * win.title = "New Title";
	 * @endcode
	 */
	struct window : public stylizer::surface {
		virtual void register_event_listener(context& ctx) = 0;

		reaction::Var<std::string> title; ///< Reactive window title.
		reaction::Var<bool> visible, maximized, minimized, fullscreen, borderless, close_requested; ///< Reactive window state properties.
		reaction::Var<stdmath::vector<int64_t, 2>> position; ///< Reactive window position.
		
		/**
		 * @brief Requests that the window be focused.
		 */
		virtual void focus() = 0;

		/**
		 * @brief Returns the content scaling factor (DPI scaling).
		 */
		virtual float content_scaling() = 0;
	};

	template<typename T>
	concept window_concept = std::derived_from<T, window> && requires(T t, context& ctx, std::string_view title, stdmath::vector<size_t, 2> size) {
		{ T::create(ctx, title, size) } -> std::convertible_to<T>;
		{ t.type } -> std::convertible_to<size_t>;
	};
	
}