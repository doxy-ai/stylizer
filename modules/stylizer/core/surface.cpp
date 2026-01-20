#include "api.hpp"

namespace stylizer {

	surface& surface::operator=(surface&& o) {
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
			api::texture_format texture_format, api::alpha_mode alphas_mode, api::usage usage
		) {
			reconfigure_impl(size, present_mode, texture_format, alphas_mode, usage);
		}, size, present_mode, texture_format, alpha_mode, usage);
		return *this;
	}

	surface surface::create(context& ctx, api::current_backend::surface& backing_surface, const stdmath::uint2& size) {
		auto config = backing_surface.determine_optimal_default_config(ctx, size);

		surface out;
		static_cast<api::current_backend::surface&>(out) = backing_surface;
		out.size = reaction::var(stdmath::uint2{config.size});
		out.present_mode = reaction::var(config.present_mode);
		out.texture_format = reaction::var(config.texture_format);
		out.alpha_mode = reaction::var(config.alpha_mode);
		out.usage = reaction::var(config.usage);
		out.creation_context = &ctx;

		out.reconfigure = reaction::action([self = &out](
			stdmath::uint2 size, enum present_mode present_mode,
			api::texture_format texture_format, api::alpha_mode alphas_mode, api::usage usage
		) {
			self->reconfigure_impl(size, present_mode, texture_format, alphas_mode, usage);
		}, out.size, out.present_mode, out.texture_format, out.alpha_mode, out.usage);

		return out;
	}

	void surface::reconfigure_impl(
		stdmath::uint2 size, enum present_mode present_mode,
		api::texture_format texture_format, api::alpha_mode alphas_mode, api::usage usage
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
}