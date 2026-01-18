#pragma once

#include <stylizer/api/api.hpp>
#include <stylizer/api/backends/current_backend.hpp>
#include <reaction/reaction.h>

#include <math/matrix.hpp>

#include "backends/webgpu/webgpu.hpp"
#include "util/maybe_owned.hpp"

#include <chrono>
#include <cstddef>
#include <ratio>
#include <unordered_map>

namespace stylizer {
	using namespace api::operators;

	/**
	 * @brief The central context for stylizer, managing the GPU device and events.
	 */
	struct context : public api::current_backend::device {
		/** @brief Base class for events. */
		struct event {
			/** @brief Virtual destructor for event. */
			virtual ~event() {}
		};

		/** @brief Signal emitted to process events. */
		signal<void(context&)> process_events;
		/** @brief Signal emitted to handle a specific event. */
		signal<void(const event&)> handle_event;

		/**
		 * @brief Updates the context by processing events.
		 * @return Reference to this context.
		 */
		context& update() {
			process_events(*this);
			return *this;
		}

		/**
		 * @brief Creates a default context.
		 * @param config Configuration for device creation.
		 * @return The created context.
		 *
		 * @code
		 * auto ctx = stylizer::context::create_default();
		 * @endcode
		 */
		static context create_default(const api::device::create_config& config = {}) {
			context out;
			static_cast<api::current_backend::device&>(out) = api::current_backend::device::create_default(config);
			out.process_events.connect([](context& ctx) {
				static_cast<api::current_backend::device&>(ctx).process_events();
			});
			return out;
		}

		/**
		 * @brief Registers a default error handler that throws on errors and prints to stderr otherwise.
		 * @return The connection object.
		 */
		connection_raw register_default_error_handler() {
			auto& errors = get_error_handler();
			return errors.connect([](api::error::severity severity, std::string_view message, size_t) {
				if (severity >= api::error::severity::Error)
					throw api::error(message);
				std::cerr << message << std::endl;
			});
		}

		/**
		 * @brief Creates a default context with the default error handler registered.
		 * @param config Configuration for device creation.
		 * @return The created context.
		 */
		static context create_default_with_error_handler(const api::device::create_config& config = {}) {
			context{}.register_default_error_handler();
			return create_default(config);
		}

		/**
		 * @brief Sends an error/info message to the error handler.
		 * @param severity Severity of the message.
		 * @param message The message string.
		 * @param error_tag Optional tag for the error.
		 */
		// TODO: Is there a better name than send?
		void send(error_severity severity, std::string_view message, size_t error_tag = 0) {
			get_error_handler()(severity, message, error_tag);
		}
		/** @brief Sends an error message. */
		void send_error(std::string_view message, size_t error_tag = 0) {
			send(error_severity::Error, message, error_tag);
		}
		/** @brief Sends a warning message. */
		void send_warning(std::string_view message, size_t error_tag = 0) {
			send(error_severity::Warning, message, error_tag);
		}
		/** @brief Sends an info message. */
		void send_info(std::string_view message, size_t error_tag = 0) {
			send(error_severity::Info, message, error_tag);
		}
		/** @brief Sends a verbose message. */
		void send_verbose(std::string_view message, size_t error_tag = 0) {
			send(error_severity::Verbose, message, error_tag);
		}

	};



	/**
	 * @brief Represents a GPU texture with reactive properties.
	 */
	struct texture : public api::current_backend::texture {
		/** @brief Default constructor. */
		texture() = default;
		/** @brief Copy constructor (deleted). */
		texture(const texture&) = delete; // TODO: Textures should probably be copyable...
		/** @brief Move constructor. */
		texture(texture&&) = default;
		/** @brief Copy assignment operator (deleted). */
		texture& operator=(const texture&) = delete;
		/** @brief Move assignment operator. */
		texture& operator=(texture&&) = default;

		/** @brief Configuration used to create the texture. */
		create_config values;

		/** @brief Reactive variable for the texture size. */
		reaction::Var<stdmath::vector<size_t, 3>> size;

