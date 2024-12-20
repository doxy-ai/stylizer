#include "stylizer/core/core.hpp"
#include "stylizer/window/window.hpp"

int main() {
	stylizer::auto_release window = stylizer::window::create({800, 600});
	stylizer::auto_release context = window.create_context();
	window.reconfigure_surface_on_resize(context, window.determine_optimal_config(context));

	stylizer::auto_release gbuffer = stylizer::gbuffer::create_default(context, window.get_size());
	window.auto_resize_geometry_buffer(context, gbuffer);

	stylizer::auto_release<stylizer::material> material; { // Braced like this so the shader code can be folded away in the editor!
		auto slang = R"_(
import stylizer;
import stylizer_default;

// Vertex Shader
struct VS_Input {
	uint vertexIndex : SV_VertexID; // Vertex index
};

struct FS_Input {
	float4 position : SV_Position;
};

[[shader("vertex")]]
FS_Input vertex(VS_Input input) {
	FS_Input output;

	// Initialize default position
	float2 p = float2(0.0, 0.0);

	// Set specific positions based on vertex index
	if (input.vertexIndex == 0) {
		p = float2(-0.5, -0.5);
	} else if (input.vertexIndex == 1) {
		p = float2(0.5, -0.5);
	} else {
		p = float2(0.0, 0.5);
	}

	// Set the position in homogeneous coordinates
	output.position = float4(p, 0.0, 1.0);

	return output;
}

[[shader("fragment")]]
fragment_output fragment() {
	fragment_output output;
	output.color = imported_color;
	return output;
})_";
		material = stylizer::material::create_from_source_for_geometry_buffer(context, slang, {
			{stylizer::api::shader::stage::Vertex, "vertex"},
			{stylizer::api::shader::stage::Fragment, "fragment"},
		}, gbuffer);
	}

	while(!window.should_close(context)) {
		gbuffer.begin_drawing(context, stylizer::float3{.1, .3, .5})
			.bind_render_pipeline(context, material.pipeline)
			.draw(context, 3)
			.one_shot_submit(context);

		// try {
			context.get_surface_texture().blit_from(context, gbuffer.color);
			context.present();
		// } catch(stylizer::api::surface::texture_acquisition_failed e) {
		// 	std::cerr << e.what() << std::endl;
		// }
	}

	context.release(true);
}