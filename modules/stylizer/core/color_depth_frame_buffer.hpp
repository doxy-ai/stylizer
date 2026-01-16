#pragma once

#include "api.hpp"
#include "util/spans.hpp"

namespace stylizer {

	struct color_depth_frame_buffer : public frame_buffer {
		texture color, depth;

		static color_depth_frame_buffer create(context& ctx, const stdmath::vector<size_t, 3>& size, const std::optional<stdmath::vector<float, 4>>& clear_value, api::texture::format color_format, api::texture::format depth_format = api::texture::format::Depth_u24, api::texture::create_config config_override = {}) {
			auto config = config_override;
			config.usage |= api::usage::RenderAttachment;
			config.format = color_format;
			config.size = size;
			api::texture::sampler_config sample;

			color_depth_frame_buffer out;
			out.size = reaction::var(size);
			out.clear_value = clear_value;
			out.color = texture::create(ctx, config);
			out.color.configure_sampler(ctx, sample);
			out.color.size = out.size; // TODO: does this link their reactive state like I hope it does?

			config.format = depth_format;
			out.depth = texture::create(ctx, config);
			out.depth.configure_sampler(ctx, sample);
			// sample.depth_comparison_function = api::comparison_function::
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