#pragma once

#include "config.hpp"
#include "slang_types.hpp"
#include "optional.h"
#include "managable.h"

#include "stylizer/api/api.hpp"
#include "thirdparty/thread_pool.hpp"
#include <concepts>
#include <cstring>

namespace stylizer {

#ifndef STYLIZER_NO_EXCEPTIONS
	struct error: public std::runtime_error {
		using std::runtime_error::runtime_error;
	};

	#define STYLIZER_THROW(x) throw error(x)
#else
	#define STYLIZER_THROW(x) assert((x, false))
#endif


//////////////////////////////////////////////////////////////////////
// # Thread Pool
//////////////////////////////////////////////////////////////////////


	struct STYLIZER_PREFIXED(thread_pool_future) : std::future<void> {};

	struct thread_pool {
	protected:
		static ZenSepiol::ThreadPool& get_thread_pool(optional<size_t> initial_pool_size = {})
#ifdef IS_STYLIZER_CORE_CPP
		{
			static ZenSepiol::ThreadPool pool(initial_pool_size ? *initial_pool_size : std::thread::hardware_concurrency() - 1);
			return pool;
		}
#else
		;
#endif

	public:
		template <typename F, typename... Args>
		static auto enqueue(F&& function, optional<size_t> initial_pool_size = {}, Args&&... args) {
			return get_thread_pool(initial_pool_size).AddTask(function, args...);
		}
	};


//////////////////////////////////////////////////////////////////////
// # Drawing State
//////////////////////////////////////////////////////////////////////


	struct drawing_state: public STYLIZER_API_NAMESPACE::render_pass {
		using super = STYLIZER_API_NAMESPACE::render_pass;

		struct context* context;
		STYLIZER_NULLABLE(struct geometry_buffer*) gbuffer = nullptr;
		operator stylizer::api::device&();

		const api::buffer& get_zero_buffer_singleton(api::usage usage = api::usage::Storage, size_t minimum_size = 0);

		STYLIZER_API_TYPE(command_buffer) end();
		void one_shot_submit();
	};


//////////////////////////////////////////////////////////////////////
// # Texture
//////////////////////////////////////////////////////////////////////


	struct texture: public STYLIZER_API_NAMESPACE::texture {
		using super = STYLIZER_API_NAMESPACE::texture;

		drawing_state begin_drawing(context& ctx, float4 clear_color, bool one_shot = true);
		drawing_state begin_drawing(context& ctx, optional<float3> clear_color = {}, bool one_shot = true) {
			return begin_drawing(ctx, clear_color ? float4(*clear_color, 1) : float4{0, 0, 0, 1}, one_shot);
		}
	};


//////////////////////////////////////////////////////////////////////
// # Context
//////////////////////////////////////////////////////////////////////


	struct context {
		static context null;

		STYLIZER_API_TYPE(device) device;
		STYLIZER_API_TYPE(surface) surface;
		operator bool() { return device || surface; }
		operator stylizer::api::device&() { // Automatically convert to an API device!
			assert(device);
			return device;
		}

#ifndef STYLIZER_USE_ABSTRACT_API
		static context create_default(stylizer::api::device::create_config config = {}, optional<stylizer::api::current_backend::surface&> surface = {}) {
			config.compatible_surface = surface ? &surface.value : nullptr;
			context out = {};
			out.device = stylizer::api::current_backend::device::create_default(config);
			if(surface) out.surface = std::move(*surface);
			return out;
		}
#endif

		void process_events() { device.process_events(); }
		void present() { surface.present(device); }

		texture get_surface_texture() {
			auto tmp = surface.next_texture(device);
			return std::move((texture&)tmp);
		}
		drawing_state begin_drawing_to_surface(float4 clear_color, bool one_shot = true) {
			return get_surface_texture().begin_drawing(*this, clear_color, one_shot);
		}
		drawing_state begin_drawing_to_surface(optional<float3> clear_color = {}, bool one_shot = true) {
			return begin_drawing_to_surface(clear_color ? float4(*clear_color, 1) : float4{0, 0, 0, 1}, one_shot);
		}

		void release(bool static_sub_objects = false) {
			device.release(static_sub_objects);
			surface.release();
		}
	};

