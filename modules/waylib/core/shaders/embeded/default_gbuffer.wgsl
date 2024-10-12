#pragma once

#include <waylib/mesh_data>
#include <waylib/utility_data>
#include <waylib/vertex_data>
#include <waylib/default_gbuffer_data>

@group(0) @binding(0) var<storage, read> utility : utility_data;

@group(1) @binding(0) var<storage, read> mesh : mesh_meta_and_data;
@group(1) @binding(1) var<storage, read> mesh_indicies : array<u32>;
@group(1) @binding(2) var<storage, read> instances : array<instance_data>;

fn ensure_pipeline_layout() {
	var trash = mesh.vertex_count;
	trash = mesh_indicies[0];
	var trashF = utility.time.delta;
	trashF = instances[0].tint.r;
}