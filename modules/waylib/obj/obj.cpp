#include "obj.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_USE_MAPBOX_EARCUT
#include "thirdparty/tiny_obj_loader.h"

WAYLIB_BEGIN_NAMESPACE

	result<model> obj::load(const std::filesystem::path& file_path) {
		auto path = std::filesystem::absolute(file_path);
		tinyobj::ObjReaderConfig reader_config;
		reader_config.triangulate = true;
		reader_config.vertex_color = true;
		reader_config.mtl_search_path = path.parent_path().string(); // Path to material files

		tinyobj::ObjReader reader;
		if (!reader.ParseFromFile(path.string(), reader_config)) {
			if (!reader.Error().empty())
				return unexpected(reader.Error());
			return {};
		}

		if (!reader.Warning().empty())
			errors::set("[TinyObjReader][Warning] " + reader.Warning());

		auto& attrib = reader.GetAttrib();
		auto& shapes = reader.GetShapes();
		auto& materials = reader.GetMaterials();

		modelC out;
		out.transform = glm::identity<glm::mat4x4>();
		out.mesh_count = shapes.size();
		out.meshes = {true, new mesh[shapes.size()]};
		// TODO: Materials

		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces(polygon)
			size_t index_offset = 0;
			out.meshes[s] = {}; // Default initialize the mesh
			out.meshes[s].vertex_count = shapes[s].mesh.num_face_vertices.size() * 3;
			out.meshes[s].triangle_count = out.meshes[s].vertex_count / 3;
			auto& dbg = out.meshes[s];
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
					if(!out.meshes[s].positions.value)
						out.meshes[s].positions = {true, new vec4f[out.meshes[s].vertex_count]};
					out.meshes[s].positions[3 * f + v] = vec4f(vx, vy, vz, 1);

					// Check if `normal_index` is zero or positive. negative = no normal data
					if (idx.normal_index >= 0) {
						tinyobj::real_t nx = attrib.normals[3*size_t(idx.normal_index)+0];
						tinyobj::real_t ny = attrib.normals[3*size_t(idx.normal_index)+1];
						tinyobj::real_t nz = attrib.normals[3*size_t(idx.normal_index)+2];
						if(!out.meshes[s].normals.value)
							out.meshes[s].normals = {true, new vec4f[out.meshes[s].vertex_count]};
						out.meshes[s].normals[3 * f + v] = vec4f(nx, ny, nz, 0);
					}

					// Check if `texcoord_index` is zero or positive. negative = no texcoord data
					if (idx.texcoord_index >= 0) {
						tinyobj::real_t tx = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
						tinyobj::real_t ty = attrib.texcoords[2*size_t(idx.texcoord_index)+1];
						if(!out.meshes[s].uvs.value)
							out.meshes[s].uvs = {true, new vec4f[out.meshes[s].vertex_count]};
						out.meshes[s].uvs[3 * f + v] = vec4f(tx, ty, std::nan(""), std::nan(""));
					}

					// Optional: vertex colors // TODO: How do we detect that there aren't colors?
					tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
					tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
					tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
					if(!out.meshes[s].colors.value)
						out.meshes[s].colors = {true, new colorC[out.meshes[s].vertex_count]};
					out.meshes[s].colors[3 * f + v] = colorC(red, green, blue, 1);
				}
				index_offset += fv;
			}
		}

		out.material_count = 0;
		out.materials = nullptr;
		out.mesh_materials = nullptr;
		return out;
	}

WAYLIB_END_NAMESPACE