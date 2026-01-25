module;

// #include <stylizer/core/util/reaction.hpp>
#include <stylizer/core/util/move_and_make_owned_macros.hpp>
#include <stylizer/graphics/util/error_macros.hpp>
#include <cassert>

export module stylizer.model:model;

import std.compat;
// import stdmath;
// import stdmath.slang;
import stylizer;

import :mesh;
import :instance_data;

#include <stylizer/core/material.flat.hpp>

namespace stylizer { inline namespace models {

	export struct model : public std::vector<std::pair<maybe_owned<mesh>, maybe_owned<material>>> { STYLIZER_MOVE_AND_MAKE_OWNED_METHODS(model)

		// static std::unordered_map<std::string, std::function<maybe_owned<model>(context&, std::span<std::byte>, std::string_view)>>& model::get_loader_set() {
		// 	static std::unordered_map<std::string, std::function<maybe_owned<model>(context&, std::span<std::byte>, std::string_view)>> loaders = []{
		// 		std::unordered_map<std::string, std::function<maybe_owned<model>(context&, std::span<std::byte>, std::string_view)>> out;
		// 		// out[".obj"] = load_tinyobj_model_generic;
		// 		return out;
		// 	}();
		// 	return loaders;
		// }
		// static maybe_owned<model> model::load(context& ctx, std::filesystem::path file) {
		// 	return load_file(ctx, file, get_loader_set()[file.extension().string()]);
		// }
		static std::unordered_map<std::string, std::function<maybe_owned<model>(context&, std::span<std::byte>, std::string_view)>>& get_loader_set() { STYLIZER_THROW("Not implemented yet"); }
		static maybe_owned<model> load(context& ctx, std::filesystem::path file) { STYLIZER_THROW("Not implemented yet"); }

		model& override_materials(material& override_material) {
			for(auto& [mesh, mat]: *this)
				mat = &override_material;
			return *this;
		}

		model& upload(context& ctx, const frame_buffer& fb) {
			for(auto& [mesh, material]: *this) {
				if(mesh.owned) 
					mesh->rebuild_gpu_caches(ctx);

				if(material.owned) {
					auto flat_mat = dynamic_cast<flat_material*>(&*material);
					if(!flat_mat) continue;
					
					flat_mat->create_from_configured(ctx, fb);
				}
			}
			return *this;
		}

		graphics::current_backend::render::pass& draw_instanced(
			context& ctx, graphics::current_backend::render::pass& render_pass,
			instance_data::buffer_base& instance_data, std::optional<utility_buffer> util = {}
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

		instance_data::buffer<1> instance_data_cache;
		graphics::current_backend::render::pass& draw(context& ctx, graphics::current_backend::render::pass& render_pass, const instance_data& instance_data = {}, std::optional<utility_buffer> util = {}) {
			instance_data_cache[0] = {instance_data};
			instance_data_cache.upload(ctx, "Stylizer Single Instance Data");

			return draw_instanced(ctx, render_pass, instance_data_cache, util);
		}
    };

}}
