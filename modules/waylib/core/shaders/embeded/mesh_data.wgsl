#pragma once

struct mesh_meta_and_data {
	is_indexed: u32,
	vertex_count: u32,
	triangle_count: u32,

	position_start: u32,
	normals_start: u32,
	tangents_start: u32,
	uvs_start: u32,
	cta_start: u32,

	colors_start: u32,
	bones_start: u32,
	bone_weights_start: u32,

	vertex_buffer_size: u32,
	mesh_data: array<f32>,
}