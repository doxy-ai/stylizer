#pragma once

#include <stylizer/core/api.hpp>

namespace stylizer {

	/**
	 * @brief Represents a window surface that can be displayed and interacted with.
	 *
	 * @code
	 * auto& win = MyWindowType::create(ctx, "Hello Stylizer", {1280, 720});
	 * win.visible = true;
	 * @endcode
	 */
	struct window : public stylizer::surface {
		/**
		 * @brief Registers this window as an event listener in the context.
		 * @param ctx The stylizer context.
		 */
		virtual void register_event_listener(context& ctx) = 0;

		/** @brief Reactive variable for the window title. */
		reaction::Var<std::string> title;
		/** @brief Reactive variables for various window states. */
		reaction::Var<bool> visible, maximized, minimized, fullscreen, borderless, close_requested;
		/** @brief Reactive variable for the window position. */
		reaction::Var<stdmath::vector<int64_t, 2>> position;
		
		/**
		 * @brief Requests the window to gain focus.
		 */
		virtual void focus() = 0;
		/**
		 * @brief Gets the content scaling factor (e.g. for high-DPI displays).
		 * @return The scaling factor.
		 */
		virtual float content_scaling() = 0;

		/** @brief Virtual destructor. */
		virtual ~window() = default;
	};

	/**
	 * @brief Concept for a valid window type.
	 */
	template<typename T>
	concept window_concept = std::derived_from<T, window> && requires(T t, context& ctx, std::string_view title, stdmath::vector<size_t, 2> size) {
		{ T::create(ctx, title, size) } -> std::convertible_to<T>;
		{ t.type } -> std::convertible_to<size_t>;
	};
	
}
