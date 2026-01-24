module;

#include "util/move_and_make_owned_macros.hpp"
#include "util/reaction.hpp"

export module stylizer:surface;

import :context;
import :texture;

import stdmath.slang;

namespace stylizer {
	using namespace magic_enum::bitwise_operators;

	export struct surface : public graphics::current_backend::surface { STYLIZER_MOVE_AND_MAKE_OWNED_METHODS(surface)
		using super = graphics::current_backend::surface;

		surface() = default;
		surface(const surface&) = delete;
		surface(surface&& o) { o = std::move(*this); }
		surface& operator=(const surface&) = delete;
		surface& operator=(surface&& o) {
			*(super*)this = std::move((super&)o);
			
			update_as_internal([&]{
				size = std::move(o.size);
				present_mode = std::move(o.present_mode);
				texture_format = std::move(o.texture_format);
				alpha_mode = std::move(o.alpha_mode);
				usage = std::move(o.usage);
				creation_context = std::move(o.creation_context);
			});

			o.reconfigure.close();
			reconfigure = reaction::action([this](
				stdmath::uint2 size, enum present_mode present_mode,
				graphics::texture_format texture_format, graphics::alpha_mode alphas_mode, graphics::usage usage
			) {
				reconfigure_impl(size, present_mode, texture_format, alphas_mode, usage);
			}, size, present_mode, texture_format, alpha_mode, usage);
			return *this;
		}

		reaction::Var<stdmath::uint2> size;
		reaction::Var<enum present_mode> present_mode; //= surface::present_mode::Fifo;
		reaction::Var<graphics::texture_format> texture_format; //= graphics::texture_format::RGBAu8_NormalizedSRGB;
		reaction::Var<graphics::alpha_mode> alpha_mode; //= graphics::alpha_mode::Opaque;
		reaction::Var<graphics::usage> usage; // = graphics::usage::RenderAttachment;

		surface& present(graphics::device& device) override {
			graphics::current_backend::surface::present(device);
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

		void release() override {
			super::release();
			size.close();
			present_mode.close();
			texture_format.close();
			alpha_mode.close();
			usage.close();
			reconfigure.close();
		}

		static surface internal_create(context& ctx, graphics::current_backend::surface& backing_surface, const stdmath::uint2& size) {
			auto config = backing_surface.determine_optimal_default_config(ctx, size);

			surface out;
			static_cast<graphics::current_backend::surface&>(out) = backing_surface;
			out.size = reaction::var(stdmath::uint2{config.size});
			out.present_mode = reaction::var(config.present_mode);
			out.texture_format = reaction::var(config.texture_format);
			out.alpha_mode = reaction::var(config.alpha_mode);
			out.usage = reaction::var(config.usage);
			out.creation_context = &ctx;

			out.reconfigure = reaction::action([self = &out](
				stdmath::uint2 size, enum present_mode present_mode,
				graphics::texture_format texture_format, graphics::alpha_mode alphas_mode, graphics::usage usage
			) {
				self->reconfigure_impl(size, present_mode, texture_format, alphas_mode, usage);
			}, out.size, out.present_mode, out.texture_format, out.alpha_mode, out.usage);

			return out;
		}

	protected:
		context* creation_context;
		bool internal_update = false;

		template<typename Tfunc>
		auto update_as_internal(const Tfunc& func) -> decltype(func()) {
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

		reaction::Action<> reconfigure; void reconfigure_impl(
			stdmath::uint2 size, enum present_mode present_mode,
			graphics::texture_format texture_format, graphics::alpha_mode alphas_mode, graphics::usage usage
		) {
			if(internal_update) return;

			surface::config config;
			config.size = size;
			config.present_mode = present_mode;
			config.texture_format = texture_format;
			config.alpha_mode = alphas_mode;
			config.usage = usage;
			configure(*creation_context, config);
		}

		// Hide some of super's methods
		using super::create_from_emscripten;
		using super::create_from_cocoa;
		using super::create_from_x11;
		using super::create_from_wayland;
		using super::create_from_win32;
		using super::determine_optimal_default_config;
		using super::configured_texture_format;
	};
}