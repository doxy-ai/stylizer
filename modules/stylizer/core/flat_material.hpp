#pragma once

#include "api.hpp"
#include "backends/webgpu/webgpu.hpp"
#include "stylizer/core/util/maybe_owned.hpp"

namespace stylizer {

	struct flat_material : public material {
		using color_t = std::variant<stdmath::vector<float, 4>, maybe_owned<texture>>;
		color_t color;
		struct config {
			stdmath::vector<float, 4> color;
			uint32_t use_texture = false; 
			std::array<float, 3> padding; // TODO: Why does slang align this so large!?!?
			enum srgb_state : uint32_t {
				InputLinearOutputLinear = 0,
				InputLinearOutputSRGB = (1 << 1),
				InputSRGBOutputLinear = (1 << 0),
				InputSRGBOutputSRGB = (1 << 1 | 1 << 0)
			} srgb;
		} config;

		api::current_backend::shader vertex, fragment;
		api::current_backend::buffer config_buffer;
		api::current_backend::bind_group group;

		flat_material& create_from_configured(context& ctx, const frame_buffer& fb);
		static flat_material create(context& ctx, const frame_buffer& fb, const color_t& initial_color = stdmath::vector<float, 4>{.5, .5, .5, 1});

		std::span<maybe_owned<api::current_backend::texture>> textures(context& ctx) override;
		std::span<maybe_owned<api::current_backend::buffer>> buffers(context& ctx) override;
		std::span<std::string_view> requested_mesh_attributes() override;
		std::span<api::current_backend::bind_group> bind_groups(context& ctx) override;

		flat_material& upload(context& ctx);
	};
}