	inline drawing_state::operator stylizer::api::device&() {
		assert(context);
		return context->device;
	}
	inline const api::buffer& drawing_state::get_zero_buffer_singleton(api::usage usage /* = api::usage::Storage */, size_t minimum_size /* = 0 */) {
		assert(context);
		STYLIZER_API_TYPE(buffer) released = {};
		auto& out = context->device.get_zero_buffer_singleton(usage, minimum_size, &released);
		if(released) defer([released = std::move(released)]() mutable {
			released.release();
		});
		return out;
	}
	inline STYLIZER_API_TYPE(command_buffer) drawing_state::end() {
		assert(context);
		return super::end(context->device);
	}
	inline void drawing_state::one_shot_submit() {
		assert(context);
		super::one_shot_submit(context->device);
	}

	inline drawing_state texture::begin_drawing(context& ctx, float4 clear_color, bool one_shot /* = true */) {
		drawing_state out;
		(STYLIZER_API_NAMESPACE::render_pass&) out = ctx.device.create_render_pass(std::array<api::render_pass::color_attachment, 1>{api::render_pass::color_attachment{
			.texture = this, .clear_value = api::convert(clear_color)
		}}, {}, one_shot);
		out.context = &ctx;
		return out;
	}


//////////////////////////////////////////////////////////////////////
// # Geometry Buffer
//////////////////////////////////////////////////////////////////////


	struct geometry_buffer_create_config {
		STYLIZER_NULLABLE(struct geometry_buffer*) previous = nullptr;
		texture::format color_format = texture::format::BGRA8_SRGB;
		texture::format depth_format = texture::format::Depth24;
	};

	struct geometry_buffer {
		using create_config = geometry_buffer_create_config;

		create_config config;
		STYLIZER_API_TYPE(texture) color, depth;
		operator bool() { return color || depth; }

		static geometry_buffer create_default(context& ctx, uint2 size, create_config config = {}) {
			using namespace api::operators;

			geometry_buffer out;
			out.config = config;
			out.color = ctx.device.create_texture({
				.label = "Stylizer Gbuffer Color Texture",
				.format = config.color_format,
				.usage = api::usage::RenderAttachment | api::usage::TextureBinding,
				.size = api::convert(uint3(size, 1))
			});
			out.color.configure_sampler(ctx);
			out.depth = ctx.device.create_texture({
				.label = "Stylizer Gbuffer Depth Texture",
				.format = config.depth_format,
				.usage = api::usage::RenderAttachment | api::usage::TextureBinding,
				.size = api::convert(uint3(size, 1))
			});
			return out;
		}

		virtual geometry_buffer& resize(context& ctx, uint2 size) {
			auto new_ = create_default(ctx, size, config);
			release();
			return *this = std::move(new_);
		}

		virtual std::span<api::render_pass::color_attachment> color_attachments() {
			static std::array<api::render_pass::color_attachment, 1> out;
			return out = {api::render_pass::color_attachment{ .texture = &color }};
		}
		virtual std::optional<api::render_pass::depth_stencil_attachment> depth_attachment() {
			return {api::render_pass::depth_stencil_attachment{
				.texture = &depth,
			}};
		}

		drawing_state begin_drawing(context& ctx, float4 clear_color, optional<float> clear_depth = {}, bool one_shot = true) {
			auto color_attachments = this->color_attachments();
			color_attachments[0].clear_value = api::convert(clear_color);
			auto depth_attachment = this->depth_attachment();
			depth_attachment->depth_clear_value = clear_depth ? *clear_depth : 1;

			drawing_state out;			
			(STYLIZER_API_NAMESPACE::render_pass&)out = ctx.device.create_render_pass(color_attachments, depth_attachment, one_shot);
			out.context = &ctx;
			out.gbuffer = this;
			return out;
		}
		drawing_state begin_drawing(context& ctx, optional<float3> clear_color = {}, optional<float> clear_depth = {}, bool one_shot = true) {
			return begin_drawing(ctx, clear_color ? float4(*clear_color, 1) : float4{0, 0, 0, 1}, clear_depth, one_shot);
		}

		void release() {
			color.release();
			depth.release();
		}
	};
	using gbuffer = geometry_buffer;


//////////////////////////////////////////////////////////////////////
// # Mesh
//////////////////////////////////////////////////////////////////////


	struct gpu_mesh {
		STYLIZER_API_TYPE(buffer) vertex_buffer, index_buffer = {};
		api::buffer& get_vertex_buffer() { return vertex_buffer; }
		api::buffer& get_index_buffer() { return index_buffer; }

