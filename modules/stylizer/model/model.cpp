#include "api.hpp"

namespace stylizer {
	void model::override_materials(material& override_material) {
		for(auto& [mesh, mat]: *this)
			mat = &override_material;
	}

	api::current_backend::render::pass& model::draw_instanced(
		context& ctx, api::current_backend::render::pass& render_pass,
		instance_data::buffer_base& instance_data, std::optional<utility_buffer> util /* = {} */
	) {
		for(auto& [mesh_, material_]: *this) {
			auto& mesh = *mesh_; auto& material = *material_;

			render_pass.bind_render_pipeline(ctx, material);
			render_pass.bind_render_group(ctx, instance_data.make_bind_group(ctx, material, util, 0));
			for(auto& group: material.bind_groups(ctx))
				render_pass.bind_render_group(ctx, group);

			// TODO: implement meshlets
			if(mesh.meshlets_view())
				assert(false && "Meshlets are not currently supported!");
			auto* index_buffer = mesh.get_index_buffer(ctx);
			if(index_buffer) render_pass.bind_index_buffer(ctx, *index_buffer);
			auto vertex_buffers = mesh.get_vertex_buffers(ctx, mesh.build_attribute_list(material.requested_mesh_attributes()));
			for(size_t i = 0; i < vertex_buffers.size(); ++i)
				render_pass.bind_vertex_buffer(ctx, i, vertex_buffers[i]);

			if(index_buffer)
				render_pass.draw_indexed(ctx, mesh.index_count());
			else render_pass.draw(ctx, mesh.vertex_count(), instance_data.count());
		}

		return render_pass;
	}

	api::current_backend::render::pass& model::draw(
		context& ctx, api::current_backend::render::pass& render_pass, 
		const instance_data& instance_data /* = {} */, std::optional<utility_buffer> util /* = {} */
	) {
		instance_data_cache[0] = {instance_data};
		instance_data_cache.upload(ctx, "Stylizer Single Instance Data");

		return draw_instanced(ctx, render_pass, instance_data_cache, util);
	}
}