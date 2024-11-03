#pragma once

struct frame_time_data {
	since_start: f32,
	delta: f32,
	average_delta: f32,
};

struct camera3D_data {
	view_matrix: mat4x4f,
	projection_matrix: mat4x4f,
	position: vec3f,
	target_position: vec3f,
	up: vec3f,
	field_of_view: f32,
	near_clip_distance: f32,
	far_clip_distance: f32,
	orthographic: u32,
};

struct camera2D_data {
	view_matrix: mat4x4f,
	projection_matrix: mat4x4f,
	offset: vec3f,
	target_position: vec3f,
	rotation: f32,
	near_clip_distance: f32,
	far_clip_distance: f32,
	zoom: f32,
	pixel_perfect: u32,
	padding0: u32,
	padding1: u32,
	padding2: u32
};

struct light_data {
	light_type: u32,
	position: vec3f,
	direction: vec3f,

	color: vec4f,
	intensity: f32,

	cutoff_start_angle: f32,
	cutoff_end_angle: f32,

	constant: f32,
	linear: f32,
	quadratic: f32,
};

#ifdef STYLIZER_CAMERA_DATA_IS_3D
	struct utility_data {
		time: frame_time_data,
		camera: camera3D_data,
		lights: array<light_data>,
	}

	fn camera_view_projection_matrix(camera: camera3D_data) -> mat4x4f {
		return camera.projection_matrix * camera.view_matrix;
	}
#else
	struct utility_data {
		time: frame_time_data,
		camera: camera2D_data,
		lights: array<light_data>,
	}

	fn camera_view_projection_matrix(camera: camera2D_data) -> mat4x4f {
		return camera.projection_matrix * camera.view_matrix;
	}
#endif