		operator bool() const { return vertex_buffer || index_buffer; }

		void release() {
			vertex_buffer.release();
			index_buffer.release();
		}
	};
	struct mesh: public gpu_mesh {
		using index_t = uint32_t;
		template<bool refs = false>
		struct vertex {
			const static float4 not_available;
			template<typename T>
			using ref_t = std::conditional_t<refs, std::add_lvalue_reference_t<T>, T>;

			index_t index, triangle;
			ref_t<float4> position; // position (xyz), mask 1 (w)
			ref_t<float4> normal; // normal (xyz), mask 2 (w)
			ref_t<float4> tangent; // tangent (xyz), sign to compute bitangent (w)
			ref_t<float4> uv; // uv 1 (xy), uv 2 (zw)

			ref_t<float4> cta; // curvature (x), thickness (y), associated faces begin (z) and length (w)
			ref_t<float4> color; // vertex color
			ref_t<uint4> bones; // 4x bone IDs
			ref_t<float4> bone_weights; // 4x bone weights
		};

		index_t triangle_count;		// Number of triangles stored (indexed or not)
		index_t vertex_count;		// Number of vertices stored (length of each of the following arrays)

		// Vertex attributes data
		std::vector<float4> positions;					// Vertex position (xyz) mask 1 (w) (shader-location = 0) NOTE: Not nullable
		std::optional<std::vector<float4>> normals;		// Vertex normals (xyz) mask 2 (w) (shader-location = 1)
		std::optional<std::vector<float4>> tangents;	// Vertex tangents (xyz) and sign (w) to compute bitangent (shader-location = 2)
		std::optional<std::vector<float4>> uvs;			// Vertex texture coordinates (xy) and (zw) (shader-location = 3)

		std::optional<std::vector<float4>> cta;			// Vertex curvature (x), thickness (y), and associated faces begin (z) and length (w) (shader-location = 4)
		std::optional<std::vector<float4>> colors;		// Vertex colors (shader-location = 5)
		std::optional<std::vector<uint4>> bones;		// Bone IDs (shader-location = 6)
		std::optional<std::vector<float4>> bone_weights;// Bone weights associated with each ID (shader-location = 7)

		std::optional<std::vector<index_t>> indices;	// Vertex indices (in case vertex data comes indexed)


		// Meshes are sorted based on wherever their positions happened to be stored in memory
		bool operator<(const mesh& o) const {
			return positions.data() < o.positions.data();
		}
		bool operator<(const mesh* o) const { // TODO: Why is this nessicary?
			assert(o);
			return positions.data() < o->positions.data();
		}

		static const std::vector<api::render::pipeline::config::vertex_buffer_layout>& vertex_buffer_layout() {
			static std::vector<api::render::pipeline::config::vertex_buffer_layout> layout {
				{{{api::render::pipeline::config::vertex_buffer_layout::attribute::format_of<float4>()}}}, // positions
				{{{api::render::pipeline::config::vertex_buffer_layout::attribute::format_of<float4>()}}}, // normals
				{{{api::render::pipeline::config::vertex_buffer_layout::attribute::format_of<float4>()}}}, // tangents
				{{{api::render::pipeline::config::vertex_buffer_layout::attribute::format_of<float4>()}}}, // uvs
				{{{api::render::pipeline::config::vertex_buffer_layout::attribute::format_of<float4>()}}}, // cta
				{{{api::render::pipeline::config::vertex_buffer_layout::attribute::format_of<float4>()}}}, // colors
				{{{api::render::pipeline::config::vertex_buffer_layout::attribute::format_of<uint4>()}}}, // bones
				{{{api::render::pipeline::config::vertex_buffer_layout::attribute::format_of<float4>()}}}, // bone_weights
			};
			return layout;
		}

		struct metadata {
			api::bool32 is_indexed;
			uint32_t vertex_count;
			uint32_t triangle_count;

			uint32_t position_start;
			uint32_t normals_start;
			uint32_t tangents_start;
			uint32_t uvs_start;
			uint32_t cta_start;

			uint32_t colors_start;
			uint32_t bones_start;
			uint32_t bone_weights_start;

			uint32_t vertex_buffer_size;

