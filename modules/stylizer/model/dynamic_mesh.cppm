module;

// #include <stylizer/core/util/reaction.hpp>
#include <stylizer/core/util/move_and_make_owned_macros.hpp>

export module stylizer.model:dynamic_mesh;

import std.compat;
// import stdmath;
// import stdmath.slang;
import stylizer;

import :mesh;

namespace stylizer { inline namespace models {

	export struct dynamic_mesh : public mesh { STYLIZER_MOVE_AND_MAKE_OWNED_DERIVED_METHODS(dynamic_mesh, mesh)
		std::unordered_map<std::string, size_t> names2index;
		std::vector<vertex_storage> attribute_data;
		std::vector<uint32_t> index_data = {};
		std::vector<meshlet> meshlets = {};
		std::optional<uint32_t> cached_vertex_count = {};

		std::optional<size_t> lookup_attribute(std::string_view name_) override {
			auto name = std::string{name_};
			if(!names2index.contains(name)) return {};
			return names2index.at(name);
		}

		std::span<std::string_view> available_attributes() override {
			static std::vector<std::string_view> out;
			out.clear();
			for(auto& [name, index] : names2index)
				out.push_back(name);
			return out;
		}

		std::span<size_t> attribute_indicies() override {
			static std::vector<size_t> out;
			out.resize(attribute_data.size());
			std::iota(out.begin(), out.end(), 0);
			return out;
		}

		vertex_storage& attribute_storage(size_t index) override {
			return attribute_data[index];
		}

		template<typename T>
		storage<T>& attribute_storage(size_t index) {
			return (storage<T>&)attribute_data[index].as_bytes();
		}

		size_t attribute_offset(size_t index) override {
			size_t total = 0;
			for(size_t i = 0; i < index; ++i)
				total += attribute_data[i].as_bytes().size();
			return total;
		}

		std::optional<std::span<uint32_t>> indicies_view() override {
			return index_data.size() ? std::optional<std::span<uint32_t>>(index_data) : std::nullopt;
		}

		std::optional<std::span<meshlet>> meshlets_view() override {
			if(meshlets.size()) return meshlets;
			if(index_data.size()) {
				meshlets = {meshlet{index_data}};
				return meshlets;
			}
			return {};
		}

		size_t vertex_count() override {
			if(cached_vertex_count) return *cached_vertex_count;
			return mesh::vertex_count();
		}

		template<typename T>
		storage<T>& add_vertex_attribute_storage(std::string_view name, storage<T>&& storage) {
				names2index[std::string(name)] = attribute_data.size();
				attribute_data.emplace_back(std::move(storage));
				return (stylizer::storage<T>&)attribute_data.back().as_bytes();
			}
			virtual vertex_storage& add_vertex_attribute(std::string_view name, vertex_storage&& storage) override {
			vertex_storage* out;
			std::visit([&](auto& s) {
				out = (vertex_storage*)&add_vertex_attribute_storage(name, std::move(s)); // This is so unsafe... expect to explode!
			}, static_cast<vertex_storage_variant&>(storage));
			return *out;
		}

		std::span<uint32_t> set_index_data(std::span<const uint32_t> indicies) override  {
			meshlets = {};
			return index_data = std::vector<uint32_t>(indicies.begin(), indicies.end());
		}

		mesh& clear_index_data() override {
			index_data.clear();
			return *this;
		}

		bool verify() override  {
			if(cached_vertex_count && *cached_vertex_count != mesh::vertex_count())
				return false;

			return mesh::verify();
		}
	};

}}
