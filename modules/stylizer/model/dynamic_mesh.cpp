#include "dynamic_mesh.hpp"

namespace stylizer {

	std::optional<size_t> dynamic_mesh::lookup_attribute(std::string_view name_) {
		auto name = std::string{name_};
		if(!names2index.contains(name)) return {};
		return names2index.at(name);
	}

	std::span<std::string_view> dynamic_mesh::available_attributes() {
		static std::vector<std::string_view> out;
		out.clear();
		for(auto& [name, index] : names2index)
			out.push_back(name);
		return out;
	}

	std::span<size_t> dynamic_mesh::attribute_indicies() {
		static std::vector<size_t> out;
		out.resize(attribute_data.size());
		std::iota(out.begin(), out.end(), 0);
		return out;
	}

	size_t dynamic_mesh::attribute_offset(size_t index) {
		size_t total = 0;
		for(size_t i = 0; i < index; ++i)
			total += attribute_data[i].as_bytes().size();
		return total;
	}

	std::optional<std::span<uint32_t>> dynamic_mesh::indicies_view() {
		return index_data.size() ? std::optional<std::span<uint32_t>>(index_data) : std::nullopt;
	}
	std::optional<std::span<mesh::meshlet>> dynamic_mesh::meshlets_view() {
		if(meshlets.size()) return meshlets;
		if(index_data.size()) {
			meshlets = {meshlet{index_data}};
			return meshlets;
		}
		return {};
	}

	size_t dynamic_mesh::vertex_count() {
		if(cached_vertex_count) return *cached_vertex_count;
		return mesh::vertex_count();
	}

	mesh::vertex_storage& dynamic_mesh::add_vertex_attribute(std::string_view name, mesh::vertex_storage&& storage) {
		vertex_storage* out;
		std::visit([&](auto& s) {
			out = (vertex_storage*)&add_vertex_attribute_storage(name, std::move(s)); // This is so unsafe... expect to explode!
		}, static_cast<vertex_storage_variant&>(storage));
		return *out;
	}

	std::span<uint32_t> dynamic_mesh::set_index_data(std::span<const uint32_t> indicies) {
		meshlets = {};
		return index_data = std::vector<uint32_t>(indicies.begin(), indicies.end());
	}

	mesh& dynamic_mesh::clear_index_data() {
		index_data.clear();
		return *this;
	}

	bool dynamic_mesh::verify() {
		if(cached_vertex_count && *cached_vertex_count != mesh::vertex_count())
			return false;

		return mesh::verify();
	}
}