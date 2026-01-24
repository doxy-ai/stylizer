module;

#include <stylizer/core/util/reaction.hpp>

export module stylizer.window;

import std.compat;
import stdmath;
import stdmath.slang;
import stylizer;

namespace stylizer {

	export struct window : public stylizer::surface {
		enum create_flags {
			None = 0,
			Borderless = (1 << 1),
			Modal = (1 << 2),
			HighPixelDensity = (1 << 3),
			MouseCapture = (1 << 4),
			MouseRelativeMode = (1 << 4),
			Utility = (1 << 5),
			Tooltip = (1 << 6),
			PopupMenu = (1 << 7),
			Transparent = (1 << 8),
			NotFocusable = (1 << 9),
		};

		window() = default;
		window(const window&) = delete;
		window(window&& o) { *this = std::move(o); }
		window& operator=(const window&) = delete;
		window& operator=(window&& o) {
			*reinterpret_cast<surface*>(this) = std::move(o);

			update_as_internal([&]{
				title = std::move(o.title);
				minimum_size = std::move(o.minimum_size);
				maximum_size = std::move(o.maximum_size);
				visible = std::move(o.visible);
				maximized = std::move(o.maximized);
				minimized = std::move(o.minimized);
				focused = std::move(o.focused);
				fullscreen = std::move(o.fullscreen);
				borderless = std::move(o.borderless);
				opacity = std::move(o.opacity);
				close_requested = std::move(o.close_requested);
				resizable = std::move(o.resizable);
				focusable = std::move(o.focusable);
				always_on_top = std::move(o.always_on_top);
				grab_keyboard = std::move(o.grab_keyboard);
				grab_mouse = std::move(o.grab_mouse);
#ifdef __EMSCRIPTEN__
				fill_document = std::move(o.fill_document);
#endif
				position = std::move(o.position);
			});

			return *this;
		}

		// Windows should not be moved after registering event listeners!
		virtual void register_event_listener(context& ctx) = 0;
		virtual void update() = 0;

		reaction::Var<std::string> title;
		reaction::Var<stdmath::uint2> minimum_size, maximum_size; // Used when resizing (0, 0) == no limit
		reaction::Var<bool> visible, maximized, minimized, focused, fullscreen, borderless;
		reaction::Var<float> opacity;
		reaction::Var<bool> close_requested, resizable, focusable, always_on_top, grab_keyboard, grab_mouse;
#ifdef __EMSCRIPTEN__
		reaction::Var<bool> fill_document;
#endif
		reaction::Var<stdmath::int2> position;

		// TODO: Icon

		virtual float content_scaling() = 0;
	};

	export template<typename T>
	concept window_concept = std::derived_from<T, window> && requires(T t, context& ctx, std::string_view title,stdmath::uint2 size, window::create_flags flags) {
		{ T::create(ctx, title, size, flags) } -> std::convertible_to<T>;
		{ t.type } -> std::convertible_to<size_t>;
	};

}

