#include "api.hpp"

namespace stylizer { inline namespace models {
	std::vector<size_t> mesh::build_attribute_list(std::span<std::string_view> requested_attributes) {
		std::vector<size_t> out;
		for(auto attribute: requested_attributes) {
			auto index = lookup_attribute(attribute);
			if(!index) return {};
			out.push_back(*index);
		}
		return out;
	}

	size_t mesh::attribute_offset(size_t index) {
		size_t total = 0;
		for(auto index : attribute_indicies())
			total += attribute_bytes(index).size();
		return total;
	}

	api::current_backend::buffer* mesh::get_index_buffer(context& ctx, bool rebuild /* = false */) {
		if(!indicies_view())
			return nullptr;

		if(!index_buffer || rebuild) {
			index_buffer.release();
			index_buffer = ctx.create_and_write_buffer(api::usage::Index, byte_span<uint32_t>(*indicies_view()), 0, "Stylizer Mesh Index Buffer");
		}
		return &index_buffer;
	}

	struct vertex_buffer_cache_helper {
		std::uint64_t operator()(const std::span<const size_t> indicies) const {
			std::uint64_t hash = std::hash<size_t>{}(indicies.front());
			for(size_t i = 1; i < indicies.size(); ++i)
				hash ^= std::hash<size_t>{}(indicies[i]);
			return hash;
		}
		bool operator()(const std::span<const size_t> a, const std::span<const size_t> b) const {
			return operator()(a) == operator()(b);
		}
	};
	std::span<api::current_backend::buffer> mesh::get_vertex_buffers(context& ctx, std::span<const size_t> attribute_indicies, bool rebuild /* = false */) {
		size_t hash = vertex_buffer_cache_helper{}(attribute_indicies);
		if(vertex_buffer_cache.contains(hash))
			return vertex_buffer_cache[hash];

		std::vector<api::current_backend::buffer> buffers;
		for(size_t index : attribute_indicies) {
			auto data = attribute_bytes(index);
			buffers.emplace_back(ctx.create_and_write_buffer(api::usage::Vertex, data, 0, "Stylizer Mesh Vertex Buffer"));
		}

		return vertex_buffer_cache[hash] = std::move(buffers);
	}

	size_t mesh::vertex_count() {
		uint32_t max = 0;
		if(auto meshlets = meshlets_view(); meshlets) {
			for(auto meshlet: *meshlets)
				max = std::max(max, *std::max_element(meshlet.indicies.begin(), meshlet.indicies.end()));
		} else if(indicies_view())
			max = std::max(max, *std::max_element(indicies_view()->begin(), indicies_view()->end()));
		else
			max = attribute_storage(0).size();
		return max;
	}

	bool mesh::verify() {
		auto indicies = attribute_indicies();
		auto vertex_count = this->vertex_count();
		for(auto idx: indicies)
			if(attribute_storage(idx).as_bytes().size() != vertex_count)
				return false;
		return true;
	}
}}