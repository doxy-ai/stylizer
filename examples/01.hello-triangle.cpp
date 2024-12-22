#include "stylizer/core/core.hpp"
#include "stylizer/obj/obj.hpp"
#include "stylizer/window/window.hpp"

int main() {
	using namespace stylizer::api::operators;

	stylizer::auto_release window = stylizer::window::create({800, 600});
	stylizer::auto_release context = window.create_context();
	window.reconfigure_surface_on_resize(context, window.determine_optimal_config(context));

	stylizer::auto_release gbuffer = stylizer::gbuffer::create_default(context, window.get_size());
	window.auto_resize_geometry_buffer(context, gbuffer);

	stylizer::auto_release obj = stylizer::obj::load("../examples/resources/tri.obj", context);

	stylizer::auto_release<stylizer::material> material; { // Braced like this so the shader code can be folded away in the editor!
		auto slang = R"_(
import stylizer;
import stylizer_default;

struct FS_Input {
	float4 position : SV_Position;
};

[[shader("vertex")]]
FS_Input vertex(vertex_input input) {
	FS_Input output;
	output.position = float4(input.position.xyz, 1.0);
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
	obj.default_material = &material;

	while(!window.should_close(context)) {
		auto draw = gbuffer.begin_drawing(context, stylizer::float3{.1, .3, .5}); {
			obj.draw(draw);
		} draw.one_shot_submit();

		// try {
			stylizer::auto_release surface_texture = context.get_surface_texture();
			surface_texture.blit_from(context, gbuffer.color);
			context.present();
		// } catch(stylizer::api::surface::texture_acquisition_failed e) {
		// 	std::cerr << e.what() << std::endl;
		// }
	}

	context.release(true);
}