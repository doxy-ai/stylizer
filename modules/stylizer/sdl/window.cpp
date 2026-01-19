#include "window.hpp"
#include "event.hpp"
#include "cstring_from_view.hpp"
#include "stylizer/core/util/maybe_owned.hpp"
#include "util/flags.hpp"

#include <stylizer/api/sdl3.hpp>

namespace stylizer::sdl {

	window& window::operator=(window&& o) {
		*reinterpret_cast<stylizer::window*>(this) = std::move(o);

		sdl = std::exchange(o.sdl, nullptr);

		update_as_internal([&] {
			o.title_updater.close();
			title_updater = reaction::action([this](std::string_view title) { title_updater_impl(title); }, title);

			o.min_max_size_updater.close();
			min_max_size_updater = reaction::action([this](const stdmath::vector<uint32_t, 2>& min, const stdmath::vector<uint32_t, 2>& max) {
				min_max_size_updater_impl(min, max);
			}, minimum_size, maximum_size);

			o.visible_updater.close();
			visible_updater = reaction::action([this](bool visible) {
				visible_updater_impl(visible);
			}, visible);

			o.max_min_updater.close();
			max_min_updater = reaction::action([this](bool maximized, bool minimized) {
				max_min_updater_impl(maximized, minimized);
			}, maximized, minimized);

			o.fullscreen_borderless_updater.close();
			fullscreen_borderless_updater = reaction::action([this](bool fullscreen, bool borderless) {
				fullscreen_borderless_updater_impl(fullscreen, borderless);
			}, fullscreen, borderless);

			o.opacity_updater.close();
			opacity_updater = reaction::action([this](float opacity) {
				opacity_updater_impl(opacity);
			}, opacity);

			o.resizable_updater.close();
			resizable_updater = reaction::action([this](bool resizable) {
				resizable_updater_impl(resizable);
			}, resizable);

			o.focusable_updater.close();
			focusable_updater = reaction::action([this](bool focusable) {
				focusable_updater_impl(focusable);
			}, focusable);

			o.always_on_top_updater.close();
			always_on_top_updater = reaction::action([this](bool always_on_top) {
				always_on_top_updater_impl(always_on_top);
			}, always_on_top);

			o.grab_keyboard_updater.close();
			grab_keyboard_updater = reaction::action([this](bool grabbed) {
				grab_keyboard_updater_impl(grabbed);
			}, grab_keyboard);

			o.grab_mouse_updater.close();
			grab_mouse_updater = reaction::action([this](bool grabbed) {
				grab_mouse_updater_impl(grabbed);
			}, grab_mouse);

	#ifdef __EMSCRIPTEN__
			o.fill_document_updater.close();
			fill_document_updater = reaction::action([this](bool fill) {
				fill_document_updater_impl(fill);
			}, fill_document);
	#endif

			o.position_updater.close();
			position_updater = reaction::action([this](const stdmath::vector<int32_t, 2>& position) {
				position_updater_impl(position);
			}, position);
		});

		return *this;
	}

