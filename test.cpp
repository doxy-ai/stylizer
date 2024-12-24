#include "stylizer/api/api.hpp"
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

	stylizer::auto_release obj = stylizer::obj::load("../examples/resources/suzane_highpoly.obj", context);

	stylizer::auto_release<stylizer::material> material; { // Braced like this so the shader code can be folded away in the editor!
		auto slang = R"_(
import stylizer;
import stylizer_core;
using namespace stylizer::math;

[[shader("vertex")]]
stylizer::vertex_to_fragment vertex(stylizer::core::vertex_input input, uint vertex_id : SV_VertexID, uint instance_id : SV_InstanceID) {
	stylizer::ensure_vertex_layout();
	return input.unpack_full(vertex_id, instance_id, stylizer::instances[0].transform, stylizer::utility.camera3D.view_projection_matrix);
}

[[vk::binding(0, 1)]] Texture2D texture;
[[vk::binding(1, 1)]] SamplerState sample;

[[shader("fragment")]]
stylizer::core::fragment_output fragment(stylizer::vertex_to_fragment input) {
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
	stylizer::ensure_vertex_layout();
	return input.unpack(vertex_id, instance_id); // NOTE: Applying more complex calculations to the vertex breaks the NDC calculations in the fragment shader
}

[[vk::binding(0, 1)]] Texture2D texture;
[[vk::binding(1, 1)]] SamplerState sample;

[[shader("fragment")]]
stylizer::fragment_output_with_depth fragment(stylizer::vertex_to_fragment input) {
	let clipSpacePos = float4(input.position.xy, 1, 1);
	let worldSpacePos = stylizer::utility.camera3D.inverse_view_projection_matrix * clipSpacePos;
	let direction = normalize(worldSpacePos.xyz - stylizer::utility.camera3D.position);

	let longitude = degrees(atan2(direction.z, direction.x)) / 360;
	let latitude = (degrees(acos(direction.y)) + 90) / 360;
	return {texture.Sample(sample, float2(longitude, latitude)), 0};
})_";
		skyMat = stylizer::material::create_from_source_for_geometry_buffer(context, slang, {
			{stylizer::api::shader::stage::Vertex, "vertex"},
			{stylizer::api::shader::stage::Fragment, "fragment"},
		}, gbuffer);
	}
	skyMat.textures.emplace_back(stylizer::img::load("../examples/resources/symmetrical_garden_02_1k.hdr")
		.upload(context, {.usage = stylizer::api::usage::TextureBinding | stylizer::api::usage::CopySource})
		.generate_mipmaps(context)
		.configure_sampler(context, {.magnify_linear = true, .minify_linear = true, .mip_linear = true}) // Trilinear
		.move()
	);
	auto skyplane = stylizer::core::model::fullscreen(context, skyMat);

	stylizer::time time;
	stylizer::camera3D camera = {{.target_position = stylizer::float3(0)}};
	stylizer::auto_release utility_buffer = time.update_utility_buffer(context);
	while(!window.should_close(context)) {
		utility_buffer = time.calculate().update_utility_buffer(context, utility_buffer);
		camera.position = {2 * cos(time.since_start), sin(time.since_start / 4), 2 * sin(time.since_start)};
		utility_buffer = camera.calculate_matrices(window.get_size()).update_utility_buffer(context, utility_buffer);

		auto draw = gbuffer.begin_drawing(context, stylizer::float3{.1, .3, .5}, {}, utility_buffer); {
			obj.draw(draw);

			skyplane.draw(draw);
		} draw.one_shot_submit();

		window.present(context, gbuffer.color);
	}

	context.release(true);
}