			static metadata for_mesh(const mesh& mesh) {
				metadata meta = {
					.is_indexed = (bool)mesh.index_buffer,
					.vertex_count = mesh.vertex_count,
					.triangle_count = mesh.triangle_count,

					.position_start = sizeof(metadata), // Others should default to zero :)
				};
				size_t offset = mesh.vertex_count * sizeof(float4) + sizeof(metadata); // Account for position and metadata
				if(mesh.normals) (meta.normals_start = offset, offset += mesh.vertex_count * sizeof(float4));
				if(mesh.tangents) (meta.tangents_start = offset, offset += mesh.vertex_count * sizeof(float4));
				if(mesh.uvs) (meta.uvs_start = offset, offset += mesh.vertex_count * sizeof(float4));
				if(mesh.cta) (meta.cta_start = offset, offset += mesh.vertex_count * sizeof(float4));
				if(mesh.colors) (meta.colors_start = offset, offset += mesh.vertex_count * sizeof(float4));
				if(mesh.bones) (meta.bones_start = offset, offset += mesh.vertex_count * sizeof(float4));
				if(mesh.bone_weights) (meta.bone_weights_start = offset, offset += mesh.vertex_count * sizeof(float4));
				meta.vertex_buffer_size = offset;
				return meta;
			}

			metadata& make_gpu() {
				position_start -= sizeof(metadata);
				if(normals_start > 0) normals_start -= sizeof(metadata);
				if(tangents_start > 0) tangents_start -= sizeof(metadata);
				if(uvs_start > 0) uvs_start -= sizeof(metadata);
				if(cta_start > 0) cta_start -= sizeof(metadata);
				if(colors_start > 0) colors_start -= sizeof(metadata);
				if(bones_start > 0) bones_start -= sizeof(metadata);
				if(bone_weights_start > 0) bone_weights_start -= sizeof(metadata);
				return *this;
			}
			static metadata gpu_for_mesh(const mesh& mesh) { return for_mesh(mesh).make_gpu(); }
		};

		metadata cpu_metadata() const { return metadata::for_mesh(*this); }
		metadata gpu_metadata() const { return metadata::gpu_for_mesh(*this); }

		vertex<true> operator[](index_t index) {
			auto triangle = index / 3;
			if(indices) index = indices->at(index);

			return {
				index, triangle,
				positions.at(index),
				normals ? normals->at(index) : (float4&)vertex<true>::not_available,
				tangents ? tangents->at(index) : (float4&)vertex<true>::not_available,
				uvs ? uvs->at(index) : (float4&)vertex<true>::not_available,
				cta ? cta->at(index) : (float4&)vertex<true>::not_available,
				colors ? colors->at(index) : (float4&)vertex<true>::not_available,
				bones ? bones->at(index) : (uint4&)vertex<true>::not_available,
				bone_weights ? bone_weights->at(index) : (float4&)vertex<true>::not_available,
			};
		}

		mesh& upload(context& ctx) {
			assert(!positions.empty());

			auto metadata = cpu_metadata();
			auto gpu_metadata = metadata; gpu_metadata.make_gpu();
			size_t attribute_size = metadata.vertex_count * sizeof(float4);

			if(vertex_buffer) vertex_buffer.release();
			vertex_buffer = ctx.device.create_buffer(api::usage::Vertex, metadata.vertex_buffer_size, true, "Stylizer Vertex Buffer");
			auto mapped = vertex_buffer.get_mapped_range(true);

			std::memcpy(mapped, &gpu_metadata, sizeof(metadata));
			std::memcpy(mapped + metadata.position_start, positions.data(), attribute_size);
			if(normals) std::memcpy(mapped + metadata.normals_start, normals->data(), attribute_size);
			if(tangents) std::memcpy(mapped + metadata.tangents_start, tangents->data(), attribute_size);
			if(uvs) std::memcpy(mapped + metadata.uvs_start, uvs->data(), attribute_size);
			if(cta) std::memcpy(mapped + metadata.cta_start, cta->data(), attribute_size);
			if(colors) std::memcpy(mapped + metadata.colors_start, colors->data(), attribute_size);
			if(bones) std::memcpy(mapped + metadata.bones_start, bones->data(), attribute_size);
			if(bone_weights) std::memcpy(mapped + metadata.bone_weights_start, bone_weights->data(), attribute_size);
			vertex_buffer.unmap();

			if(indices) {
				if(index_buffer) index_buffer.release();
				index_buffer = ctx.device.create_and_write_buffer(api::usage::Index, byte_span<index_t>(*indices), 0, "Stylizer Index Buffer");
			}
			return *this;
		}
	};

