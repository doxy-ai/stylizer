#pragma once

#include "api.hpp"
#include "stylizer/core/util/maybe_owned.hpp"
#include "util/spans.hpp"

namespace stylizer {

	struct single_texture_frame_buffer : public frame_buffer { STYLIZER_MOVE_AND_MAKE_OWNED_DERIVED_METHODS(single_texture_frame_buffer, frame_buffer)
		maybe_owned<texture> texture;

		static single_texture_frame_buffer create(
			context& ctx, const stdmath::vector<uint32_t, 3>& size,
			const std::optional<stdmath::vector<float, 4>>& clear_value, api::texture::format color_format,
			api::texture::create_config config_override = {}, const std::optional<texture::sampler_config>& sampler = {}
		) {
			auto config = config_override;
			config.usage |= api::usage::RenderAttachment;
			config.format = color_format;
			config.size = size;

			single_texture_frame_buffer out;
			out.size = reaction::var(size);
			out.clear_value = clear_value;
			out.texture = maybe_owned<stylizer::texture>::make_owned(texture::create(ctx, config, sampler));
			out.texture->size = out.size; // TODO: does this link their reactive state like I hope it does?

			return out;
		}

		static single_texture_frame_buffer create_from_texture(stylizer::texture& texture, const std::optional<stdmath::vector<float, 4>>& clear_value = {}) {
			single_texture_frame_buffer out;
			out.size = texture.size;
			out.clear_value = clear_value;
			out.texture = &texture;
			return out;
		}

		stylizer::texture& color_texture() override { return *texture; }

		std::span<const api::color_attachment> color_attachments(api::color_attachment attachment_template = {}) const override {
			static api::color_attachment tmp;
			tmp = attachment_template;
			tmp.clear_value = clear_value;
			tmp.texture = (stylizer::texture*)&texture;
			return span_from_value(tmp);
		}

		std::optional<api::depth_stencil_attachment> depth_stencil_attachment(api::depth_stencil_attachment attachment_template = {}) const override {
			return {};
		}

	};

}