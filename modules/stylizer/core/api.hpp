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

	struct context : public api::current_backend::device {
		struct event {
			virtual ~event() {}
		};

		signal<void(context&)> process_events;
		signal<void(const event&)> handle_event;

		context& update() {
			process_events(*this);
			return *this;
		}

		static context create_default(const api::device::create_config& config = {}) {
			context out;
			static_cast<api::current_backend::device&>(out) = api::current_backend::device::create_default(config);
			out.process_events.connect([](context& ctx) {
				static_cast<api::current_backend::device&>(ctx).process_events();
			});
			return out;
		}

		connection_raw register_default_error_handler() {
			auto& errors = get_error_handler();
			return errors.connect([](api::error::severity severity, std::string_view message, size_t) {
				if (severity >= api::error::severity::Error)
					throw api::error(message);
				std::cerr << message << std::endl;
			});
		}

		static context create_default_with_error_handler(const api::device::create_config& config = {}) {
			context{}.register_default_error_handler();
			return create_default(config);
		}

		// TODO: Is there a better name than send?
		void send(error_severity severity, std::string_view message, size_t error_tag) {
			get_error_handler()(severity, message, error_tag);
		}
		void send_error(std::string_view message, size_t error_tag) {
			send(error_severity::Error, message, error_tag);
		}
		void send_warning(std::string_view message, size_t error_tag) {
			send(error_severity::Warning, message, error_tag);
		}
		void send_info(std::string_view message, size_t error_tag) {
			send(error_severity::Info, message, error_tag);
		}
		void send_verbose(std::string_view message, size_t error_tag) {
			send(error_severity::Verbose, message, error_tag);
		}

	};



	struct texture : public api::current_backend::texture {
		texture() = default;
		texture(const texture&) = delete; // TODO: Textures should probably be copyable...
		texture(texture&&) = default;
		texture& operator=(const texture&) = delete;
		texture& operator=(texture&&) = default;

		create_config values;

		reaction::Var<stdmath::vector<size_t, 3>> size;

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
		static texture create_and_write(api::device& device, std::span<const std::byte> data, const data_layout& layout, create_config config = {}) {
			config.size = { data.size() / layout.rows_per_image / bytes_per_pixel(config.format), layout.rows_per_image, 1 };
			config.usage |= api::usage::CopyDestination;
			auto out = create(device, config);
			out.write(device, data, layout, config.size);
			return out;
		}

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

	protected:
		reaction::Action<> resize;
	};

	struct surface : protected api::current_backend::surface {
		surface() = default;
		surface(const surface&) = delete;
		surface(surface&&) = default;
		surface& operator=(const surface&) = delete;
		surface& operator=(surface&&) = default;

		reaction::Var<stdmath::vector<size_t, 2>> size;
		reaction::Var<enum present_mode> present_mode; //= surface::present_mode::Fifo;
		reaction::Var<api::texture_format> texture_format; //= api::texture_format::RGBAu8_NormalizedSRGB;
		reaction::Var<api::alpha_mode> alpha_mode; //= api::alpha_mode::Opaque;
		reaction::Var<api::usage> usage; // = api::usage::RenderAttachment;
		reaction::Var<struct context*> associated_context;

		using api::current_backend::surface::operator bool;
		using api::current_backend::surface::next_texture;

		surface& present(api::device& device) override {
			api::current_backend::surface::present(device);
			return *this;
		}

		surface& present(context& ctx, texture& color_texture) {
			stylizer::auto_release texture = next_texture(ctx);
			texture.blit_from(ctx, color_texture);
			return present(ctx);
		}

		using api::current_backend::surface::release;
		using api::current_backend::surface::auto_release;

		friend struct window;
protected:
		static surface create(context& ctx, api::current_backend::surface& surface, const stdmath::vector<size_t, 2>& size);

		reaction::Action<> resize;
	};




	struct frame_buffer {
		reaction::Var<stdmath::vector<size_t, 3>> size;
		std::optional<stdmath::vector<float, 4>> clear_value = {};

		virtual texture& color_texture() = 0;
		virtual std::span<const api::color_attachment> color_attachments(api::color_attachment attachment_template = {}) const = 0;
		virtual std::optional<api::depth_stencil_attachment> depth_stencil_attachment(api::depth_stencil_attachment attachment_template = {}) const { return {}; }

		virtual api::current_backend::render::pass create_render_pass(context& ctx, api::color_attachment color_template = {}, api::depth_stencil_attachment depth_template = {}, bool one_shot = true) {
			return ctx.create_render_pass(color_attachments(color_template), depth_stencil_attachment(depth_template), one_shot);
		}

		constexpr static api::depth_stencil_attachment default_draw_to_depth_config {
			.depth_clear_value = 1,
			.depth_comparison_function = api::comparison_function::Less
		};

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




	struct material : public api::current_backend::render::pipeline {
		material() = default; // TODO: Default materials okay?
		material(api::current_backend::render::pipeline&& pipeline)
			: api::current_backend::render::pipeline(std::move(pipeline)) {}

		material(const material&) = default;
		material(material&) = default;

		material& operator=(api::current_backend::render::pipeline&& pipeline) {
			static_cast<api::current_backend::render::pipeline&>(*this) = std::move(pipeline);
			return *this;
		}
		material& operator=(const material&) = default;
		material& operator=(material&) = default;


		virtual std::span<maybe_owned<api::current_backend::texture>> textures(context& ctx) = 0;
		virtual std::span<maybe_owned<api::current_backend::buffer>> buffers(context& ctx) = 0;
		virtual std::span<std::string_view> requested_mesh_attributes() = 0;

		virtual std::span<api::current_backend::bind_group> bind_groups(context& ctx) = 0;
	};

	template<typename T>
	concept material_concept = std::derived_from<T, material> && requires (T t, context ctx, frame_buffer gbuffer) {
		{ T::create(ctx, gbuffer) } -> std::convertible_to<T>;
	};

	inline namespace common_mesh_attributes {
		constexpr static std::string_view positions = "positions";
		constexpr static std::string_view normals = "normals";
		constexpr static std::string_view uvs = "uvs";
		constexpr static std::string_view texture_coordinates = "uvs"; // alias to uvs
	}



	struct managed_buffer : public api::current_backend::buffer {
		virtual std::span<const std::byte> to_bytes() = 0;
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



	struct time {
		float total = 0, delta = 0, smoothed_delta = 0;
		uint32_t frame = 0;

		template<typename Tprecision = std::micro, typename Tclock = std::chrono::system_clock>
		inline time& compute_delta() {
			static auto last_time = Tclock::now();
			auto now = Tclock::now();
			auto dt_precise = std::chrono::duration_cast<std::chrono::duration<int64_t, Tprecision>>(now - last_time).count();
			delta = stdmath::stl::float64_t(dt_precise * Tprecision::num) / Tprecision::den;
			last_time = now;
			return *this;
		}

		inline time& increment_frame() {
			++frame;
			return *this;
		}

		template<typename Tprecision = std::micro, typename Tclock = std::chrono::system_clock>
		inline time& compute_delta_and_frame() {
			compute_delta<Tprecision, Tclock>();
			increment_frame();
			return *this;
		}

		template<typename Tprecision = std::micro, typename Tclock = std::chrono::system_clock>
		inline time& update(float alpha = .9) {
			compute_delta_and_frame<Tprecision, Tclock>();
			total += delta;
			smoothed_delta = alpha * smoothed_delta + (1 - alpha) * delta;
			return *this;
		}
	};

	struct camera {
		virtual stdmath::matrix<float, 4, 4> view_matrix() const = 0;
		virtual stdmath::matrix<float, 4, 4> projection_matrix(const stdmath::vector<size_t, 2>& screen_size) const = 0;

		virtual stdmath::matrix<float, 4, 4> inverse_view_matrix() const {
			return inverse(view_matrix());
		}
		virtual stdmath::matrix<float, 4, 4> inverse_projection_matrix(const stdmath::vector<size_t, 2>& screen_size) const {
			return inverse(projection_matrix(screen_size));
		}
	};

	struct concrete_camera : public camera {
		stdmath::matrix<float, 4, 4> view, projection;
		stdmath::matrix<float, 4, 4> inverse_view, inverse_projection;

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

	struct utility_buffer : public managed_buffer {
		concrete_camera camera;
		struct time time;

		std::span<const std::byte> to_bytes() override {
			// NOTE: concrete camera has a vtable so its uploadable data starts at camera.view
			return {(std::byte*)&camera.view, ((std::byte*)&time) - ((std::byte*)&camera.view) + sizeof(time)};
		}

		utility_buffer& update(context& ctx, float alpha = .9) {
			time.update(alpha);
			upload(ctx, "Stylizer Utility Buffer");
			return *this;
		}
	};

	struct instance_data {
		stdmath::matrix<float, 4, 4> model = stdmath::matrix<float, 4, 4>::identity();

		struct buffer_base : public managed_buffer {
			virtual size_t count() const = 0;

			std::unordered_map<utility_buffer*, api::current_backend::bind_group> group_cache;

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

		template<size_t N = std::dynamic_extent>
		struct buffer : public buffer_base, public std::array<instance_data, N> {
			using base = instance_data::buffer_base;

			size_t count() const override { return N; }

			std::span<const std::byte> to_bytes() override {
				return byte_span<instance_data>(static_cast<std::array<instance_data, N>&>(*this));
			}
		};
	};

	template<>
	struct instance_data::buffer<std::dynamic_extent> : public instance_data::buffer_base, public std::vector<instance_data> {
		using base = instance_data::buffer_base;

		size_t count() const override { return std::vector<instance_data>::size(); }

		std::span<const std::byte> to_bytes() override {
			return byte_span<instance_data>(static_cast<std::vector<instance_data>&>(*this));
		}
	};

}