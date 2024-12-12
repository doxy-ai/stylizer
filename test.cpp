#include "stylizer/core/core.hpp"
#include "stylizer/window/window.hpp"

int main() {
	stylizer::auto_release window = stylizer::window::create({800, 600});
	stylizer::auto_release context = window.create_context();
	window.reconfigure_surface_on_resize(context, window.determine_optimal_config(context));

	stylizer::auto_release shader = stylizer::api::webgpu::shader::create_from_source(context, stylizer::api::shader::language::WGSL, R"(
@vertex
fn vertex(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4f {
	var p = vec2f(0.0, 0.0);
	if (in_vertex_index == 0u) {
		p = vec2f(-0.5, -0.5);
	} else if (in_vertex_index == 1u) {
		p = vec2f(0.5, -0.5);
	} else {
		p = vec2f(0.0, 0.5);
	}
	return vec4f(p, 0.0, 1.0);
}

@fragment
fn fragment() -> @location(0) vec4f {
	return vec4f(1.0, 0.4, 1.0, 1.0);
})");

	std::vector<stylizer::api::color_attachment> color_attachments = {{
		.texture_format = stylizer::api::texture::format::BGRA8_SRGB,
		.clear_value = {{.3, .5, 1}},
	}};
	stylizer::auto_release pipeline = stylizer::api::webgpu::render_pipeline::create(
		context,
		{
			{stylizer::api::shader::stage::Vertex, {&shader, "vertex"}},
			{stylizer::api::shader::stage::Fragment, {&shader, "fragment"}}
		},
		color_attachments
	);

	while(!window.should_close(context)) {
		// try {
			stylizer::auto_release texture = context.surface.next_texture(context);
			color_attachments[0].texture = &texture;
			context.device.create_render_pass(color_attachments, {}, true)
				.bind_render_pipeline(context, pipeline)
				.draw(context, 3)
				.one_shot_submit(context);
			context.surface.present(context);
		// } catch(stylizer::api::surface::texture_acquisition_failed fail) {
		// 	std::cerr << fail.what() << std::endl;
		// }
	}

	context.release(true);
}