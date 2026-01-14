#pragma once

#include <stylizer/core/api.hpp>
#include <stylizer/window/api.hpp>

#include <imgui.h>

namespace stylizer {

	struct imgui {
		imgui(context& ctx, window& window, api::texture::format color_format, api::texture::format depth_format = api::texture::format::Undefined);
		~imgui();

		stylizer::connection event_listener_handle;
		ImFont* default_font = nullptr;

		void begin_frame();
		void end_frame();

		template<typename Tfunction>
		void new_frame(const Tfunction& func) {
			begin_frame();
			func();
			end_frame();
		}

		stylizer::api::render::pass& draw(stylizer::api::render::pass& render_pass);
	};

}