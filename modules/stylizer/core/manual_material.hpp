#pragma once

#include "api.hpp"

namespace stylizer {

	// Material where every aspect is configured manually
	template<typename Tconfig = void>
	struct manual_material : public material {
		using material::material;

		static manual_material create(context& ctx, const basic_geometry_buffer& gbuffer) {
			assert(false && "Manual material should use the create_pipeline function");
			return {};
		}
		static manual_material create_pipeline(context& ctx, const pipeline::entry_points& entry_points, const basic_geometry_buffer& gbuffer, const render_pipeline::config& config = {}, const std::string_view label = "Stylizer Manual Material Pipeline") {
			return {ctx.create_render_pipeline(entry_points, gbuffer.color_attachments(), gbuffer.depth_stencil_attachment(), config, label)};
		}


		std::conditional_t<std::is_same_v<Tconfig, void>, float, Tconfig> config = {};
		std::vector<maybe_owned<api::current_backend::texture>> manual_textures;
		std::vector<maybe_owned<api::current_backend::buffer>> manual_buffers;
		std::vector<std::string_view> manual_requested_mesh_attributes;
		std::vector<api::current_backend::bind_group> manual_bind_groups;

		std::span<const std::byte> config_data() override { return byte_span(span_from_value(config)); }
		std::span<maybe_owned<api::current_backend::texture>> textures() override { return manual_textures; }
		std::span<maybe_owned<api::current_backend::buffer>> buffers() override { return manual_buffers; }
		std::span<std::string_view> requested_mesh_attributes() override { return manual_requested_mesh_attributes; }

		std::span<api::current_backend::bind_group> bind_groups() override { return manual_bind_groups; }
	};

	static_assert(material_concept<manual_material<void>>);

}