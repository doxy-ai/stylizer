#include "obj.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "thirdparty/tiny_obj_loader.h"

namespace stylizer::obj {

	core::model load(const std::filesystem::path& file_path, context& ctx /* = context::null */) {
		auto path = std::filesystem::absolute(file_path);
		tinyobj::ObjReaderConfig reader_config;
		reader_config.triangulate = true;
		reader_config.vertex_color = true;
		reader_config.mtl_search_path = path.parent_path().string(); // Path to material files

		tinyobj::ObjReader reader;
		if (!reader.ParseFromFile(path.string(), reader_config)) {
			if (!reader.Error().empty())
				STYLIZER_THROW(reader.Error());
			STYLIZER_THROW("An unknown error occurred while loading " + file_path.string());
		}

		if (!reader.Warning().empty())
			std::cerr << ("[TinyObjReader][Warning] " + reader.Warning()) << std::endl;

		auto& attrib = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		auto& materials = reader.GetMaterials();

		std::vector<core::mesh> meshes(shapes.size());
		// TODO: Materials

		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces(polygon)
			size_t index_offset = 0;
			meshes[s] = {}; // Default initialize the mesh
			meshes[s].vertex_count = shapes[s].mesh.num_face_vertices.size() * 3;
			meshes[s].triangle_count = meshes[s].vertex_count / 3;
			auto& dbg = meshes[s];
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
				assert(fv == 3); // We only support triangles (passing triangulate to the reader means this check should never fail...)

				// per-face material
				// shapes[s].mesh.material_ids[f];
				// TODO: Different material faces should go into different meshes

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					tinyobj::real_t vx = attrib.vertices[3*size_t(idx.vertex_index)+0];
					tinyobj::real_t vy = attrib.vertices[3*size_t(idx.vertex_index)+1];
					tinyobj::real_t vz = attrib.vertices[3*size_t(idx.vertex_index)+2];
					if(meshes[s].positions.empty())
						meshes[s].positions.resize(meshes[s].vertex_count);
					meshes[s].positions[3 * f + v] = {vx, vy, vz, 1};

					// Check if `normal_index` is zero or positive. negative = no normal data
					if (idx.normal_index >= 0) {
						tinyobj::real_t nx = attrib.normals[3*size_t(idx.normal_index)+0];
						tinyobj::real_t ny = attrib.normals[3*size_t(idx.normal_index)+1];
						tinyobj::real_t nz = attrib.normals[3*size_t(idx.normal_index)+2];
						if(!meshes[s].normals || meshes[s].normals->empty())
							meshes[s].normals = std::vector<float4>(meshes[s].vertex_count);
						(*meshes[s].normals)[3 * f + v] = {nx, ny, nz, 0};
					}

					// Check if `texcoord_index` is zero or positive. negative = no texcoord data
					if (idx.texcoord_index >= 0) {
						tinyobj::real_t tx = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
						tinyobj::real_t ty = attrib.texcoords[2*size_t(idx.texcoord_index)+1];
						if(!meshes[s].uvs || meshes[s].uvs->empty())
							meshes[s].uvs = std::vector<float4>(meshes[s].vertex_count);
						(*meshes[s].uvs)[3 * f + v] = {tx, ty, std::nan(""), std::nan("")};
					}

					// Optional: vertex colors // TODO: How do we detect that there aren't colors?
					tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
					tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
					tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
					if(!meshes[s].colors || meshes[s].colors->empty())
							meshes[s].colors = std::vector<float4>(meshes[s].vertex_count);
					(*meshes[s].colors)[3 * f + v] = {red, green, blue, 1};
				}
				index_offset += fv;
			}
		}

		core::model out;
		out.transform = identity_matrix;
		for(auto& mesh: meshes)
			out.material_mapped_meshes.emplace(std::move(ctx ? mesh.upload(ctx) : mesh), nullptr);
		return out;
	}

} // namespace stylizer::obj