	struct transformed_gpu_mesh {
		STYLIZER_API_TYPE(buffer) vertex_buffer_transformed, index_buffer_transformed = {};
		api::buffer& get_vertex_buffer() { return vertex_buffer_transformed; }
		api::buffer& get_index_buffer() { return index_buffer_transformed; }

		operator bool() const { return vertex_buffer_transformed || index_buffer_transformed; }

		void release() {
			vertex_buffer_transformed.release();
			index_buffer_transformed.release();
		}
	};
	struct transformed_mesh: public mesh, public transformed_gpu_mesh {
		using transformed_gpu_mesh::get_index_buffer;
		using transformed_gpu_mesh::get_vertex_buffer;

		operator bool() const { return mesh::operator bool() || transformed_gpu_mesh::operator bool(); }

		void release() {
			mesh::release();
			transformed_gpu_mesh::release();
		}
	};
	// struct shape_key_mesh: public transformed_mesh {
	// 	struct shape_key {
	// 		float weight;
	// 		struct mesh mesh;
	// 	};
	// 	std::unordered_map<std::string, shape_key> shape_keys;
	// };


//////////////////////////////////////////////////////////////////////
// # Material
//////////////////////////////////////////////////////////////////////


	struct shader_processor {
		using entry_points = std::unordered_map<api::shader::stage, std::string_view>;

		static slcross::slang::session* get_session() {
			static slcross::slang::session* session = slcross::slang::create_session();
			return session;
		}

		static void inject_default_virtual_filesystem();

		static std::pair<std::vector<managable<STYLIZER_API_TYPE(shader)>>, api::pipeline::entry_points> process_shaders(context& ctx, std::string_view content, const entry_points& eps, std::string_view module = "generated") {
			inject_default_virtual_filesystem();

			std::vector<managable<STYLIZER_API_TYPE(shader)>> shaders; shaders.reserve(eps.size());
			api::pipeline::entry_points api;
			auto session = get_session();
			auto path = std::string{module} + ".slang";
			for(auto& [stage, ep]: eps) {
				auto spirv = slcross::glsl::canonicalize(
					slcross::slang::parse_from_memory(session, content, ep, path, module),
					api::shader::to_slcross(stage)
				);
				assert(spirv.size());
				shaders.emplace_back(true, ctx.device.create_shader_from_spirv(std::move(spirv)));
				api.emplace(stage, api::pipeline::entry_point{.shader = &shaders.back().value});
			}
			return {std::move(shaders), api};
		}
	};


	struct material {
		STYLIZER_API_TYPE(render_pipeline) pipeline = {};
		std::vector<managable<STYLIZER_API_TYPE(shader)>> shaders;
		std::vector<managable<STYLIZER_API_TYPE(buffer)>> buffers;
		std::vector<managable<texture>> textures;

		operator bool() const { return pipeline; }

		static material create_from_shaders(context& ctx, const api::pipeline::entry_points& entry_points, std::span<const api::color_attachment> color_attachments = {}, const std::optional<api::depth_stencil_attachment>& depth_attachment = {}, const std::vector<api::render::pipeline::config::vertex_buffer_layout>& vertex_layout = mesh::vertex_buffer_layout(), const api::render_pipeline::config& config = {}) {
			material out{};
			out.upload_from_shaders(ctx, entry_points, color_attachments, depth_attachment, vertex_layout, config);
			return out;
		}
		static material create_from_shaders_for_geometry_buffer(context& ctx, const api::pipeline::entry_points& entry_points, geometry_buffer& gbuffer, const std::vector<api::render::pipeline::config::vertex_buffer_layout>& vertex_layout = mesh::vertex_buffer_layout(), const api::render_pipeline::config& config = {}) {
			material out{};
			out.upload_from_shaders_for_geometry_buffer(ctx, entry_points, gbuffer, vertex_layout, config);
			return out;
		}
		static material create_from_source(context& ctx, std::string_view content, const shader_processor::entry_points& entry_points, std::span<const api::color_attachment> color_attachments = {}, const std::optional<api::depth_stencil_attachment>& depth_attachment = {}, const std::vector<api::render::pipeline::config::vertex_buffer_layout>& vertex_layout = mesh::vertex_buffer_layout(), std::string_view module = "generated", const api::render_pipeline::config& config = {}) {
			material out{};
			out.upload_from_source(ctx, content, entry_points, color_attachments, depth_attachment, vertex_layout, module, config);
			return out;
		}
		static material create_from_source_for_geometry_buffer(context& ctx, std::string_view content, const shader_processor::entry_points& entry_points, geometry_buffer& gbuffer, const std::vector<api::render::pipeline::config::vertex_buffer_layout>& vertex_layout = mesh::vertex_buffer_layout(), std::string_view module = "generated", const api::render_pipeline::config& config = {}) {
			material out{};
			out.upload_from_source_for_geometry_buffer(ctx, content, entry_points, gbuffer, vertex_layout, module, config);
			return out;
		}

