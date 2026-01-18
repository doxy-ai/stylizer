#pragma once

#include "type_erased_storage.hpp"

#include <stylizer/core/api.hpp>
#include <stylizer/core/flat_material.hpp>
#include <stylizer/core/util/load_file.hpp>
#include <stylizer/core/util/maybe_owned.hpp>

#include <unordered_map>

namespace stylizer { inline namespace models {

    /**
     * @brief Represents a 3D mesh consisting of vertices and optional indices.
     */
    struct mesh {
		/**
		 * @brief The type of primitives in the mesh.
		 */
		enum class Type {
			Triangle,
			Line,
			Point,
		} type = Type::Triangle;

		/**
		 * @brief Variant for different types of vertex storage.
		 */
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

		/**
		 * @brief Manages storage for a single vertex attribute.
		 */
		struct vertex_storage: public vertex_storage_variant {
			/** @brief Base type alias. */
			using super = vertex_storage_variant;
			/** @brief Inherit constructors from vertex_storage_variant. */
			using super::super;

			/**
			 * @brief Returns the storage as a byte storage.
			 * @return Reference to storage<std::byte>.
			 */
			storage<std::byte>& as_bytes() {
				return std::visit([](auto& s) -> storage<std::byte>& {
					return (storage<std::byte>&)s;
				}, *this);
			}

			/**
			 * @brief Gets the number of elements in the storage.
			 * @return The size.
			 */
			size_t size() {
				return std::visit([](auto& s) -> size_t {
					return s.size();
				}, *this);
			}

			/**
			 * @brief Gets a span of bytes for the underlying data.
			 * @return A span of bytes.
			 */
			std::span<std::byte> byte_span() {
				return std::visit([](auto& s) -> std::span<std::byte> {
					return s.byte_span();
				}, *this);
			}

			/** @brief Accesses storage as float. */
			storage<float>& get_float() { return std::get<storage<float>>(*this); }
			/** @brief Accesses storage as float2. */
			storage<stdmath::vector<float, 2>>& get_float2() { return std::get<storage<stdmath::vector<float, 2>>>(*this); }
			// storage<stdmath::vector<float, 3>>& get_vector3() { return std::get<storage<stdmath::vector<float, 3>>>(*this); }
			/** @brief Accesses storage as float4. */
			storage<stdmath::vector<float, 4>>& get_float4() { return std::get<storage<stdmath::vector<float, 4>>>(*this); }
			/** @brief Accesses storage as int32. */
			storage<int32_t>& get_int() { return std::get<storage<int32_t>>(*this); }
			/** @brief Accesses storage as int2. */
			storage<stdmath::vector<int32_t, 2>>& get_int2() { return std::get<storage<stdmath::vector<int32_t, 2>>>(*this); }
			// storage<stdmath::vector<int32_t, 3>>& get_int_vector3() { return std::get<storage<stdmath::vector<int32_t, 3>>>(*this); }
			/** @brief Accesses storage as int4. */
			storage<stdmath::vector<int32_t, 4>>& get_int4() { return std::get<storage<stdmath::vector<int32_t, 4>>>(*this); }
		};

		/**
		 * @brief Looks up the index of an attribute by name.
		 * @param name The name of the attribute.
		 * @return The attribute index if found, std::nullopt otherwise.
		 */
		virtual std::optional<size_t> lookup_attribute(std::string_view name) = 0;

		/**
		 * @brief Gets the names of all available attributes.
		 * @return A span of attribute names.
		 */
		virtual std::span<std::string_view> available_attributes() = 0;

		/**
		 * @brief Builds a list of attribute indices based on requested names.
		 * @param requested_attributes The names of the requested attributes.
		 * @return A vector of attribute indices.
		 */
		virtual std::vector<size_t> build_attribute_list(std::span<std::string_view> requested_attributes);

		/**
		 * @brief Gets the indices of all attributes.
		 * @return A span of attribute indices.
		 */
		virtual std::span<size_t> attribute_indicies() = 0;

		/**
		 * @brief Gets the storage for a specific attribute index.
		 * @param index The attribute index.
		 * @return Reference to the vertex storage.
		 */
		virtual vertex_storage& attribute_storage(size_t index) = 0;

		/**
		 * @brief Gets the bytes for a specific attribute index.
		 * @param index The attribute index.
		 * @return A span of bytes.
		 */
		virtual std::span<std::byte> attribute_bytes(size_t index) {
			return attribute_storage(index).byte_span();
		}

		/**
		 * @brief Gets the offset of an attribute in a packed vertex.
		 * @param index The attribute index.
		 * @return The offset in bytes.
		 */
		virtual size_t attribute_offset(size_t index);

		/**
		 * @brief Gets a view of the indices.
		 * @return A span of uint32 indices if available.
		 */
		virtual std::optional<std::span<uint32_t>> indicies_view() = 0;

		/**
		 * @brief Represents a meshlet, a small portion of a mesh.
		 */
		struct meshlet {
			/** @brief Indices belonging to this meshlet. */
			std::span<uint32_t> indicies;
		};

		/**
		 * @brief Gets a view of the meshlets.
		 * @return A span of meshlets if available.
		 */
		virtual std::optional<std::span<meshlet>> meshlets_view() = 0;

		/** @brief The cached index buffer on the GPU. */
		api::current_backend::buffer index_buffer = {};

