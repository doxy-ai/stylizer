module;

#include "util/move_and_make_owned_macros.hpp"
#include "util/reaction.hpp"

export module stylizer:single_texture_frame_buffer;

import :frame_buffer;

// import stdmath;
// import stdmath.slang;

namespace stylizer {
	using namespace magic_enum::bitwise_operators;

	export struct single_texture_frame_buffer : public frame_buffer { STYLIZER_MOVE_AND_MAKE_OWNED_DERIVED_METHODS(single_texture_frame_buffer, frame_buffer)
		maybe_owned<stylizer::texture> texture;

		static single_texture_frame_buffer create(
			context& ctx, const stdmath::uint3& size,
			const std::optional<stdmath::float4>& clear_value, texture::format color_format,
			texture::create_config config_override = {}, const std::optional<texture::sampler_config>& sampler = texture::sampler_config{}
		) {
			auto config = config_override;
			config.usage |= graphics::usage::RenderAttachment;
			config.format = color_format;
			config.size = size;

			single_texture_frame_buffer out;
			out.size = reaction::var(size);
			out.clear_value = clear_value;
			out.texture = texture::create(ctx, config, sampler).move_to_owned();
			out.texture->size = out.size; // TODO: does this link their reactive state like I hope it does?

			return out;
		}

		static single_texture_frame_buffer create_from_texture(stylizer::texture& texture, const std::optional<stdmath::float4>& clear_value = {}) {
			single_texture_frame_buffer out;
			out.size = texture.size;
			out.clear_value = clear_value;
			out.texture = &texture;
			return out;
		}

		stylizer::texture& color_texture() override { return *texture.value; }

		std::span<const graphics::color_attachment> color_attachments(graphics::color_attachment attachment_template = {}) const override {
			static graphics::color_attachment tmp;
			tmp = attachment_template;
			tmp.clear_value = clear_value;
			tmp.texture = texture.value;
			return span_from_value(tmp);
		}

		std::optional<graphics::depth_stencil_attachment> depth_stencil_attachment(graphics::depth_stencil_attachment attachment_template = {}) const override {
			return {};
		}

	};

}