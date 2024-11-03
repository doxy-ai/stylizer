#pragma once

// TODO: Does it make more sense for this to go in its own file?
struct instance_data {
	transform: mat4x4f,
	tint: vec4f,
}

struct vertex_input {
	@location(0) position: vec4f,
	@location(1) normal: vec4f,
	@location(2) tangent: vec4f,
	@location(3) uv: vec4f,
	@location(4) cta: vec4f,
	@location(5) color: vec4f,
	@location(6) bones: vec4u,
	@location(7) bone_weights: vec4f,
}

struct vertex_output {
	@builtin(position) builtin_position: vec4f,
	@location(0) position: vec3f,
	@location(1) mask1: f32,
	@location(2) normal: vec3f,
	@location(3) mask2: f32,
	@location(4) tangent: vec3f,
	@location(5) bitangent: vec3f,
	@location(6) uv: vec2f,
	@location(7) uv2: vec2f,
	@location(8) curvature: f32,
	@location(9) thickness: f32,
	@location(10) @interpolate(flat) adjacency: vec2<u32>,
	@location(11) color: vec4f,
	@location(12) @interpolate(flat) bones: vec4u, // TODO: Does fragment shader need to know about bones?
	@location(13) bone_weights: vec4f,
}

fn unpack_vertex_input(in: vertex_input) -> vertex_output {
	return vertex_output(
		vec4f(in.position.xyz, 1), // builtin_position
		in.position.xyz, // position
		in.position.w, // mask1
		in.normal.xyz, // normal
		in.normal.w, // mask2
		in.tangent.xyz, // tangent
		vec3f(in.tangent.w), // bitangent
		in.uv.xy, // uv
		in.uv.zw, // uv2
		in.cta.x, // curvature
		in.cta.y, // thickness
		bitcast<vec2<u32>>(in.cta.zw), // adjacency
		in.color, // color
		in.bones, // bones
		in.bone_weights, // bone_weights
	);
}

fn vertex_output_apply_model_view_projection_matrix(in: vertex_output, model_matrix: mat4x4f, view_projection_matrix: mat4x4f) -> vertex_output {
	var out = in;
	let world_position = model_matrix * in.builtin_position;
	out.builtin_position = view_projection_matrix * world_position;
	out.position = world_position.xyz;
	out.normal = normalize(model_matrix * vec4f(in.normal, 0)).xyz;
	return out;
}

fn vertex_output_generate_bitangent(in: vertex_output) -> vertex_output {
	var out = in;
	out.bitangent = out.bitangent.x * cross(out.normal, out.tangent);
	return out;
}

fn unpack_vertex_input_full(in: vertex_input, model_matrix: mat4x4f, view_projection_matrix: mat4x4f) -> vertex_output {
	return vertex_output_generate_bitangent(
		vertex_output_apply_model_view_projection_matrix(unpack_vertex_input(in), model_matrix, view_projection_matrix)
	);
}

fn normal_mapping(in: vertex_output, tangent_space_normal: vec3f) -> vec3f {
	let TBN = mat3x3(normalize(in.tangent), normalize(in.bitangent), normalize(in.normal));
	return TBN * tangent_space_normal;
}