		/**
		 * @brief Creates a texture.
		 * @param device The GPU device.
		 * @param config Configuration for texture creation.
		 * @return The created texture.
		 *
		 * @code
		 * auto tex = stylizer::texture::create(ctx, { .size = {1024, 1024, 1} });
		 * @endcode
		 */
		static texture create(api::device& device, const create_config& config = {}) {
			texture out;
			out.values = config;
			out.size = reaction::var(out.values.size);
			static_cast<api::current_backend::texture&>(out) = api::current_backend::texture::create(device, config);
			out.resize = reaction::action([](const stdmath::vector<size_t, 3>& size){
				// TODO: Create new texture and blit the current texture to it
			}, out.size);
			return out;
		}

		/**
		 * @brief Creates a texture and writes initial data to it.
		 * @param device The GPU device.
		 * @param data Initial data.
		 * @param layout Layout of the data.
		 * @param config Configuration for texture creation.
		 * @return The created texture.
		 */
		static texture create_and_write(api::device& device, std::span<const std::byte> data, const data_layout& layout, create_config config = {}) {
			config.size = { data.size() / layout.rows_per_image / bytes_per_pixel(config.format), layout.rows_per_image, 1 };
			config.usage |= api::usage::CopyDestination;
			auto out = create(device, config);
			out.write(device, data, layout, config.size);
			return out;
		}

		/**
		 * @brief Gets a singleton default texture (e.g. for fallback).
		 * @param ctx The stylizer context.
		 * @return Reference to the default texture.
		 */
		static texture& get_default_texture(context& ctx) {
			static texture global = [](context& ctx) -> texture {
				std::array<stdmath::vector<float, 4>, 4> default_texture_data = {{
					{1, 0, 0, 1},
					{0, 1, 0, 1},
					{0, 0, 1, 1},
					{1, 1, 1, 1}
				}};
				auto out = stylizer::texture::create_and_write(ctx, stylizer::byte_span<stdmath::vector<float, 4>>(default_texture_data), stylizer::api::texture::data_layout{
					.offset = 0,
					.bytes_per_row = sizeof(default_texture_data[0]) * 2,
					.rows_per_image = 2,
				}, {
					.format = api::texture::format::RGBA32
				});
				out.configure_sampler(ctx);
				return out;
			}(ctx);

			return global;
		}

		/**
		 * @brief Configures the sampler for this texture.
		 * @param device The GPU device.
		 * @param config Sampler configuration.
		 * @return Reference to this texture.
		 */
		texture& configure_sampler(api::device &device, const sampler_config &config = {}) {
			api::current_backend::texture::configure_sampler(device, config);
			return *this;
		}

	protected:
		/** @brief Reactive action for resizing the texture. */
		reaction::Action<> resize;
	};

	/**
	 * @brief Represents a surface that can be rendered to (e.g. a window surface).
	 */
	struct surface : protected api::current_backend::surface {
		/** @brief Default constructor. */
		surface() = default;
		/** @brief Copy constructor (deleted). */
		surface(const surface&) = delete;
		/** @brief Move constructor. */
		surface(surface&&) = default;
		/** @brief Copy assignment operator (deleted). */
		surface& operator=(const surface&) = delete;
		/** @brief Move assignment operator. */
		surface& operator=(surface&&) = default;

		/** @brief Reactive variable for the surface size. */
		reaction::Var<stdmath::vector<size_t, 2>> size;
		/** @brief Reactive variable for the present mode. */
		reaction::Var<enum present_mode> present_mode; //= surface::present_mode::Fifo;
		/** @brief Reactive variable for the texture format. */
		reaction::Var<api::texture_format> texture_format; //= api::texture_format::RGBAu8_NormalizedSRGB;
		/** @brief Reactive variable for the alpha mode. */
		reaction::Var<api::alpha_mode> alpha_mode; //= api::alpha_mode::Opaque;
		/** @brief Reactive variable for the usage flags. */
		reaction::Var<api::usage> usage; // = api::usage::RenderAttachment;
		/** @brief Reactive variable for the associated context. */
		reaction::Var<struct context*> associated_context;

