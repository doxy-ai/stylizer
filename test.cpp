#include "stylizer/core/core.hpp"
#include "stylizer/img/img.hpp"
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
import stylizer_core;
using namespace stylizer::math;

[[shader("vertex")]]
stylizer::vertex_to_fragment vertex(stylizer::vertex_input input, uint vertex_id : SV_VertexID, uint instance_id : SV_InstanceID) {
	return input.unpack_full(vertex_id, instance_id, identity_matrix, identity_matrix);
}

[[vk::binding(0)]] Texture2D texture;
[[vk::binding(1)]] SamplerState sample;

[[shader("fragment")]]
stylizer::fragment_output fragment(stylizer::vertex_to_fragment input) {
	return {texture.Sample(sample, input.uv)};
})_";
		material = stylizer::material::create_from_source_for_geometry_buffer(context, slang, {
			{stylizer::api::shader::stage::Vertex, "vertex"},
			{stylizer::api::shader::stage::Fragment, "fragment"},
		}, gbuffer);
	}
	material.textures.emplace_back(stylizer::img::load("../examples/resources/test.hdr")
		.upload(context, {.usage = stylizer::api::usage::TextureBinding | stylizer::api::usage::CopySource})
		.generate_mipmaps(context)
		.configure_sampler(context /*, {.magnify_linear = true, .minify_linear = true, .mip_linear = true} */) // Trilinear
		.move()
	);
	obj.default_material = &material;

	stylizer::auto_release<stylizer::material> skyMat; { // Braced like this so the shader code can be folded away in the editor!
		auto slang = R"_(
import stylizer;
import stylizer_core;
using namespace stylizer::math;

[[shader("vertex")]]
stylizer::vertex_to_fragment vertex(stylizer::vertex_input input, uint vertex_id : SV_VertexID, uint instance_id : SV_InstanceID) {
	return input.unpack_full(vertex_id, instance_id, identity_matrix, identity_matrix);
}

[[vk::binding(0)]] Texture2D texture;
[[vk::binding(1)]] SamplerState sample;

[[shader("fragment")]]
stylizer::fragment_output fragment(stylizer::vertex_to_fragment input) {
	return {texture.Sample(sample, input.uv)};
})_";
		skyMat = stylizer::material::create_from_source_for_geometry_buffer(context, slang, {
			{stylizer::api::shader::stage::Vertex, "vertex"},
			{stylizer::api::shader::stage::Fragment, "fragment"},
		}, gbuffer);
	}
	auto skyplane = stylizer::core::model::fullscreen(context, skyMat);

	stylizer::time time;
	stylizer::auto_release utility_buffer = time.update_utility_buffer(context);
	while(!window.should_close(context)) {
		utility_buffer = time.calculate().update_utility_buffer(context, utility_buffer);

		auto draw = gbuffer.begin_drawing(context, stylizer::float3{.1, .3, .5}); {
			// skyplane.draw(draw);
			obj.draw(draw);
		} draw.one_shot_submit();

		context.present(gbuffer.color);
	}

	context.release(true);
}