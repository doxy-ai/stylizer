#pragma once

#include "common.hpp"

#include <stylizer/window/api.hpp>

struct SDL_Window;

namespace stylizer::sdl {
	struct window : public stylizer::window {
		size_t type = magic_number;
		bool internal_update = false; // Indicates that an update came from SDL and we shouldn't resize/move in response to it
		SDL_Window* sdl;

		static window create(context& ctx, std::string_view title, stdmath::vector<size_t, 2> size);

		template<typename Tfunc>
		auto update_as_internal(const Tfunc& func) {
			internal_update = true;
			if constexpr (std::is_same_v<decltype(func()), void>) {
				func();
				internal_update = false;
			} else {
				auto out = func();
				internal_update = false;
				return out;
			}
		}

		void register_event_listener(context& ctx) override;
		void focus() override;
		float content_scaling() override;
	
	protected:
		reaction::Action<> title_updater, visible_updater, max_min_updater, fullscreen_borderless_updater, position_updater;
	};

	static_assert(window_concept<window>);
}