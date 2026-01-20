#include "flat_material.hpp"

namespace stylizer {

	flat_material& flat_material::create_from_configured(context& ctx, const frame_buffer& fb) {
		constexpr static std::string_view source = R"(
struct time {
	float total = 0, delta = 0, smoothed_delta = 0;
	uint frame = 0;
}

struct camera {
	float4x4 view, projection;
	float4x4 inverse_view, inverse_projection;
}

struct utility_buffer {
	camera camera;
	time time;
}

struct instance_data {
	float4x4 model;
}

[[vk::binding(0, 0)]] StructuredBuffer<instance_data> instances;
[[vk::binding(1, 0)]] StructuredBuffer<utility_buffer> utils;

struct config {
	float4 color;
	uint use_texture_;
	enum srgb_state : uint {
		InputLinearOutputLinear = 0,
		InputLinearOutputSRGB = (1 << 1),
		InputSRGBOutputLinear = (1 << 0),
		InputSRGBOutputSRGB = (1 << 1 | 1 << 0)
	} srgb;

	bool use_texture() { return use_texture_ != 0; }
	bool input_srgb() { return (srgb & (1 << 0)) > 0; }
	bool output_srgb() { return (srgb & (1 << 1)) > 0; }
}

[[vk::binding(0, 1)]] Texture2D<float4> texture;
[[vk::binding(1, 1)]] SamplerState sampler;
[[vk::binding(2, 1)]] StructuredBuffer<config> configs;

struct vertex_input {
	float3 pos : POSITIONS;
	float4 uv : UVS;
}

struct varryings {
	float4 pos : SV_Position;
	float2 uv : UVS;
}

[shader("vertex")]
varryings vertex(vertex_input vert, uint instance_id : SV_InstanceID) {
	var util = utils[0];
	var instance = instances[instance_id];
	var MVP = mul(util.camera.projection, mul(util.camera.view, instance.model));
	return { mul(MVP, float4(vert.pos, 1.0)), float2(vert.uv.x, 1 - vert.uv.y) }; // TODO: Why is the v axis inverted!?!?
}

float4 srgb_to_linear(float4 c) {
	return pow(c, 2.2);
}

float4 linear_to_srgb(float4 c) {
	return pow(c, 1 / 2.2);
}

float4 normalize_srgb(float4 c, bool input_srgb, bool output_srgb) {
	if(input_srgb == output_srgb) return c;
	float4 linear = c;
	if(input_srgb) linear = srgb_to_linear(linear);
	if(output_srgb) return linear_to_srgb(linear);
	return linear;
}

[shader("fragment")]
float4 fragment(varryings vert) : SV_Target {
	var sample = texture.Sample(sampler, vert.uv);
	var color = configs[0].color;
	if(configs[0].use_texture())
		color = sample;
	
	return normalize_srgb(color, configs[0].input_srgb(), configs[0].output_srgb());
	// return float4(231.0/255, 39.0/255, 37.0/255, 1.0) + sample * .001 + color * .001;
})";
		config_buffer = ctx.create_and_write_buffer(api::usage::Storage, byte_span<struct config>(span_from_value(config)));

		vertex = ctx.create_shader_from_source(
			api::shader::language::Slang,
			api::shader::stage::Vertex,
			source, "vertex"
		);
		fragment = ctx.create_shader_from_source(
			api::shader::language::Slang,
			api::shader::stage::Fragment,
			source, "fragment"
		);

		static_cast<api::current_backend::render::pipeline&>(*this) = ctx.create_render_pipeline({
			{api::shader::stage::Vertex, {&vertex, "vertex"}},
			{api::shader::stage::Fragment, {&fragment, "fragment"}}
		}, fb.color_attachments(), fb.depth_stencil_attachment(), {
			.vertex_buffers = {
				{.attributes = {{.format = api::render::pipeline::config::attribute_format::f32x3}}}, // positions
				{.attributes = {{.format = api::render::pipeline::config::attribute_format::f32x2}}} // uvs
			}
		});

		return *this;
	}

	flat_material flat_material::create(context& ctx, const frame_buffer& fb, const color_t& initial_color /* = stdmath::float4{.5, .5, .5, 1} */) {
		flat_material out;
		out.color = std::move((color_t&)initial_color);
		bool input_srgb = true;
		if(std::holds_alternative<stdmath::float4>(out.color))
			out.config.color = std::get<stdmath::float4>(out.color);
		else {
			out.config.use_texture = true;
			input_srgb = api::is_srgb(std::get<maybe_owned<texture>>(out.color)->texture_format());
		}

		bool output_srgb = is_srgb(const_cast<frame_buffer&>(fb).color_texture().texture_format());
		((uint32_t&)out.config.srgb) = (input_srgb << 0) | (output_srgb << 1);

		return std::move(out.create_from_configured(ctx, fb));
	}

	std::span<maybe_owned<api::current_backend::texture>> flat_material::textures(context& ctx) {
		if(std::holds_alternative<maybe_owned<texture>>(color)) {
			auto& unsafe = (maybe_owned<api::current_backend::texture>&)std::get<maybe_owned<texture>>(color);
			return span_from_value(unsafe);
		}
		return maybe_owned_span(span_from_value<api::current_backend::texture>(texture::get_default_texture(ctx)));
	}

	std::span<maybe_owned<api::current_backend::buffer>> flat_material::buffers(context& ctx) {
		return maybe_owned_span(span_from_value<api::current_backend::buffer>(config_buffer));
	}

	std::span<std::string_view> flat_material::requested_mesh_attributes() {
		static std::array<std::string_view, 2> attributes{
			common_mesh_attributes::positions,
			common_mesh_attributes::uvs
		};
		return attributes;
	}

	std::span<api::current_backend::bind_group> flat_material::bind_groups(context& ctx) {
		if(group) return span_from_value(group);

		auto t = textures(ctx);
		group = create_bind_group(ctx, 1, std::array<api::bind_group::binding, 2> {
			api::bind_group::texture_binding(&*t[0]),
			api::bind_group::buffer_binding(&config_buffer)
		});
		return span_from_value(group);
	}

	flat_material& flat_material::upload(context& ctx) {
		config_buffer.release();
		config_buffer = ctx.create_and_write_buffer(api::usage::Storage, byte_span<struct config>(span_from_value(config)));

		group.release();
		group = {};
		bind_groups(ctx);
		return *this;
	}

}