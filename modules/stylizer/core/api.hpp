#pragma once

#include <stylizer/api/api.hpp>
#include <stylizer/api/backends/current_backend.hpp>
#include <reaction/reaction.h>

#include <chrono>
#include <cstddef>
#include <ratio>

namespace stylizer {

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

	struct context : public api::current_backend::device {
		struct event {
			virtual ~event() {}
		};

		stylizer::signal<void(context&)> process_events;
		stylizer::signal<void(const event&)> handle_event;

		void update() {
			process_events(*this);
		}

		static context create_default(const api::device::create_config& config = {}) {
			context out;
			static_cast<api::current_backend::device&>(out) = api::current_backend::device::create_default(config);
			out.process_events.connect([](context& ctx) {
				static_cast<api::current_backend::device&>(ctx).process_events();
			});
			return out;
		}
	};	

	struct time {
		float total = 0, delta = 0, smoothed_delta = 0;
		size_t frame = 0;

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

}