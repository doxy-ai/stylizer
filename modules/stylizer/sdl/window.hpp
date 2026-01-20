#pragma once

#include "common.hpp"

#include <stylizer/window/api.hpp>

struct SDL_Window;

namespace stylizer::sdl {
	struct window : public stylizer::window { STYLIZER_MOVE_AND_MAKE_OWNED_DERIVED_METHODS(window, surface)
		size_t type = magic_number;
		SDL_Window* sdl;

		window() {}
		window(window&& o) { *this = std::move(o); }
		window& operator=(window&& o);

		static window create(context& ctx, std::string_view title,stdmath::uint2 size, create_flags flags = create_flags::None);

		void register_event_listener(context& ctx) override;
		void update() override;
		float content_scaling() override;

	protected:
		reaction::Action<> title_updater; void title_updater_impl(std::string_view title);
		reaction::Action<> min_max_size_updater; void min_max_size_updater_impl(const stdmath::uint2& min, const stdmath::uint2& max);
		reaction::Action<> visible_updater; void visible_updater_impl(bool visible);
		reaction::Action<> max_min_updater; void max_min_updater_impl(bool maximized, bool minimized);
		// reaction::Action<> focused_updater; void focused_updater_impl(bool focused);
		reaction::Action<> fullscreen_borderless_updater; void fullscreen_borderless_updater_impl(bool fullscreen, bool borderless);
		reaction::Action<> opacity_updater; void opacity_updater_impl(float opacity);
		reaction::Action<> resizable_updater; void resizable_updater_impl(bool resizable);
		reaction::Action<> focusable_updater; void focusable_updater_impl(bool focusable);
		reaction::Action<> always_on_top_updater; void always_on_top_updater_impl(bool always_on_top);
		reaction::Action<> grab_keyboard_updater; void grab_keyboard_updater_impl(bool grab_keyboard);
		reaction::Action<> grab_mouse_updater; void grab_mouse_updater_impl(bool grab_mouse);
#ifdef __EMSCRIPTEN__
		reaction::Action<> fill_document_updater; void fill_document_updater_impl(bool fill_document);
#endif
		reaction::Action<> position_updater; void position_updater_impl(const stdmath::int2& position);
	};

	static_assert(window_concept<window>);
}