		/**
		 * @brief Gets the GPU index buffer, rebuilding it if necessary.
		 * @param ctx The stylizer context.
		 * @param rebuild Whether to force a rebuild.
		 * @return Pointer to the GPU index buffer.
		 */
		virtual api::current_backend::buffer* get_index_buffer(context& ctx, bool rebuild = false);

		/** @brief Cache for GPU vertex buffers. */
		std::unordered_map<size_t, std::vector<api::current_backend::buffer>> vertex_buffer_cache;

		/**
		 * @brief Gets the GPU vertex buffers for specific attributes.
		 * @param ctx The stylizer context.
		 * @param attribute_indicies The indices of requested attributes.
		 * @param rebuild Whether to force a rebuild.
		 * @return A span of GPU vertex buffers.
		 */
		virtual std::span<api::current_backend::buffer> get_vertex_buffers(context& ctx, std::span<const size_t> attribute_indicies, bool rebuild = false);

		/**
		 * @brief Rebuilds all GPU-side caches.
		 * @param ctx The stylizer context.
		 */
		virtual void rebuild_gpu_caches(context& ctx) {
			get_index_buffer(ctx, true);
			vertex_buffer_cache.clear();
		}

		/**
		 * @brief Gets the total number of indices.
		 * @return The index count.
		 */
		virtual size_t index_count() {
			if(auto idx = indicies_view(); idx)
				return idx->size();
			return 0;
		}

		/**
		 * @brief Gets the total number of vertices.
		 * @return The vertex count.
		 */
		virtual size_t vertex_count();

		/**
		 * @brief Adds a new vertex attribute to the mesh.
		 * @param name The name of the attribute.
		 * @param storage The storage for the attribute.
		 * @return Reference to the added vertex storage.
		 */
		virtual vertex_storage& add_vertex_attribute(std::string_view name, vertex_storage&& storage) = 0;

		/**
		 * @brief Sets the index data for the mesh.
		 * @param indicies The indices to set.
		 * @return A span of the set indices.
		 */
		virtual std::span<uint32_t> set_index_data(std::span<const uint32_t> indicies) = 0;

		/**
		 * @brief Clears all index data.
		 * @return Reference to this mesh.
		 */
		virtual mesh& clear_index_data() { return *this; }

		/**
		 * @brief Verifies that the mesh data is valid.
		 * @return True if valid.
		 */
		virtual bool verify();
	};

    /**
     * @brief Represents a 3D model, which is a collection of mesh-material pairs.
     *
     * @code
     * auto mdl = stylizer::model::load(ctx, "path/to/model.obj");
     * if (mdl) {
     *     mdl->draw(ctx, render_pass);
     * }
     * @endcode
     */
    struct model : public std::vector<std::pair<maybe_owned<mesh>, maybe_owned<material>>> {

		/**
		 * @brief Gets the set of registered model loaders.
		 * @return Reference to the loader set map.
		 */
		static std::unordered_map<std::string, std::function<maybe_owned<model>(context&, std::span<std::byte>, std::string_view)>>& get_loader_set();

		/**
		 * @brief Loads a model from a file.
		 * @param ctx The stylizer context.
		 * @param file The path to the model file.
		 * @return A maybe_owned pointer to the loaded model.
		 */
		static maybe_owned<model> load(context& ctx, std::filesystem::path file);

		/**
		 * @brief Overrides all materials in the model with a single material.
		 * @param override_material The material to use.
		 * @return Reference to this model.
		 */
		model& override_materials(material& override_material);

		/**
		 * @brief Uploads all meshes in the model to the GPU.
		 * @param ctx The stylizer context.
		 * @param fb The frame buffer (used to determine material compatibility).
		 * @return Reference to this model.
		 */
		model& upload(context& ctx, const frame_buffer& fb);

		/**
		 * @brief Draws the model multiple times using instancing.
		 * @param ctx The stylizer context.
		 * @param render_pass The render pass to record draw commands into.
		 * @param instance_data The buffer containing instance data.
		 * @param util Optional utility buffer.
		 * @return Reference to the render pass.
		 */
		api::current_backend::render::pass& draw_instanced(
			context& ctx, api::current_backend::render::pass& render_pass,
			instance_data::buffer_base& instance_data, std::optional<utility_buffer> util = {}
		);

		/** @brief Internal cache for instance data when drawing a single instance. */
		instance_data::buffer<1> instance_data_cache;

		/**
		 * @brief Draws the model once.
		 * @param ctx The stylizer context.
		 * @param render_pass The render pass to record draw commands into.
		 * @param instance_data The instance data for this single draw call.
		 * @param util Optional utility buffer.
		 * @return Reference to the render pass.
		 */
		api::current_backend::render::pass& draw(context& ctx, api::current_backend::render::pass& render_pass, const instance_data& instance_data = {}, std::optional<utility_buffer> util = {});
    };

	/**
	 * @brief Generic loader using tinyobjloader.
	 * @param ctx The stylizer context.
	 * @param memory The memory buffer containing the model data.
	 * @param extension The file extension/format.
	 * @return A maybe_owned model.
	 */
	maybe_owned<model> load_tinyobj_model_generic(stylizer::context& ctx, std::span<std::byte> memory, std::string_view extension);
}}
