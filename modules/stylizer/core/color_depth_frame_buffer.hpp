#pragma once

#include "api.hpp"
#include "util/spans.hpp"

namespace stylizer {

	struct color_depth_frame_buffer : public frame_buffer { STYLIZER_MOVE_AND_MAKE_OWNED_DERIVED_METHODS(color_depth_frame_buffer, frame_buffer)
		texture color, depth;

		static color_depth_frame_buffer create(
			context& ctx, const stdmath::uint3& size, const std::optional<stdmath::float4>& clear_value, 
			texture::format color_format, texture::format depth_format = texture::format::Depth_u24, 
			const texture::create_config& config_override = {}, const texture::sampler_config& sampler = {}) 
		{
			auto config = config_override;
			config.usage |= api::usage::RenderAttachment;
			config.format = color_format;
			config.size = size;

			color_depth_frame_buffer out;
			out.size = reaction::var(size);
			out.clear_value = clear_value;
			out.color = texture::create(ctx, config, sampler);
			out.color.size = out.size; // TODO: does this link their reactive state like I hope it does?

			config.format = depth_format;
			out.depth = texture::create(ctx, config, sampler);
			out.depth.size = out.size;

			return out;
		}

		texture& color_texture() override { return color; }

		std::span<const api::color_attachment> color_attachments(api::color_attachment attachment_template = {}) const override {
			static api::color_attachment tmp;
			tmp = attachment_template;
			tmp.clear_value = clear_value;
			tmp.texture = (texture*)&color;
			return span_from_value(tmp);
		}

		std::optional<api::depth_stencil_attachment> depth_stencil_attachment(api::depth_stencil_attachment attachment_template = {}) const override {
			attachment_template.texture = (texture*)&depth;
			return attachment_template;
		}

	};

}