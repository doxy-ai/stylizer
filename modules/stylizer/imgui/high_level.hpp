#pragma once
#include <stylizer/core/api.hpp>
#include <stylizer/window/api.hpp>

#include <imgui.h>

namespace stylizer {

	inline ImVec2 convert(const stdmath::float2& v) {
		return {v.x, v.y};
	}
	inline stdmath::float2 convert(const ImVec2& v) {
		return {v.x, v.y};
	}

	inline ImVec4 convert(const stdmath::color32& c) {
		return {c.x, c.y, c.z, c.w};
	}
	inline stdmath::color32 convert(const ImVec4& c) {
		return {c.x, c.y, c.z, c.w};
	}

	struct imgui {
		imgui(context& ctx, window& window, std::optional<api::texture::format> color_format_override = {}, api::texture::format depth_format = api::texture::format::Undefined);
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