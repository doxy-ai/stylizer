#pragma once

struct time_gpu_data {
	float since_start = 0; // at byte offset 0
	float delta = 0; // at byte offset 4
	float average_delta = 0; // at byte offset 8
	uint32_t frames = 0;
};

struct camera3D_gpu_data {
	float4x4 view_matrix; // at byte offset 0
	float4x4 projection_matrix; // at byte offset 64
	float4x4 inverse_projection_matrix; // at byte offset 64
	float3 position; // at byte offset 128
	float pad0;
	float3 target_position; // at byte offset 144
	float pad1;
	float3 up = {0, 1, 0}; // at byte offset 160
	// float pad2;
	float field_of_view = 90; // at byte offset 172
	float near_clip_distance = .01; // at byte offset 176
	float far_clip_distance = 1000; // at byte offset 180
	api::bool32 orthographic = false; // at byte offset 184
	// float _pad2;
};

struct camera2D_gpu_data {
	float4x4 view_matrix; // at byte offset 0
	float4x4 projection_matrix; // at byte offset 64
	float4x4 inverse_projection_matrix; // at byte offset 64
	float3 offset = {0, 0, 0}; // at byte offset 128
	float pad0;
	float3 target_position = {0, 0, 1}; // at byte offset 144
	// float pad1;
	float rotation = 0; // at byte offset 156
	float near_clip_distance = .01; // at byte offset 160
	float far_clip_distance = 10; // at byte offset 164
	float zoom = 1; // at byte offset 168
	uint32_t padding0; // at byte offset 172
	uint32_t padding1; // at byte offset 176
	uint32_t padding2; // at byte offset 180
	uint32_t padding3; // at byte offset 184
	// uint32_t padding3; // at byte offset 184
};

static_assert(sizeof(camera3D_gpu_data) == sizeof(camera2D_gpu_data), "The two camera classes need to have the same size in bytes");

enum class light_type {
	Ambient = 0,
	Directional,
	Point,
	Spot,
	Force32 = 0x7FFFFFFF
};

struct light_gpu_data {
	float3 position; // at byte offset 32
	float pad0;
	float3 direction; // at byte offset 48
	float pad1;

	float4 color; // at byte offset 64
	float intensity; // at byte offset 4

	float cutoff_start_angle; // at byte offset 8
	float cutoff_end_angle; // at byte offset 12
	float falloff; // at byte offset 16

	enum light_type light_type; // at byte offset 0
	float _pad0[3];
	
	// float _pad1[5];
};

#ifdef IS_STYLIZER_CORE_CPP
size_t utility_buffer_light_offset = sizeof(time_gpu_data) + sizeof(camera3D_gpu_data);
size_t minimum_utility_buffer_size = utility_buffer_light_offset + sizeof(light_gpu_data);
#else 
extern const size_t minimum_utility_buffer_size;
extern const size_t utility_buffer_light_offset;
#endif