module;

#include "util/move_and_make_owned_macros.hpp"
#include "battery/embed.hpp"

// export module stylizer:flat_material;
export module stylizer.flat_material;

// import :material;
import std.compat;
import stdmath.slang;
import stylizer;

namespace stylizer {

    export struct flat_material : public stylizer::material { STYLIZER_MOVE_AND_MAKE_OWNED_DERIVED_METHODS(flat_material, stylizer::material)
		using color_t = std::variant<stdmath::float4, stylizer::maybe_owned<stylizer::texture>>;
		color_t color;
		struct config {
			stdmath::float4 color;
			uint32_t use_texture = false; 
			std::array<float, 3> padding; // TODO: Why does slang align this so large!?!?
			enum srgb_state : uint32_t {
				InputLinearOutputLinear = 0,
				InputLinearOutputSRGB = (1 << 1),
				InputSRGBOutputLinear = (1 << 0),
				InputSRGBOutputSRGB = (1 << 1 | 1 << 0)
			} srgb;
		} config;

		stylizer::graphics::current_backend::shader vertex, fragment;
		stylizer::graphics::current_backend::buffer config_buffer;
		stylizer::graphics::current_backend::bind_group group;

		flat_material& create_from_configured(stylizer::context& ctx, const stylizer::frame_buffer& fb) {
			config_buffer = ctx.create_and_write_buffer(stylizer::graphics::usage::Storage, stylizer::byte_span<struct config>(stylizer::span_from_value(config)));

			auto source = b::embed<"shaders/flat_material.slang">().str();
			vertex = ctx.create_shader_from_source(
				stylizer::graphics::shader::language::Slang,
				stylizer::graphics::shader::stage::Vertex,
				source, "vertex"
			);
			fragment = ctx.create_shader_from_source(
				stylizer::graphics::shader::language::Slang,
				stylizer::graphics::shader::stage::Fragment,
				source, "fragment"
			);

			static_cast<stylizer::graphics::current_backend::render::pipeline&>(*this) = ctx.create_render_pipeline({
				{stylizer::graphics::shader::stage::Vertex, {&vertex, "vertex"}},
				{stylizer::graphics::shader::stage::Fragment, {&fragment, "fragment"}}
			}, fb.color_attachments(), fb.depth_stencil_attachment(), {
				.vertex_buffers = {
					{.attributes = {{.format = stylizer::graphics::render::pipeline::config::vertex_buffer_layout::attribute::format::f32x3}}}, // positions
					{.attributes = {{.format = stylizer::graphics::render::pipeline::config::vertex_buffer_layout::attribute::format::f32x2}}} // uvs
				}
			});

			return *this;
		}
		static flat_material create(stylizer::context& ctx, const stylizer::frame_buffer& fb, const color_t& initial_color = stdmath::float4{.5, .5, .5, 1}) {
			flat_material out;
			out.color = std::move((color_t&)initial_color);
			bool input_srgb = true;
			if(std::holds_alternative<stdmath::float4>(out.color))
				out.config.color = std::get<stdmath::float4>(out.color);
			else {
				out.config.use_texture = true;
				input_srgb = stylizer::graphics::is_srgb(std::get<stylizer::maybe_owned<stylizer::texture>>(out.color)->texture_format());
			}

			bool output_srgb = is_srgb(fb.color_texture().texture_format());
			((uint32_t&)out.config.srgb) = (input_srgb << 0) | (output_srgb << 1);

			return std::move(out.create_from_configured(ctx, fb));
		}

		std::span<stylizer::maybe_owned<stylizer::graphics::current_backend::texture>> textures(stylizer::context& ctx) override {
			if(std::holds_alternative<stylizer::maybe_owned<stylizer::texture>>(color)) {
				auto& unsafe = (stylizer::maybe_owned<stylizer::graphics::current_backend::texture>&)std::get<stylizer::maybe_owned<stylizer::texture>>(color);
				return span_from_value(unsafe);
			}
			return stylizer::maybe_owned_span(stylizer::span_from_value<stylizer::graphics::current_backend::texture>(stylizer::texture::get_default_texture(ctx)));
		}

		std::span<stylizer::maybe_owned<stylizer::graphics::current_backend::buffer>> buffers(stylizer::context& ctx) override {
			return stylizer::maybe_owned_span(stylizer::span_from_value<stylizer::graphics::current_backend::buffer>(config_buffer));
		}

		std::span<std::string_view> requested_mesh_attributes() override {
			static std::array<std::string_view, 2> attributes{
				stylizer::common_mesh_attributes::positions,
				stylizer::common_mesh_attributes::uvs
			};
			return attributes;
		}

		std::span<stylizer::graphics::current_backend::bind_group> bind_groups(stylizer::context& ctx) override {
			if(group) return stylizer::span_from_value(group);

			auto t = textures(ctx);
			group = create_bind_group(ctx, 1, std::array<stylizer::graphics::bind_group::binding, 2> {
				stylizer::graphics::bind_group::texture_binding(&*t[0]),
				stylizer::graphics::bind_group::buffer_binding(&config_buffer)
			});
			return stylizer::span_from_value(group);
		}

		stylizer::material& upload(stylizer::context& ctx) override {
			config_buffer.release();
			config_buffer = ctx.create_and_write_buffer(stylizer::graphics::usage::Storage, stylizer::byte_span<struct config>(stylizer::span_from_value(config)));

			group.release();
			group = {};
			bind_groups(ctx);
			return *this;
		}
	};
}