		using api::current_backend::surface::operator bool;
		using api::current_backend::surface::next_texture;

		/**
		 * @brief Presents the surface.
		 * @param device The GPU device.
		 * @return Reference to this surface.
		 */
		surface& present(api::device& device) override {
			api::current_backend::surface::present(device);
			return *this;
		}

		/**
		 * @brief Presents a color texture to the surface.
		 * @param ctx The stylizer context.
		 * @param color_texture The texture to present.
		 * @return Reference to this surface.
		 */
		surface& present(context& ctx, texture& color_texture) {
			stylizer::auto_release texture = next_texture(ctx);
			texture.blit_from(ctx, color_texture);
			return present(ctx);
		}

		using api::current_backend::surface::release;
		using api::current_backend::surface::auto_release;

		friend struct window;
protected:
		/** @brief Internal creation method. */
		static surface create(context& ctx, api::current_backend::surface& surface, const stdmath::vector<size_t, 2>& size);

		/** @brief Reactive action for resizing the surface. */
		reaction::Action<> resize;
	};



	/**
	 * @brief Abstract interface for a frame buffer that can be rendered into.
	 */
	struct frame_buffer {
		/** @brief Reactive variable for the frame buffer size. */
		reaction::Var<stdmath::vector<size_t, 3>> size;
		/** @brief Clear value for the color attachment. */
		std::optional<stdmath::vector<float, 4>> clear_value = {};

		/**
		 * @brief Gets the primary color texture.
		 * @return Reference to the texture.
		 */
		virtual texture& color_texture() = 0;
		/**
		 * @brief Gets the color attachments for a render pass.
		 * @param attachment_template Template for the attachments.
		 * @return A span of color attachments.
		 */
		virtual std::span<const api::color_attachment> color_attachments(api::color_attachment attachment_template = {}) const = 0;
		/**
		 * @brief Gets the depth/stencil attachment for a render pass.
		 * @param attachment_template Template for the attachment.
		 * @return Optional depth/stencil attachment.
		 */
		virtual std::optional<api::depth_stencil_attachment> depth_stencil_attachment(api::depth_stencil_attachment attachment_template = {}) const { return {}; }

		/**
		 * @brief Creates a render pass for this frame buffer.
		 * @param ctx The stylizer context.
		 * @param color_template Template for color attachments.
		 * @param depth_template Template for depth/stencil attachment.
		 * @param one_shot Whether this is a one-shot pass.
		 * @return The created render pass.
		 */
		virtual api::current_backend::render::pass create_render_pass(context& ctx, api::color_attachment color_template = {}, api::depth_stencil_attachment depth_template = {}, bool one_shot = true) {
			return ctx.create_render_pass(color_attachments(color_template), depth_stencil_attachment(depth_template), one_shot);
		}

		/** @brief Default configuration for drawing to depth. */
		constexpr static api::depth_stencil_attachment default_draw_to_depth_config {
			.depth_clear_value = 1,
			.depth_comparison_function = api::comparison_function::Less
		};

		/**
		 * @brief Helper to perform drawing operations to this frame buffer.
		 * @param ctx The stylizer context.
		 * @param func The drawing function, receiving a render pass.
		 * @param color_template Template for color attachments.
		 * @param depth_template Template for depth/stencil attachment.
		 * @return The result of the drawing function or reference to this frame buffer.
		 *
		 * @code
		 * fb.draw_to(ctx, [&](auto& pass) {
		 *    mdl.draw(ctx, pass);
		 * });
		 * @endcode
		 */
		template<typename Tfunc>
		auto&& draw_to(context& ctx, const Tfunc& func, api::color_attachment color_template = {}, api::depth_stencil_attachment depth_template = default_draw_to_depth_config) {
			auto pass = create_render_pass(ctx, color_template, depth_template);
			if constexpr (std::is_same_v<decltype(func(pass)), void>) {
				func(pass);
				pass.one_shot_submit(ctx);
				return *this;
			} else {
				auto out = func(pass);
				pass.one_shot_submit(ctx);
				return out;
			}
		}
	};