		material& upload_from_shaders(context& ctx, const api::pipeline::entry_points& entry_points, std::span<const api::color_attachment> color_attachments = {}, const std::optional<api::depth_stencil_attachment>& depth_attachment = {}, const std::vector<api::render::pipeline::config::vertex_buffer_layout>& vertex_layout = mesh::vertex_buffer_layout(), api::render_pipeline::config config = {}) {
			if(!vertex_layout.empty()) config.vertex_buffers = vertex_layout;
			if(pipeline) pipeline.release();
			pipeline = ctx.device.create_render_pipeline(entry_points, color_attachments, depth_attachment, config, "Stylizer Default Material Pipeline");
			return *this;
		}
		material& upload_from_shaders_for_geometry_buffer(context& ctx, const api::pipeline::entry_points& entry_points, geometry_buffer& gbuffer, const std::vector<api::render::pipeline::config::vertex_buffer_layout>& vertex_layout = mesh::vertex_buffer_layout(), api::render_pipeline::config config = {}) {
			if(!vertex_layout.empty()) config.vertex_buffers = vertex_layout;
			if(pipeline) pipeline.release();
			auto_release pass = gbuffer.begin_drawing(ctx);
			pipeline = ctx.device.create_render_pipeline_from_compatible_render_pass(entry_points, pass, config, "Stylizer Default Material Pipeline");
			return *this;
		}

		material& upload_from_source(context& ctx, std::string_view content, const shader_processor::entry_points& entry_points, std::span<const api::color_attachment> color_attachments = {}, const std::optional<api::depth_stencil_attachment>& depth_attachment = {}, const std::vector<api::render::pipeline::config::vertex_buffer_layout>& vertex_layout = mesh::vertex_buffer_layout(), std::string_view module = "generated", const api::render_pipeline::config& config = {}) {
			auto [shaders, eps] = shader_processor::process_shaders(ctx, content, entry_points, module);
			release_shaders();
			this->shaders = std::move(shaders);
			return upload_from_shaders(ctx, eps, color_attachments, depth_attachment, vertex_layout, config);
		}
		material& upload_from_source_for_geometry_buffer(context& ctx, std::string_view content, const shader_processor::entry_points& entry_points, geometry_buffer& gbuffer, const std::vector<api::render::pipeline::config::vertex_buffer_layout>& vertex_layout = mesh::vertex_buffer_layout(), std::string_view module = "generated", const api::render_pipeline::config& config = {}) {
			auto [shaders, eps] = shader_processor::process_shaders(ctx, content, entry_points, module);
			release_shaders();
			this->shaders = std::move(shaders);
			return upload_from_shaders_for_geometry_buffer(ctx, eps, gbuffer, vertex_layout, config);
		}

		void release_shaders() {
			for(auto& shader: shaders)
				if(shader.is_managed) shader->release();
		}

		void release() {
			pipeline.release();
			release_shaders();
			for(auto& buffer: buffers)
				if(buffer.is_managed) buffer->release();
			for(auto& texture: textures)
				if(texture.is_managed) texture->release();
		}
	};


//////////////////////////////////////////////////////////////////////
// # Model
//////////////////////////////////////////////////////////////////////


	struct instance_data {
		float4x4 transform = float4x4::identity();
		float4 color = 1;
	};

	template<std::derived_from<mesh> Tmesh = mesh, std::derived_from<material> Tmaterial = material>
	struct model {
		template<typename T>
		using maybe_owned_t = std::variant<T, T*>;
		template<typename T>
		static T& get(maybe_owned_t<T>& owned) {
			if(owned.index() == 0)
				return std::get<0>(owned);

			auto ptr = std::get<1>(owned);
			assert(ptr);
			return *ptr;
		}
		template<typename T>
		static const T& get(const maybe_owned_t<T>& owned) {
			if(owned.index() == 0)
				return std::get<0>(owned);

			auto ptr = std::get<1>(owned);
			assert(ptr);
			return *ptr;
		}
		template<typename T>
		static bool valid(const maybe_owned_t<T>& owned) {
			return (owned.index() == 0 && std::get<0>(owned))
				|| (owned.index() == 1 && std::get<1>(owned) && *std::get<1>(owned));
		}