	window window::create(context& ctx, std::string_view title, stdmath::vector<size_t, 2> size, create_flags flags /* = create_flags::None */) {
		get_global_sdl_event_handler(); // Setup SDL

		SDL_WindowFlags sdl_flags = 0;
		if(api::flags_set(flags, create_flags::Borderless))
			sdl_flags |= SDL_WINDOW_BORDERLESS;
		if(api::flags_set(flags, create_flags::Modal))
			sdl_flags |= SDL_WINDOW_MODAL;
		if(api::flags_set(flags, create_flags::HighPixelDensity))
			sdl_flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
		if(api::flags_set(flags, create_flags::MouseCapture))
			sdl_flags |= SDL_WINDOW_MOUSE_CAPTURE;
		if(api::flags_set(flags, create_flags::MouseRelativeMode))
			sdl_flags |= SDL_WINDOW_MOUSE_RELATIVE_MODE;
		if(api::flags_set(flags, create_flags::Utility))
			sdl_flags |= SDL_WINDOW_UTILITY;
		if(api::flags_set(flags, create_flags::Tooltip))
			sdl_flags |= SDL_WINDOW_TOOLTIP;
		if(api::flags_set(flags, create_flags::PopupMenu))
			sdl_flags |= SDL_WINDOW_POPUP_MENU;
		if(api::flags_set(flags, create_flags::Transparent))
			sdl_flags |= SDL_WINDOW_TRANSPARENT;
		if(api::flags_set(flags, create_flags::NotFocusable))
			sdl_flags |= SDL_WINDOW_NOT_FOCUSABLE;

		window out;
		out.type = magic_number;
		out.sdl = SDL_CreateWindow(cstring_from_view(title), size.x, size.y, sdl_flags);
		if (!out.sdl) {
			get_error_handler()(api::error::severity::Error, "Failed to create SDL window", 0);
			return {};
		}

		out.title = reaction::var(std::string(title));
		out.title_updater = reaction::action([]{});

		out.minimum_size = reaction::var(stdmath::vector<uint32_t, 2>{0});
		out.maximum_size = reaction::var(stdmath::vector<uint32_t, 2>{0});
		out.min_max_size_updater = reaction::action([]{});

		out.visible = reaction::var(true);
		out.visible_updater = reaction::action([]{});

		out.maximized = reaction::var(false);
		out.minimized = reaction::var(false);
		out.max_min_updater = reaction::action([]{});

		out.focused = reaction::var(false);

		out.fullscreen = reaction::var(false);
		out.borderless = reaction::var(false);
		out.fullscreen_borderless_updater = reaction::action([]{});

		out.opacity = reaction::var(1.f);
		out.opacity_updater = reaction::action([]{});

		out.close_requested = reaction::var(false);

		out.resizable = reaction::var(false);
		out.resizable_updater = reaction::action([]{});

		out.focusable = reaction::var(true);
		out.focusable_updater = reaction::action([]{});

		out.always_on_top = reaction::var(false);
		out.always_on_top_updater = reaction::action([]{});

		out.grab_keyboard = reaction::var(false);
		out.grab_keyboard_updater = reaction::action([]{});

		out.grab_mouse = reaction::var(false);
		out.grab_mouse_updater = reaction::action([]{});

#ifdef __EMSCRIPTEN__
		out.fill_document = reaction::var(false);
		out.fill_document_updater = reaction::action([]{});
#endif

		int x, y;
		SDL_GetWindowPosition(out.sdl, &x, &y);
		out.position = reaction::var(stdmath::vector<int32_t, 2>{x, y});
		out.position_updater = reaction::action([]() {});

		auto surface = api::sdl3::create_surface<api::current_backend::surface>(out.sdl);
		static_cast<stylizer::surface&>(out) = stylizer::surface::create(ctx, surface, size);

		out.update_as_internal([&] {
			SDL_GetWindowSize(out.sdl, &x, &y);
			update_if_different<stdmath::vector<uint32_t, 2>>(out.size, stdmath::vector<int32_t, 2>{x, y});
		});

		return out;
	}

		void window::register_event_listener(context& ctx) {
		if(!get_global_sdl_event_handler())
			get_global_sdl_event_handler() = setup_sdl_events(ctx);

		ctx.handle_event.connect([this](const stylizer::context::event& e) {
			auto eve = event2sdl(e);
			if(!eve) return;
			auto& event = eve->sdl;

			auto us = SDL_GetWindowID(sdl);
			if(event.window.windowID != us)
				return;

			// std::cout << event.type << std::endl;

			update_as_internal([&]{
				switch (event.type) {
				break; case SDL_EVENT_WINDOW_SHOWN:
					update_if_different(visible, true);
				break; case SDL_EVENT_WINDOW_HIDDEN:
					update_if_different(visible, false);
				// break; case SDL_EVENT_WINDOW_MOVED:
				// 	// TODO: Why don't these update?
				// 	position.value(stdmath::vector<int64_t, 2>{event.window.data1, event.window.data2});
				// break; case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
				// 	// TODO: Why don't these update?
				// 	size.value(stdmath::vector<int64_t, 2>{event.window.data1, event.window.data2});
				break; case SDL_EVENT_WINDOW_MINIMIZED:
					update_if_different(minimized, true); // NOTE: These won't update on wayland... do we care?
				break; case SDL_EVENT_WINDOW_MAXIMIZED:
					update_if_different(maximized, true);
				break; case SDL_EVENT_WINDOW_RESTORED:
					reaction::batchExecute([&] {
						update_if_different(minimized, false);
						update_if_different(maximized, false);
					});
				break; case SDL_EVENT_WINDOW_FOCUS_GAINED:
					update_if_different(focused, true);
				break; case SDL_EVENT_WINDOW_FOCUS_LOST:
					update_if_different(focused, false);
				break; case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
					update_if_different(close_requested, true);
				break; case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
					update_if_different(fullscreen, true);
				break; case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
					update_if_different(fullscreen, false);
				}
			});
		});
	}