	/**
	 * @brief Represents a material, which encapsulates a render pipeline and its bindings.
	 */
	struct material : public api::current_backend::render::pipeline {
		/** @brief Default constructor. */
		material() = default; // TODO: Default materials okay?
		/** @brief Constructor from a render pipeline. */
		material(api::current_backend::render::pipeline&& pipeline)
			: api::current_backend::render::pipeline(std::move(pipeline)) {}

		/** @brief Copy constructor (const). */
		material(const material&) = default;
		/** @brief Copy constructor. */
		material(material&) = default;

		/** @brief Move assignment from a render pipeline. */
		material& operator=(api::current_backend::render::pipeline&& pipeline) {
			static_cast<api::current_backend::render::pipeline&>(*this) = std::move(pipeline);
			return *this;
		}
		/** @brief Copy assignment operator (const). */
		material& operator=(const material&) = default;
		/** @brief Copy assignment operator. */
		material& operator=(material&) = default;

		/**
		 * @brief Gets the textures used by this material.
		 * @param ctx The stylizer context.
		 * @return A span of textures.
		 */
		virtual std::span<maybe_owned<api::current_backend::texture>> textures(context& ctx) = 0;
		/**
		 * @brief Gets the buffers used by this material.
		 * @param ctx The stylizer context.
		 * @return A span of buffers.
		 */
		virtual std::span<maybe_owned<api::current_backend::buffer>> buffers(context& ctx) = 0;
		/**
		 * @brief Gets the mesh attributes requested by this material's shader.
		 * @return A span of attribute names.
		 */
		virtual std::span<std::string_view> requested_mesh_attributes() = 0;

		/**
		 * @brief Gets the bind groups for this material.
		 * @param ctx The stylizer context.
		 * @return A span of bind groups.
		 */
		virtual std::span<api::current_backend::bind_group> bind_groups(context& ctx) = 0;
	};

	/**
	 * @brief Concept for a valid material type.
	 */
	template<typename T>
	concept material_concept = std::derived_from<T, material> && requires (T t, context ctx, frame_buffer fb) {
		{ T::create(ctx, fb) } -> std::convertible_to<T>;
	};

	/** @brief Common mesh attribute names. */
	inline namespace common_mesh_attributes {
		constexpr static std::string_view positions = "positions";
		constexpr static std::string_view normals = "normals";
		constexpr static std::string_view colors = "colors";
		constexpr static std::string_view vertex_colors = "colors"; // alias to colors
		constexpr static std::string_view uvs = "uvs";
		constexpr static std::string_view texture_coordinates = "uvs"; // alias to uvs
	}


	/**
	 * @brief A GPU buffer whose data can be managed and uploaded easily.
	 */
	struct managed_buffer : public api::current_backend::buffer {
		/**
		 * @brief Converts the underlying data to a byte span for upload.
		 * @return A span of bytes.
		 */
		virtual std::span<const std::byte> to_bytes() = 0;
		/**
		 * @brief Uploads the data to the GPU.
		 * @param ctx The stylizer context.
		 * @param label Optional label for the GPU buffer.
		 * @return Reference to this buffer.
		 */
		virtual managed_buffer& upload(context& ctx, std::string_view label = "Stylizer Managed Buffer") {
			auto bytes = to_bytes();
			if(!*this || size() < bytes.size()) {
				auto usage = *this ? this->usage() : api::usage::Storage;
				static_cast<api::current_backend::buffer&>(*this) = ctx.create_and_write_buffer(usage, bytes, 0, label);
			} else
				write(ctx, bytes);
			return *this;
		}
	};


	/**
	 * @brief Tracks time and frame information for the application.
	 */
	struct time {
		/** @brief Total elapsed time, delta time since last frame, and smoothed delta time. */
		float total = 0, delta = 0, smoothed_delta = 0;
		/** @brief Total number of frames processed. */
		uint32_t frame = 0;

