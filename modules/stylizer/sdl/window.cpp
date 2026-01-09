#include "window.hpp"
#include "event.hpp"
#include "cstring_from_view.hpp"

#include <stylizer/api/sdl3.hpp>

namespace stylizer::sdl {
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
					visible.value(true);
				break; case SDL_EVENT_WINDOW_HIDDEN:
					visible.value(false);
				break; case SDL_EVENT_WINDOW_MOVED:
					// TODO: Why don't these update?
					position.value(stdmath::vector<int64_t, 2>{event.window.data1, event.window.data2});
				break; case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
					// TODO: Why don't these update?
					size.value(stdmath::vector<int64_t, 2>{event.window.data1, event.window.data2});
				break; case SDL_EVENT_WINDOW_MINIMIZED:
					minimized.value(true); // NOTE: These won't update on wayland... do we care?
				break; case SDL_EVENT_WINDOW_MAXIMIZED:
					maximized.value(true);
				break; case SDL_EVENT_WINDOW_RESTORED:
					reaction::batchExecute([&] {
						minimized.value(false);
						maximized.value(false);
					}); 
				break; case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
					close_requested.value(true);
				break; case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
					fullscreen.value(true);
				break; case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
					fullscreen.value(false);
				}
			});
		});
	}

	window window::create(context& ctx, std::string_view title, stdmath::vector<size_t, 2> size) {
		get_global_sdl_event_handler(); // Setup SDL

		window out;
		out.type = magic_number;
		out.sdl = SDL_CreateWindow(cstring_from_view(title), size.x, size.y, 0); // TODO: Flags?
		if (!out.sdl) {
			get_error_handler()(api::error::severity::Error, "Failed to create SDL window", 0);
			return {};
		}

		out.title = reaction::var(std::string(title));
		out.title_updater = reaction::action([&out](std::string_view title) {
			if(out.internal_update) return;
			
			SDL_SetWindowTitle(out.sdl, cstring_from_view(title));
		}, out.title);

		out.visible = reaction::var(true);
		out.visible_updater = reaction::action([&out](bool visible) {
			if(out.internal_update) return;
			
			if(visible) SDL_ShowWindow(out.sdl);
			else SDL_HideWindow(out.sdl);
		}, out.visible);

		out.maximized = reaction::var(false);
		out.minimized = reaction::var(false);
		out.max_min_updater = reaction::action([&out](bool maximized, bool minimized) {
			if(out.internal_update) return;
			
			if(maximized && !minimized) SDL_MaximizeWindow(out.sdl);
			if(minimized && !maximized) SDL_MinimizeWindow(out.sdl);
			else SDL_RestoreWindow(out.sdl);
		}, out.maximized, out.minimized);

		out.fullscreen = reaction::var(false);
		out.borderless = reaction::var(true);
		out.fullscreen_borderless_updater = reaction::action([&out](bool fullscreen, bool borderless){
			if(out.internal_update) return;

			SDL_SetWindowFullscreen(out.sdl, fullscreen);

			auto modes = SDL_GetFullscreenDisplayModes(SDL_GetDisplayForWindow(out.sdl), nullptr);
			SDL_SetWindowFullscreenMode(out.sdl, borderless ? nullptr : modes[0]);
		}, out.fullscreen, out.borderless);

		out.close_requested = reaction::var(false);

		int x, y;
		SDL_GetWindowPosition(out.sdl, &x, &y);
		out.position = reaction::var(stdmath::vector<int64_t, 2>{x, y});
		out.position_updater = reaction::action([&out](const stdmath::vector<int64_t, 2> position){
			if(out.internal_update) return;

			SDL_SetWindowPosition(out.sdl, position.x, position.y);
		}, out.position);

		auto surface = api::sdl3::create_surface<api::current_backend::surface>(out.sdl);
		static_cast<stylizer::surface&>(out) = std::move(stylizer::surface::create(ctx, surface, size));
		return out;
	}

	void window::focus() {
		SDL_RaiseWindow(sdl);
	}

	float window::content_scaling() {
		return SDL_GetDisplayContentScale(SDL_GetDisplayForWindow(sdl));
	}
}