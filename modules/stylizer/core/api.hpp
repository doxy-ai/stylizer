#pragma once

#include <stylizer/api/api.hpp>
#include <stylizer/api/backends/current_backend.hpp>

#include <math/slang_names.hpp>

#include "util/maybe_owned.hpp"
#include "util/reactive.hpp"

#include <chrono>
#include <cstddef>
#include <ratio>

namespace stylizer {
	using namespace api::operators;

	#define STYLIZER_MOVE_AND_MAKE_OWNED_BASE_METHODS(type)\
		type&& move() { return std::move(*this); }\
		virtual stylizer::maybe_owned<type> move_to_owned() = 0;
	#define STYLIZER_MOVE_AND_MAKE_OWNED_METHODS(type)\
		type&& move() { return std::move(*this); }\
		virtual stylizer::maybe_owned<type> move_to_owned() {\
			return stylizer::maybe_owned<type>::make_owned_and_move(*this);\
		}
	#define STYLIZER_MOVE_AND_MAKE_OWNED_DERIVED_METHODS(type, lowest_base)\
		type&& move() { return std::move(*this); }\
		stylizer::maybe_owned<lowest_base> move_to_owned() override {\
			return stylizer::maybe_owned<type>::make_owned_and_move(*this).template move_as<lowest_base>();\
		}



	struct context : public api::current_backend::device {
		using super = api::current_backend::device;

		context() {}
		context(const context&) = delete; // Contexts can't be copied!
		context(context&&) = default; // Contexts shouldn't be moved after reactive objects are created from them...
		context& operator=(const context&) = delete;
		context& operator=(context&&) = delete;

		struct event {
			virtual ~event() {}
		};

		signal<void(context&)> process_events;
		signal<void(const event&)> handle_event;

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

		context& update() {
			process_events(*this);
			return *this;
		}

		// TODO: Is there a better name than send?
		void send(error_severity severity, std::string_view message, size_t error_tag = 0) {
			get_error_handler()(severity, message, error_tag);
		}
		void send_error(std::string_view message, size_t error_tag = 0) {
			send(error_severity::Error, message, error_tag);
		}
		void send_warning(std::string_view message, size_t error_tag = 0) {
			send(error_severity::Warning, message, error_tag);
		}
		void send_info(std::string_view message, size_t error_tag = 0) {
			send(error_severity::Info, message, error_tag);
		}
		void send_verbose(std::string_view message, size_t error_tag = 0) {
			send(error_severity::Verbose, message, error_tag);
		}

	protected:
		// Hide some of super's methods
		using super::tick;
	};



	struct texture : public api::current_backend::texture { STYLIZER_MOVE_AND_MAKE_OWNED_METHODS(texture)
		using super = api::current_backend::texture;

		reaction::Var<stdmath::uint3> size;

		texture() = default;
		texture(const texture& o) { *this = o; }
		texture(texture&& o) { *this = std::move(o); }
		texture& operator=(const texture&);
		texture& operator=(texture&&);

		static texture create(context& ctx, const create_config& config = {}, const std::optional<sampler_config>& sampler = {});
		static texture create_and_write(context& ctx, std::span<const std::byte> data, const data_layout& layout, const create_config& config = {}, const std::optional<sampler_config>& sampler = {});

		static texture& get_default_texture(context& ctx);

		texture& configure_sampler(context& ctx, const sampler_config& config = {}) {
			sample_config = config;
			api::current_backend::texture::configure_sampler(ctx, config);
			return *this;
		}

	protected:
		bool internal_update = false;
		create_config config;
		std::optional<sampler_config> sample_config;
		context* creation_context;

		template<typename Tfunc>
		auto update_as_internal(const Tfunc& func) {
			internal_update = true;
			if constexpr (std::is_same_v<decltype(func()), void>) {
				func();
				internal_update = false;
			} else {
				auto out = func();
				internal_update = false;
				return out;
			}
		}

		reaction::Action<> resize; void resize_impl(const stdmath::uint3& size);

		// Hide some of super's methods
		using super::create;
		using super::create_and_write;
		using super::configure_sampler;
	};

