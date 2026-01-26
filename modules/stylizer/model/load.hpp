#pragma once

// #include "thirdparty/tinyobjloader.h"

#include "model.hpp"
#include "dynamic_mesh.hpp"

namespace stylizer { inline namespace models {

	inline model load_tinyobj_model_with_material(stylizer::context& ctx, std::span<std::byte> memory, std::span<std::byte> mtl) {
		tinyobj::ObjReader reader;

		tinyobj::ObjReaderConfig reader_config;
		reader_config.mtl_search_path = std::filesystem::current_path().c_str(); // Path to material files
		reader_config.vertex_color = true;

		if (!reader.ParseFromString(std::string((char*)memory.data(), (char*)(memory.data() + memory.size())), std::string((char*)mtl.data(), (char*)(mtl.data() + mtl.size())), reader_config))
			if (!reader.Error().empty())
				stylizer::get_error_handler()(stylizer::error_severity::Error, reader.Error(), 0);

		if (!reader.Warning().empty())
			stylizer::get_error_handler()(stylizer::error_severity::Warning, reader.Warning(), 0);

		auto& attrib = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		auto& materials = reader.GetMaterials();

		stylizer::model out;
		struct attributes {
			stylizer::storage<stdmath::float4> positions;
			stylizer::storage<stdmath::float4> colors;
			stylizer::storage<stdmath::float4> normals;
			stylizer::storage<stdmath::float2> uvs;

			stylizer::dynamic_mesh make_mesh() {
				stylizer::dynamic_mesh mesh;
				mesh.add_vertex_attribute(stylizer::common_mesh_attributes::positions, std::move(positions));
				mesh.add_vertex_attribute(stylizer::common_mesh_attributes::colors, std::move(colors));
				if(normals.size()) mesh.add_vertex_attribute(stylizer::common_mesh_attributes::normals, std::move(normals));
				if(uvs.size()) mesh.add_vertex_attribute(stylizer::common_mesh_attributes::uvs, std::move(uvs));
				return mesh;
			}
		};

		std::unordered_map<int, stylizer::flat_material*> material_map;

		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
			std::unordered_map<int, attributes> material_meshes;

			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
				auto mat = shapes[s].mesh.material_ids[f];

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					tinyobj::real_t vx = attrib.vertices[3*size_t(idx.vertex_index)+0];
					tinyobj::real_t vy = attrib.vertices[3*size_t(idx.vertex_index)+1];
					tinyobj::real_t vz = attrib.vertices[3*size_t(idx.vertex_index)+2];
					material_meshes[mat].positions.emplace_back(vx, vy, vz, 0);

					// Check if `normal_index` is zero or positive. negative = no normal data
					if (idx.normal_index >= 0) {
						tinyobj::real_t nx = attrib.normals[3*size_t(idx.normal_index)+0];
						tinyobj::real_t ny = attrib.normals[3*size_t(idx.normal_index)+1];
						tinyobj::real_t nz = attrib.normals[3*size_t(idx.normal_index)+2];
						material_meshes[mat].normals.emplace_back(nx, ny, nz, 0);
					}

					// Check if `texcoord_index` is zero or positive. negative = no texcoord data
					if (idx.texcoord_index >= 0) {
						tinyobj::real_t tx = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
						tinyobj::real_t ty = attrib.texcoords[2*size_t(idx.texcoord_index)+1];
						material_meshes[mat].uvs.emplace_back(tx, ty); // TODO: Why is the v axis inverted!?!?
					}

					// Optional: vertex colors
					tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
					tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
					tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
					material_meshes[mat].colors.emplace_back(red, green, blue, 1);
				}
				index_offset += fv;
			}

			for(auto& [mat, attrs]: material_meshes) {
				if(!material_map.contains(mat)) {
					stylizer::flat_material material;
					if(mat >= 0) material.color = stdmath::float4(materials[mat].diffuse[0], materials[mat].diffuse[1], materials[mat].diffuse[2], 1);
					else material.color = stdmath::float4(.5, .5, .5, 1);

					if(mat >= 0 && !materials[mat].diffuse_texname.empty()) {
						std::filesystem::path path = materials[mat].diffuse_texname;
						material.color = stylizer::image::load(ctx, path)->upload(ctx).configure_sampler(ctx).move_to_owned();
					}

					auto& real = out.emplace_back(attrs.make_mesh().move_to_owned(), material.move_to_owned());
					material_map[mat] = (stylizer::flat_material*)&*real.second;
					continue;
				}

				out.emplace_back(attrs.make_mesh().move_to_owned(), material_map[mat]);
			}
		}

		return out;
	}

	inline model load_tinyobj_model(stylizer::context& ctx, std::span<std::byte> memory, std::string_view extension = {}) {
		return load_tinyobj_model_with_material(ctx, memory, {});
	}

	inline maybe_owned<model> load_tinyobj_model_generic(stylizer::context& ctx, std::span<std::byte> memory, std::string_view extension = {}) {
		return load_tinyobj_model(ctx, memory).move_to_owned();
	}

	std::unordered_map<std::string, std::function<maybe_owned<model>(context&, std::span<std::byte>, std::string_view)>>& model::get_loader_set() {
		static std::unordered_map<std::string, std::function<maybe_owned<model>(context&, std::span<std::byte>, std::string_view)>> loaders = []{
			std::unordered_map<std::string, std::function<maybe_owned<model>(context&, std::span<std::byte>, std::string_view)>> out;
			out[".obj"] = load_tinyobj_model_generic;
			return out;
		}();
		return loaders;
	}
	maybe_owned<model> model::load(context& ctx, std::filesystem::path file) {
		return load_file(ctx, file, get_loader_set()[file.extension().string()]);
	}

}}
