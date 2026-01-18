#pragma once

#include "type_erased_storage.hpp"

#include <stylizer/core/api.hpp>
#include <stylizer/core/flat_material.hpp>
#include <stylizer/core/util/load_file.hpp>
#include <stylizer/core/util/maybe_owned.hpp>

#include <unordered_map>

namespace stylizer { inline namespace models {

    struct mesh {
		enum class Type {
			Triangle,
			Line,
			Point,
		} type = Type::Triangle;

		using vertex_storage_variant = std::variant<
			storage<std::byte>,
			storage<float>,
			storage<stdmath::vector<float, 2>>,
			// storage<stdmath::vector<float, 3>>, // NOTE: Vec3s are stored as vec4s with w = 0
			storage<stdmath::vector<float, 4>>,
			storage<int32_t>,
			storage<stdmath::vector<int32_t, 2>>,
			// storage<stdmath::vector<int32_t, 3>>, // NOTE: Vec3s are stored as vec4s with w = 0
			storage<stdmath::vector<int32_t, 4>>
		>;
		struct vertex_storage: public vertex_storage_variant {
			using super = vertex_storage_variant;
			using super::super;

			storage<std::byte>& as_bytes() {
				return std::visit([](auto& s) -> storage<std::byte>& {
					return (storage<std::byte>&)s;
				}, *this);
			}

			size_t size() {
				return std::visit([](auto& s) -> size_t {
					return s.size();
				}, *this);
			}

			std::span<std::byte> byte_span() {
				return std::visit([](auto& s) -> std::span<std::byte> {
					return s.byte_span();
				}, *this);
			}

			storage<float>& get_float() { return std::get<storage<float>>(*this); }
			storage<stdmath::vector<float, 2>>& get_float2() { return std::get<storage<stdmath::vector<float, 2>>>(*this); }
			// storage<stdmath::vector<float, 3>>& get_vector3() { return std::get<storage<stdmath::vector<float, 3>>>(*this); }
			storage<stdmath::vector<float, 4>>& get_float4() { return std::get<storage<stdmath::vector<float, 4>>>(*this); }
			storage<int32_t>& get_int() { return std::get<storage<int32_t>>(*this); }
			storage<stdmath::vector<int32_t, 2>>& get_int2() { return std::get<storage<stdmath::vector<int32_t, 2>>>(*this); }
			// storage<stdmath::vector<int32_t, 3>>& get_int_vector3() { return std::get<storage<stdmath::vector<int32_t, 3>>>(*this); }
			storage<stdmath::vector<int32_t, 4>>& get_int4() { return std::get<storage<stdmath::vector<int32_t, 4>>>(*this); }
		};

		virtual std::optional<size_t> lookup_attribute(std::string_view name) = 0;
		virtual std::span<std::string_view> available_attributes() = 0;
		virtual std::vector<size_t> build_attribute_list(std::span<std::string_view> requested_attributes);
		virtual std::span<size_t> attribute_indicies() = 0;
		virtual vertex_storage& attribute_storage(size_t index) = 0;
		virtual std::span<std::byte> attribute_bytes(size_t index) {
			return attribute_storage(index).byte_span();
		}

		virtual size_t attribute_offset(size_t index);

		virtual std::optional<std::span<uint32_t>> indicies_view() = 0;
		struct meshlet {
			std::span<uint32_t> indicies;
		};
		virtual std::optional<std::span<meshlet>> meshlets_view() = 0;

		api::current_backend::buffer index_buffer = {};
        virtual api::current_backend::buffer* get_index_buffer(context& ctx, bool rebuild = false);

		std::unordered_map<size_t, std::vector<api::current_backend::buffer>> vertex_buffer_cache;
		virtual std::span<api::current_backend::buffer> get_vertex_buffers(context& ctx, std::span<const size_t> attribute_indicies, bool rebuild = false);

		virtual void rebuild_gpu_caches(context& ctx) {
			get_index_buffer(ctx, true);
			vertex_buffer_cache.clear();
		}

		virtual size_t index_count() {
			if(auto idx = indicies_view(); idx)
				return idx->size();
			return 0;
		}
		virtual size_t vertex_count();

		virtual vertex_storage& add_vertex_attribute(std::string_view name, vertex_storage&& storage) = 0;
		virtual std::span<uint32_t> set_index_data(std::span<const uint32_t> indicies) = 0;
		virtual mesh& clear_index_data() { return *this; }

		virtual bool verify();
	};

    struct model : public std::vector<std::pair<maybe_owned<mesh>, maybe_owned<material>>> {

		static std::unordered_map<std::string, std::function<maybe_owned<model>(context&, std::span<std::byte>, std::string_view)>>& get_loader_set();
		static maybe_owned<model> load(context& ctx, std::filesystem::path file);

		model& override_materials(material& override_material);
		model& upload(context& ctx, const frame_buffer& fb);

		api::current_backend::render::pass& draw_instanced(
			context& ctx, api::current_backend::render::pass& render_pass,
			instance_data::buffer_base& instance_data, std::optional<utility_buffer> util = {}
		);

		instance_data::buffer<1> instance_data_cache;
		api::current_backend::render::pass& draw(context& ctx, api::current_backend::render::pass& render_pass, const instance_data& instance_data = {}, std::optional<utility_buffer> util = {});
    };

	maybe_owned<model> load_tinyobj_model_generic(stylizer::context& ctx, std::span<std::byte> memory, std::string_view extension);
}}