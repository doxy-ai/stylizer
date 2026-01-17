#pragma once

#include "api.hpp"

namespace stylizer {

	// Material where every aspect is configured manually
	struct manual_material : public material {
		using material::material;

		static manual_material create(context& ctx, const frame_buffer& fb) {
			assert(false && "Manual material should use the create_pipeline function");
			return {};
		}
		static manual_material create_pipeline(context& ctx, const pipeline::entry_points& entry_points, const frame_buffer& fb, const render_pipeline::config& config = {}, const std::string_view label = "Stylizer Manual Material Pipeline") {
			return {ctx.create_render_pipeline(entry_points, fb.color_attachments(), fb.depth_stencil_attachment(), config, label)};
		}

		std::vector<maybe_owned<api::current_backend::texture>> manual_textures;
		std::vector<maybe_owned<api::current_backend::buffer>> manual_buffers;
		std::vector<std::string_view> manual_requested_mesh_attributes;
		std::vector<api::current_backend::bind_group> manual_bind_groups;

		std::span<maybe_owned<api::current_backend::texture>> textures(context&) override { return manual_textures; }
		std::span<maybe_owned<api::current_backend::buffer>> buffers(context&) override { return manual_buffers; }
		std::span<std::string_view> requested_mesh_attributes() override { return manual_requested_mesh_attributes; }

		std::span<api::current_backend::bind_group> bind_groups(context&) override { return manual_bind_groups; }
	};

	static_assert(material_concept<manual_material>);

}