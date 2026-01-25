module;

// #include <stylizer/core/util/reaction.hpp>
#include <stylizer/core/util/move_and_make_owned_macros.hpp>

export module stylizer.model:mesh;

import std.compat;
// import stdmath;
// import stdmath.slang;
import stylizer;

import :storage;

namespace stylizer { inline namespace models {

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

	export struct mesh { STYLIZER_MOVE_AND_MAKE_OWNED_BASE_METHODS(mesh)
		enum class Type {
			Triangle,
			Line,
			Point,
		} type = Type::Triangle;

		using vertex_storage_variant = std::variant<
			storage<std::byte>,
			storage<float>,
			storage<stdmath::float2>,
			// storage<stdmath::float3>, // NOTE: Vec3s are stored as vec4s with w = 0
			storage<stdmath::float4>,
			storage<int32_t>,
			storage<stdmath::int2>,
			// storage<stdmath::int3>, // NOTE: Vec3s are stored as vec4s with w = 0
			storage<stdmath::int4>
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
			storage<stdmath::float2>& get_float2() { return std::get<storage<stdmath::float2>>(*this); }
			// storage<stdmath::float3>& get_vector3() { return std::get<storage<stdmath::float3>>(*this); }
			storage<stdmath::float4>& get_float4() { return std::get<storage<stdmath::float4>>(*this); }
			storage<int32_t>& get_int() { return std::get<storage<int32_t>>(*this); }
			storage<stdmath::int2>& get_int2() { return std::get<storage<stdmath::int2>>(*this); }
			// storage<stdmath::int3>& get_int_vector3() { return std::get<storage<stdmath::int3>>(*this); }
			storage<stdmath::int4>& get_int4() { return std::get<storage<stdmath::int4>>(*this); }
		};

		virtual ~mesh() {}

		virtual std::optional<size_t> lookup_attribute(std::string_view name) = 0;
		virtual std::span<std::string_view> available_attributes() = 0;
		virtual std::vector<size_t> build_attribute_list(std::span<std::string_view> requested_attributes) {
			std::vector<size_t> out;
			for(auto attribute: requested_attributes) {
				auto index = lookup_attribute(attribute);
				if(!index) return {};
				out.push_back(*index);
			}
			return out;
		}
		virtual std::span<size_t> attribute_indicies() = 0;
		virtual vertex_storage& attribute_storage(size_t index) = 0;
		virtual std::span<std::byte> attribute_bytes(size_t index) {
			return attribute_storage(index).byte_span();
		}

		virtual size_t attribute_offset(size_t index) {
			size_t total = 0;
			for(auto index : attribute_indicies())
				total += attribute_bytes(index).size();
			return total;
		}

		virtual std::optional<std::span<uint32_t>> indicies_view() = 0;
		struct meshlet {
			std::span<uint32_t> indicies;
		};
		virtual std::optional<std::span<meshlet>> meshlets_view() = 0;

		graphics::current_backend::buffer index_buffer = {};
		virtual graphics::current_backend::buffer* get_index_buffer(context& ctx, bool rebuild = false) {
			if(!indicies_view())
				return nullptr;

			if(!index_buffer || rebuild) {
				index_buffer.release();
				index_buffer = ctx.create_and_write_buffer(graphics::usage::Index, byte_span<uint32_t>(*indicies_view()), 0, "Stylizer Mesh Index Buffer");
			}
			return &index_buffer;
		}

		std::unordered_map<size_t, std::vector<graphics::current_backend::buffer>> vertex_buffer_cache;
		virtual std::span<graphics::current_backend::buffer> get_vertex_buffers(context& ctx, std::span<const size_t> attribute_indicies, bool rebuild = false) {
			size_t hash = vertex_buffer_cache_helper{}(attribute_indicies);
			if(vertex_buffer_cache.contains(hash))
				return vertex_buffer_cache[hash];

			std::vector<graphics::current_backend::buffer> buffers;
			for(size_t index : attribute_indicies) {
				auto data = attribute_bytes(index);
				buffers.emplace_back(ctx.create_and_write_buffer(graphics::usage::Vertex, data, 0, "Stylizer Mesh Vertex Buffer"));
			}

			return vertex_buffer_cache[hash] = std::move(buffers);
		}

		virtual void rebuild_gpu_caches(context& ctx) {
			get_index_buffer(ctx, true);
			vertex_buffer_cache.clear();
		}

		virtual size_t index_count() {
			if(auto idx = indicies_view(); idx)
				return idx->size();
			return 0;
		}
		virtual size_t vertex_count() {
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

		virtual vertex_storage& add_vertex_attribute(std::string_view name, vertex_storage&& storage) = 0;
		virtual std::span<uint32_t> set_index_data(std::span<const uint32_t> indicies) = 0;
		virtual mesh& clear_index_data() { return *this; }

		virtual bool verify() {
			auto indicies = attribute_indicies();
			auto vertex_count = this->vertex_count();
			for(auto idx: indicies)
				if(attribute_storage(idx).as_bytes().size() != vertex_count)
					return false;
			return true;
		}
	};

}}