	struct surface : public api::current_backend::surface { STYLIZER_MOVE_AND_MAKE_OWNED_METHODS(surface)
		using super = api::current_backend::surface;

		surface() = default;
		surface(const surface&) = delete;
		surface(surface&& o) { o = std::move(*this); }
		surface& operator=(const surface&) = delete;
		surface& operator=(surface&&);

		reaction::Var<stdmath::uint2> size;
		reaction::Var<enum present_mode> present_mode; //= surface::present_mode::Fifo;
		reaction::Var<api::texture_format> texture_format; //= api::texture_format::RGBAu8_NormalizedSRGB;
		reaction::Var<api::alpha_mode> alpha_mode; //= api::alpha_mode::Opaque;
		reaction::Var<api::usage> usage; // = api::usage::RenderAttachment;

		surface& present(api::device& device) override {
			api::current_backend::surface::present(device);
			return *this;
		}

		surface& present(context& ctx, texture& color_texture) {
			stylizer::auto_release texture = next_texture(ctx);
			texture.blit_from(ctx, color_texture);
			return present(ctx);
		}

		reaction::Calc<stdmath::uint3> texture_size() { return reaction::calc([](const stdmath::uint2& size){
			return stdmath::uint3{size, 1};
		}, size); }

		using api::current_backend::surface::release;
		using api::current_backend::surface::auto_release;

		friend struct window;
	protected:
		static surface create(context& ctx, api::current_backend::surface& surface, const stdmath::uint2& size);

		context* creation_context;
		bool internal_update = false;

		template<typename Tfunc>
		auto update_as_internal(const Tfunc& func) {
			internal_update = true;
			if constexpr (std::is_same_v<decltype(func()), void>) {
				func();
				internal_update = false;
			} else {
				auto out = func();
				internal_update = false;
				return out;
			}
		}

		reaction::Action<> reconfigure; void reconfigure_impl(stdmath::uint2 size, enum present_mode present_mode,
			api::texture_format texture_format, api::alpha_mode alphas_mode, api::usage usage);

		// Hide some of super's methods
		using super::create_from_emscripten;
		using super::create_from_cocoa;
		using super::create_from_x11;
		using super::create_from_wayland;
		using super::create_from_win32;
		using super::determine_optimal_default_config;
		using super::configured_texture_format;
	};




	struct frame_buffer { STYLIZER_MOVE_AND_MAKE_OWNED_BASE_METHODS(frame_buffer)
		reaction::Var<stdmath::uint3> size;
		std::optional<stdmath::float4> clear_value = {};

		virtual texture& color_texture() = 0;
		virtual std::span<const api::color_attachment> color_attachments(api::color_attachment attachment_template = {}) const = 0;
		virtual std::optional<api::depth_stencil_attachment> depth_stencil_attachment(api::depth_stencil_attachment attachment_template = {}) const { return {}; }

		virtual api::current_backend::render::pass create_render_pass(context& ctx, api::color_attachment color_template = {}, api::depth_stencil_attachment depth_template = {}, bool one_shot = true) {
			return ctx.create_render_pass(color_attachments(color_template), depth_stencil_attachment(depth_template), one_shot);
		}

		virtual frame_buffer& update_size_debounced(const stdmath::uint3& size, float dt, float time_to_wait = .1);
		frame_buffer& update_size_debounced(const stdmath::uint3& size, struct time& time, float time_to_wait = .1);

		frame_buffer& update_size_from_surface(surface& surface, struct time& time, float time_to_wait = .1) {
			return update_size_debounced(surface.texture_size()(), time, time_to_wait);
		}
		frame_buffer& update_size_from_texture(texture& texture, struct time& time, float time_to_wait = .1) {
			return update_size_debounced(texture.size(), time, time_to_wait);
		}

