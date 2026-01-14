#include "api.hpp"

namespace stylizer {

    surface surface::create(context& ctx, api::current_backend::surface& surface, const stdmath::vector<size_t, 2>& size) {
        auto config = surface.determine_optimal_default_config(ctx, size);
        
        struct surface out;
        static_cast<api::current_backend::surface&>(out) = surface;
        out.size = reaction::var(config.size);
        out.present_mode = reaction::var(config.present_mode);
        out.texture_format = reaction::var(config.texture_format);
        out.alpha_mode = reaction::var(config.alpha_mode);
        out.usage = reaction::var(config.usage);
        out.usage = reaction::var(config.usage);
        out.associated_context = reaction::var(&ctx);
        
        out.resize = reaction::action([surface = static_cast<api::current_backend::surface&>(out)](
            stdmath::vector<size_t, 2> size, enum present_mode present_mode,
            api::texture_format texture_format, api::alpha_mode alphas_mode, api::usage usage,
            context* associated_context
        ) {
            auto& unsafe = const_cast<api::current_backend::surface&>(surface);
            surface::config config;
            config.size = size;
            config.present_mode = present_mode;
            config.texture_format = texture_format;
            config.alpha_mode = alphas_mode;
            config.usage = usage;
            unsafe.configure(*associated_context, config);
        }, out.size, out.present_mode, out.texture_format, out.alpha_mode, out.usage, out.associated_context);

        return out;
    }
}