	void window::update() {
		update_as_internal([this]{ reaction::batchExecute([this]{
			int x, y;
			SDL_GetWindowPosition(sdl, &x, &y);
			update_if_any_different(position, {x, y});

			SDL_GetWindowSize(sdl, &x, &y);
			update_if_any_different<stdmath::vector<uint32_t, 2>>(size, stdmath::vector<int32_t, 2>{x, y});
		}); });
	}

	float window::content_scaling() {
		return SDL_GetDisplayContentScale(SDL_GetDisplayForWindow(sdl));
	}

	void window::title_updater_impl(std::string_view title) {
		if(internal_update) return;

		SDL_SetWindowTitle(sdl, cstring_from_view(title));
	}

	void window::min_max_size_updater_impl(const stdmath::vector<uint32_t, 2>& min, const stdmath::vector<uint32_t, 2>& max) {
		if(internal_update) return;

		if(maximized && !minimized) SDL_MaximizeWindow(sdl);
		if(minimized && !maximized) SDL_MinimizeWindow(sdl);
		else SDL_RestoreWindow(sdl);
	}

	void window::visible_updater_impl(bool visible) {
		if(internal_update) return;

		if(visible) SDL_ShowWindow(sdl);
		else SDL_HideWindow(sdl);
	}

	void window::max_min_updater_impl(bool maximized, bool minimized) {
		if(internal_update) return;

		if(maximized && !minimized) SDL_MaximizeWindow(sdl);
		if(minimized && !maximized) SDL_MinimizeWindow(sdl);
		else SDL_RestoreWindow(sdl);
	}

	void window::fullscreen_borderless_updater_impl(bool fullscreen, bool borderless) {
		if(internal_update) return;

		SDL_SetWindowFullscreen(sdl, fullscreen);

		auto modes = SDL_GetFullscreenDisplayModes(SDL_GetDisplayForWindow(sdl), nullptr);
		SDL_SetWindowFullscreenMode(sdl, borderless ? nullptr : modes[0]);
	}

	void window::opacity_updater_impl(float opacity) {
		if(internal_update) return;

		SDL_SetWindowOpacity(sdl, opacity);
	}

	void window::resizable_updater_impl(bool resizable) {
		if(internal_update) return;

		SDL_SetWindowResizable(sdl, resizable);
	}

	void window::focusable_updater_impl(bool focusable) {
		if(internal_update) return;

		SDL_SetWindowFocusable(sdl, focusable);
	}

	void window::always_on_top_updater_impl(bool always_on_top) {
		if(internal_update) return;

		SDL_SetWindowAlwaysOnTop(sdl, always_on_top);
	}

	void window::grab_keyboard_updater_impl(bool grabbed) {
		if(internal_update) return;

		SDL_SetWindowKeyboardGrab(sdl, grabbed);
	}

	void window::grab_mouse_updater_impl(bool grabbed) {
		if(internal_update) return;

		SDL_SetWindowMouseGrab(sdl, grabbed);
	}

#ifdef __EMSCRIPTEN__
	void window::fill_document_updater_impl(bool fill_document) {
		if(internal_update) return;

		SDL_SetWindowFillDocument(sdl, fill_document);
	}
#endif

	void window::position_updater_impl(const stdmath::vector<int32_t, 2>& position) {
		if(internal_update) return;

		SDL_SetWindowPosition(sdl, position.x, position.y);
	}

}