		virtual reaction::Action<> link_size_to_surface(surface& surface) {
			return reaction::action([this](const stdmath::uint3& size) {
				this->size.value(size);
			}, surface.texture_size());
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




	struct material : public api::current_backend::render::pipeline { STYLIZER_MOVE_AND_MAKE_OWNED_BASE_METHODS(material)
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
	concept material_concept = std::derived_from<T, material> && requires (T t, context ctx, frame_buffer fb) {
		{ T::create(ctx, fb) } -> std::convertible_to<T>;
	};

	inline namespace common_mesh_attributes {
		constexpr static std::string_view positions = "positions";
		constexpr static std::string_view normals = "normals";
		constexpr static std::string_view colors = "colors";
		constexpr static std::string_view vertex_colors = "colors"; // alias to colors
		constexpr static std::string_view uvs = "uvs";
		constexpr static std::string_view texture_coordinates = "uvs"; // alias to uvs
	}



	struct managed_buffer : public api::current_backend::buffer { STYLIZER_MOVE_AND_MAKE_OWNED_BASE_METHODS(managed_buffer)
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
			delta = double(dt_precise * Tprecision::num) / Tprecision::den;
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

	struct camera { STYLIZER_MOVE_AND_MAKE_OWNED_BASE_METHODS(camera)
		virtual stdmath::float4x4 view_matrix() const = 0;
		virtual stdmath::float4x4 projection_matrix(const stdmath::uint2& screen_size) const = 0;

		virtual stdmath::float4x4 inverse_view_matrix() const {
			return inverse(view_matrix());
		}
		virtual stdmath::float4x4 inverse_projection_matrix(const stdmath::uint2& screen_size) const {
			return inverse(projection_matrix(screen_size));
		}
	};

	struct concrete_camera : public camera { STYLIZER_MOVE_AND_MAKE_OWNED_DERIVED_METHODS(concrete_camera, camera)
		stdmath::float4x4 view, projection;
		stdmath::float4x4 inverse_view, inverse_projection;

		static concrete_camera from_camera(const camera& camera, const stdmath::uint2& screen_size) {
			concrete_camera out;
			out.view = camera.view_matrix();
			out.projection = camera.projection_matrix(screen_size);
			out.inverse_view = camera.inverse_view_matrix();
			out.inverse_projection = camera.inverse_projection_matrix(screen_size);
			return out;
		}

		stdmath::float4x4 view_matrix() const override { return view; }
		stdmath::float4x4 projection_matrix(const stdmath::uint2& screen_size = {}) const override { return projection; }
		stdmath::float4x4 inverse_view_matrix() const override { return inverse_view; }
		stdmath::float4x4 inverse_projection_matrix(const stdmath::uint2& screen_size = {}) const override { return inverse_projection; }
	};

	struct utility_buffer : public managed_buffer { STYLIZER_MOVE_AND_MAKE_OWNED_DERIVED_METHODS(utility_buffer, managed_buffer)
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
		stdmath::float4x4 model = stdmath::float4x4::identity();

		struct buffer_base : public managed_buffer {
			std::unordered_map<utility_buffer*, api::current_backend::bind_group> group_cache;

			api::current_backend::bind_group make_bind_group(context& ctx,api::current_backend::render_pipeline& pipeline, std::optional<utility_buffer> util = {}, size_t index = 0, size_t minimum_util_size = 272);

			virtual size_t count() const = 0;

			virtual void rebuild_gpu_caches() {
				group_cache.clear();
			}
		};

		template<size_t N = std::dynamic_extent>
		struct buffer : public buffer_base, public std::array<instance_data, N> { STYLIZER_MOVE_AND_MAKE_OWNED_DERIVED_METHODS(buffer, managed_buffer)
			using base = instance_data::buffer_base;

			size_t count() const override { return N; }

			std::span<const std::byte> to_bytes() override {
				return byte_span<instance_data>(static_cast<std::array<instance_data, N>&>(*this));
			}
		};
	};

	template<>
	struct instance_data::buffer<std::dynamic_extent> : public instance_data::buffer_base, public std::vector<instance_data> { STYLIZER_MOVE_AND_MAKE_OWNED_DERIVED_METHODS(buffer, managed_buffer)
		using base = instance_data::buffer_base;

		size_t count() const override { return std::vector<instance_data>::size(); }

		std::span<const std::byte> to_bytes() override {
			return byte_span<instance_data>(static_cast<std::vector<instance_data>&>(*this));
		}
	};

}