		/**
		 * @brief Computes the delta time since the last call.
		 * @tparam Tprecision Precision for duration cast.
		 * @tparam Tclock The clock to use.
		 * @return Reference to this time object.
		 */
		template<typename Tprecision = std::micro, typename Tclock = std::chrono::system_clock>
		inline time& compute_delta() {
			static auto last_time = Tclock::now();
			auto now = Tclock::now();
			auto dt_precise = std::chrono::duration_cast<std::chrono::duration<int64_t, Tprecision>>(now - last_time).count();
			delta = double(dt_precise * Tprecision::num) / Tprecision::den;
			last_time = now;
			return *this;
		}

		/**
		 * @brief Increments the frame counter.
		 * @return Reference to this time object.
		 */
		inline time& increment_frame() {
			++frame;
			return *this;
		}

		/**
		 * @brief Computes delta and increments frame.
		 * @return Reference to this time object.
		 */
		template<typename Tprecision = std::micro, typename Tclock = std::chrono::system_clock>
		inline time& compute_delta_and_frame() {
			compute_delta<Tprecision, Tclock>();
			increment_frame();
			return *this;
		}

		/**
		 * @brief Updates time and smoothed delta.
		 * @param alpha Smoothing factor for delta time.
		 * @return Reference to this time object.
		 */
		template<typename Tprecision = std::micro, typename Tclock = std::chrono::system_clock>
		inline time& update(float alpha = .9) {
			compute_delta_and_frame<Tprecision, Tclock>();
			total += delta;
			smoothed_delta = alpha * smoothed_delta + (1 - alpha) * delta;
			return *this;
		}
	};

	/**
	 * @brief Abstract interface for a camera, providing view and projection matrices.
	 */
	struct camera {
		/**
		 * @brief Gets the view matrix.
		 * @return The 4x4 view matrix.
		 */
		virtual stdmath::matrix<float, 4, 4> view_matrix() const = 0;
		/**
		 * @brief Gets the projection matrix.
		 * @param screen_size The size of the screen/view-port.
		 * @return The 4x4 projection matrix.
		 */
		virtual stdmath::matrix<float, 4, 4> projection_matrix(const stdmath::vector<size_t, 2>& screen_size) const = 0;

		/** @brief Gets the inverse view matrix. */
		virtual stdmath::matrix<float, 4, 4> inverse_view_matrix() const {
			return inverse(view_matrix());
		}
		/** @brief Gets the inverse projection matrix. */
		virtual stdmath::matrix<float, 4, 4> inverse_projection_matrix(const stdmath::vector<size_t, 2>& screen_size) const {
			return inverse(projection_matrix(screen_size));
		}
	};

	/**
	 * @brief A concrete implementation of a camera with stored matrices.
	 */
	struct concrete_camera : public camera {
		/** @brief View matrix. */
		stdmath::matrix<float, 4, 4> view;
		/** @brief Projection matrix. */
		stdmath::matrix<float, 4, 4> projection;
		/** @brief Inverse view matrix. */
		stdmath::matrix<float, 4, 4> inverse_view;
		/** @brief Inverse projection matrix. */
		stdmath::matrix<float, 4, 4> inverse_projection;

		/**
		 * @brief Creates a concrete camera from any camera implementation.
		 * @param camera The camera to copy from.
		 * @param screen_size The screen size for projection.
		 * @return The concrete camera.
		 */
		static concrete_camera from_camera(const camera& camera, const stdmath::vector<size_t, 2>& screen_size) {
			concrete_camera out;
			out.view = camera.view_matrix();
			out.projection = camera.projection_matrix(screen_size);
			out.inverse_view = camera.inverse_view_matrix();
			out.inverse_projection = camera.inverse_projection_matrix(screen_size);
			return out;
		}

		stdmath::matrix<float, 4, 4> view_matrix() const override { return view; }
		stdmath::matrix<float, 4, 4> projection_matrix(const stdmath::vector<size_t, 2>& screen_size = {}) const override { return projection; }
		stdmath::matrix<float, 4, 4> inverse_view_matrix() const override { return inverse_view; }
		stdmath::matrix<float, 4, 4> inverse_projection_matrix(const stdmath::vector<size_t, 2>& screen_size = {}) const override { return inverse_projection; }
	};

