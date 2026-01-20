#pragma once

#include "api.hpp"
#include <optional>

namespace stylizer { inline namespace models { 

	struct dynamic_mesh : public mesh { STYLIZER_MOVE_AND_MAKE_OWNED_DERIVED_METHODS(dynamic_mesh, mesh)
		std::unordered_map<std::string, size_t> names2index;
		std::vector<vertex_storage> attribute_data;
		std::vector<uint32_t> index_data = {};
		std::vector<meshlet> meshlets = {};
		std::optional<uint32_t> cached_vertex_count = {};

		std::optional<size_t> lookup_attribute(std::string_view name_) override;

		std::span<std::string_view> available_attributes() override;

		std::span<size_t> attribute_indicies() override;

		vertex_storage& attribute_storage(size_t index) override {
			return attribute_data[index];
		}

		template<typename T>
		storage<T>& attribute_storage(size_t index) {
			return (storage<T>&)attribute_data[index].as_bytes();
		}

		size_t attribute_offset(size_t index) override;

		std::optional<std::span<uint32_t>> indicies_view() override;
		std::optional<std::span<meshlet>> meshlets_view() override;

		size_t vertex_count() override;

		template<typename T>
		storage<T>& add_vertex_attribute_storage(std::string_view name, storage<T>&& storage) {
			names2index[std::string(name)] = attribute_data.size();
			attribute_data.emplace_back(std::move(storage));
			return (stylizer::storage<T>&)attribute_data.back().as_bytes();
		}
		virtual vertex_storage& add_vertex_attribute(std::string_view name, vertex_storage&& storage) override;

		std::span<uint32_t> set_index_data(std::span<const uint32_t> indicies) override;

		mesh& clear_index_data() override;

		bool verify() override;
	};

	model load_tinyobj_model_with_material(stylizer::context& ctx, std::span<std::byte> memory, std::span<std::byte> mtl);
	model load_tinyobj_model(stylizer::context& ctx, std::span<std::byte> memory, std::string_view extension = {});
}}