		float4x4 transform = float4x4::identity();
		maybe_owned_t<Tmaterial> default_material = nullptr;
		struct material_mapped_mesh{
			maybe_owned_t<Tmesh> mesh;
			STYLIZER_NULLABLE(maybe_owned_t<Tmaterial>) material = nullptr;

			bool operator<(const material_mapped_mesh& o) const {
				return get(mesh) < get(o.mesh);
			}
		};
		std::set<material_mapped_mesh> material_mapped_meshes;

		operator bool() const {
			if(valid(default_material)) return true;
			for(auto& [mesh, material]: material_mapped_meshes)
				if(valid(mesh) || valid(material)) return true;
			return false;
		}

		model& draw_instanced(drawing_state& draw, std::span<const instance_data> instances) {
			using namespace api::operators;

			assert(!instances.empty());
			auto instance_buffer = draw.context->device.create_and_write_buffer(api::usage::Storage, byte_span<instance_data>(instances), 0, "Stylizer Instance Buffer");
			draw.defer([instance_buffer]() mutable { instance_buffer.release(); });

			for(auto& [mesh_, material_]: material_mapped_meshes) {
				auto& mesh = const_cast<Tmesh&>(get(mesh_));
				auto& material = valid(material_) ? get(material_) : (assert(valid(default_material)), get(default_material));

				auto metadata = mesh.cpu_metadata();
				size_t attribute_size = metadata.vertex_count * sizeof(float4);

				auto& zero_buffer = draw.get_zero_buffer_singleton(api::usage::Index | api::usage::Vertex, attribute_size);

				draw.bind_render_pipeline(draw, material.pipeline)
					.bind_vertex_buffer(draw, 0, mesh.get_vertex_buffer(), metadata.position_start, attribute_size)
					.bind_vertex_buffer(draw, 1, mesh.normals ? mesh.get_vertex_buffer() : zero_buffer, metadata.normals_start, attribute_size)
					.bind_vertex_buffer(draw, 2, mesh.tangents ? mesh.get_vertex_buffer() : zero_buffer, metadata.tangents_start, attribute_size)
					.bind_vertex_buffer(draw, 3, mesh.uvs ? mesh.get_vertex_buffer() : zero_buffer, metadata.uvs_start, attribute_size)
					.bind_vertex_buffer(draw, 4, mesh.cta ? mesh.get_vertex_buffer() : zero_buffer, metadata.cta_start, attribute_size)
					.bind_vertex_buffer(draw, 5, mesh.colors ? mesh.get_vertex_buffer() : zero_buffer, metadata.colors_start, attribute_size)
					.bind_vertex_buffer(draw, 6, mesh.bones ? mesh.get_vertex_buffer() : zero_buffer, metadata.bones_start, attribute_size)
					.bind_vertex_buffer(draw, 7, mesh.bone_weights ? mesh.get_vertex_buffer() : zero_buffer, metadata.bone_weights_start, attribute_size);
				if(metadata.is_indexed)
					draw.bind_index_buffer(draw, mesh.get_index_buffer())
						.draw_indexed(draw, metadata.triangle_count * 3);
				else draw.draw(draw, metadata.vertex_count, instances.size());
			}

			return *this;
		}
		model& draw(drawing_state& draw) { return draw_instanced(draw, span_from_value(instance_data{})); }

		void release() {
			if(default_material.index() == 0) std::get<0>(default_material).release();
			default_material = nullptr;

			for(auto& [mesh, material]: material_mapped_meshes) {
				if(mesh.index() == 0) const_cast<Tmesh&>(std::get<0>(mesh)).release();
				if(material.index() == 0) const_cast<Tmaterial&>(std::get<0>(material)).release();
			}
			material_mapped_meshes.clear();
		}
	};


	namespace core { // default
		using mesh = stylizer::mesh;
		using material = stylizer::material;
		using model = stylizer::model<mesh, material>;
	}

} // namespace stylizer