	/**
	 * @brief A GPU buffer containing common utility data like camera matrices and time.
	 */
	struct utility_buffer : public managed_buffer {
		/** @brief The concrete camera data. */
		concrete_camera camera;
		/** @brief The time data. */
		struct time time;

		std::span<const std::byte> to_bytes() override {
			// NOTE: concrete camera has a vtable so its uploadable data starts at camera.view
			return {(std::byte*)&camera.view, ((std::byte*)&time) - ((std::byte*)&camera.view) + sizeof(time)};
		}

		/**
		 * @brief Updates time and uploads the buffer to the GPU.
		 * @param ctx The stylizer context.
		 * @param alpha Smoothing factor for time.
		 * @return Reference to this buffer.
		 */
		utility_buffer& update(context& ctx, float alpha = .9) {
			time.update(alpha);
			upload(ctx, "Stylizer Utility Buffer");
			return *this;
		}
	};

	/**
	 * @brief Represents data for a single instance of a model.
	 */
	struct instance_data {
		/** @brief The model matrix for this instance. */
		stdmath::matrix<float, 4, 4> model = stdmath::matrix<float, 4, 4>::identity();

		/** @brief Base class for instance data buffers. */
		struct buffer_base : public managed_buffer {
			/** @brief Gets the number of instances in the buffer. */
			virtual size_t count() const = 0;

			/** @brief Cache for bind groups associated with this buffer. */
			std::unordered_map<utility_buffer*, api::current_backend::bind_group> group_cache;

			/**
			 * @brief Creates a bind group for this instance data and an optional utility buffer.
			 * @param ctx The stylizer context.
			 * @param pipeline The render pipeline.
			 * @param util Optional utility buffer.
			 * @param index The bind group index.
			 * @param minimum_util_size Minimum size for the utility buffer binding.
			 * @return The bind group.
			 */
			api::current_backend::bind_group make_bind_group(context& ctx,api::current_backend::render_pipeline& pipeline, std::optional<utility_buffer> util = {}, size_t index = 0, size_t minimum_util_size = 272) {
				utility_buffer* cacher = util ? &*util : nullptr;
				if(group_cache.contains(cacher)) return group_cache[cacher];

				std::array<api::bind_group::binding, 2> bindings;
				bindings[0] = api::bind_group::buffer_binding{&*this};
				if(util) bindings[1] = api::bind_group::buffer_binding{&*util};
				else bindings[1] = api::bind_group::buffer_binding{&ctx.get_zero_buffer_singleton(api::usage::Storage, minimum_util_size)};
				return group_cache[cacher] = pipeline.create_bind_group(ctx, index, bindings);
			}
		};

		/**
		 * @brief A GPU-backed array of instance data.
		 * @tparam N Number of instances (static or dynamic).
		 */
		template<size_t N = std::dynamic_extent>
		struct buffer : public buffer_base, public std::array<instance_data, N> {
			/** @brief Base type alias. */
			using base = instance_data::buffer_base;

			/** @brief Gets the number of instances in the buffer. */
			size_t count() const override { return N; }

			/** @brief Converts the underlying data to a byte span for upload. */
			std::span<const std::byte> to_bytes() override {
				return byte_span<instance_data>(static_cast<std::array<instance_data, N>&>(*this));
			}
		};
	};

	/** @brief Specialization for dynamic-size instance data buffer. */
	template<>
	struct instance_data::buffer<std::dynamic_extent> : public instance_data::buffer_base, public std::vector<instance_data> {
		/** @brief Base type alias. */
		using base = instance_data::buffer_base;

		/** @brief Gets the number of instances in the buffer. */
		size_t count() const override { return std::vector<instance_data>::size(); }

		/** @brief Converts the underlying data to a byte span for upload. */
		std::span<const std::byte> to_bytes() override {
			return byte_span<instance_data>(static_cast<std::vector<instance_data>&>(*this));
		}
	};

}
