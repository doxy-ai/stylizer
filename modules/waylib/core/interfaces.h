#ifndef WAYLIB_INTERFACES_f9641d712fbea9630f1ce0a0613269d7
#define WAYLIB_INTERFACES_f9641d712fbea9630f1ce0a0613269d7

#ifndef __cplusplus
	#include "config.h"
	#include "optional.h"
	#include <stdint.h>

	#include <webgpu/webgpu.h>
#endif

#include "interfaces.manageable.h"

//////////////////////////////////////////////////////////////////////
// # Math
//////////////////////////////////////////////////////////////////////

#ifndef WAYLIB_NO_SCALAR_ALIASES
	typedef uint8_t WAYLIB_PREFIXED(u8);
	typedef uint16_t WAYLIB_PREFIXED(u16);
	typedef uint32_t WAYLIB_PREFIXED(u32);
	typedef uint64_t WAYLIB_PREFIXED(u64);
	typedef int8_t WAYLIB_PREFIXED(i8);
	typedef int16_t WAYLIB_PREFIXED(i16);
	typedef int32_t WAYLIB_PREFIXED(i32);
	typedef int64_t WAYLIB_PREFIXED(i64);
	typedef float WAYLIB_PREFIXED(f32);
	typedef double WAYLIB_PREFIXED(f64);
#endif

typedef uint32_t WAYLIB_PREFIXED(bool32);
typedef uint32_t WAYLIB_PREFIXED(index_t);

typedef struct {
	uint32_t x, y;
} WAYLIB_PREFIXED_C_CPP_TYPE(vec2u, vec2uC);

#ifdef __cplusplus
	inline vec2uC& toC(const vec2u& v) { return *(vec2uC*)&v; }
	inline vec2u& fromC(const vec2uC& v) { return *(vec2u*)&v; }
#endif

typedef struct {
	int32_t x, y;
} WAYLIB_PREFIXED_C_CPP_TYPE(vec2i, vec2iC);

#ifdef __cplusplus
	inline vec2iC& toC(const vec2i& v) { return *(vec2iC*)&v; }
	inline vec2i& fromC(const vec2iC& v) { return *(vec2i*)&v; }
#endif

typedef struct {
	float x, y;
} WAYLIB_PREFIXED_C_CPP_TYPE(vec2f, vec2fC);

#ifdef __cplusplus
	inline vec2fC& toC(const vec2f& v) { return *(vec2fC*)&v; }
	inline vec2f& fromC(const vec2fC& v) { return *(vec2f*)&v; }
#endif

typedef struct {
	uint32_t x, y, z;
} WAYLIB_PREFIXED_C_CPP_TYPE(vec3u, vec3uC);

#ifdef __cplusplus
	inline vec3uC& toC(const vec3u& v) { return *(vec3uC*)&v; }
	inline vec3u& fromC(const vec3uC& v) { return *(vec3u*)&v; }
#endif

typedef struct {
	int32_t x, y, z;
} WAYLIB_PREFIXED_C_CPP_TYPE(vec3i, vec3iC);

#ifdef __cplusplus
	inline vec3iC& toC(const vec3i& v) { return *(vec3iC*)&v; }
	inline vec3i& fromC(const vec3iC& v) { return *(vec3i*)&v; }
#endif

typedef struct {
	float x, y, z;
} WAYLIB_PREFIXED_C_CPP_TYPE(vec3f, vec3fC);

#ifdef __cplusplus
	inline vec3fC& toC(const vec3f& v) { return *(vec3fC*)&v; }
	inline vec3f& fromC(const vec3fC& v) { return *(vec3f*)&v; }
#endif

typedef struct {
	uint32_t x, y, z, w;
} WAYLIB_PREFIXED_C_CPP_TYPE(vec4u, vec4uC);

#ifdef __cplusplus
	inline vec4uC& toC(const vec4u& v) { return *(vec4uC*)&v; }
	inline vec4u& fromC(const vec4uC& v) { return *(vec4u*)&v; }
#endif

typedef struct {
	int32_t x, y, z, w;
} WAYLIB_PREFIXED_C_CPP_TYPE(vec4i, vec4iC);

#ifdef __cplusplus
	inline vec4iC& toC(const vec4i& v) { return *(vec4iC*)&v; }
	inline vec4i& fromC(const vec4iC& v) { return *(vec4i*)&v; }
#endif

typedef struct {
	float x, y, z, w;
} WAYLIB_PREFIXED_C_CPP_TYPE(vec4f, vec4fC);

#ifdef __cplusplus
	inline vec4fC& toC(const vec4f& v) { return *(vec4fC*)&v; }
	inline vec4f& fromC(const vec4fC& v) { return *(vec4f*)&v; }
#endif

typedef WAYLIB_PREFIXED_C_CPP_TYPE(vec4f, vec4fC) WAYLIB_PREFIXED_C_CPP_TYPE(color, colorC);

#ifdef __cplusplus
	inline WGPUColor toWGPU(const colorC& c) { return {c.x, c.y, c.z, c.w}; }
#endif

typedef struct {
	float m0, m4, m8, m12;      // Matrix first row (4 components)
	float m1, m5, m9, m13;      // Matrix second row (4 components)
	float m2, m6, m10, m14;     // Matrix third row (4 components)
	float m3, m7, m11, m15;     // Matrix fourth row (4 components)
} WAYLIB_PREFIXED_C_CPP_TYPE(mat4x4f, mat4x4fC);

#ifdef __cplusplus
	inline mat4x4fC& toC(const mat4x4f& mat) { return *(mat4x4fC*)&mat; }
	inline mat4x4f& fromC(const mat4x4fC& mat) { return *(mat4x4f*)&mat; }
#endif


//////////////////////////////////////////////////////////////////////
// # Opaque Types
//////////////////////////////////////////////////////////////////////


struct WAYLIB_PREFIXED(window);
struct WAYLIB_PREFIXED(shader_preprocessor);
struct WAYLIB_PREFIXED(finalizer_list);


//////////////////////////////////////////////////////////////////////
// # Interfaces
//////////////////////////////////////////////////////////////////////


// WGPU State
typedef struct {
	WAYLIB_C_OR_CPP_TYPE(WGPUInstance, wgpu::Instance) instance;
	WAYLIB_C_OR_CPP_TYPE(WGPUAdapter, wgpu::Adapter) adapter;
	WAYLIB_C_OR_CPP_TYPE(WGPUDevice, wgpu::Device) device;
	WAYLIB_NULLABLE(WAYLIB_C_OR_CPP_TYPE(WGPUSurface, wgpu::Surface)) surface
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	; WAYLIB_C_OR_CPP_TYPE(WGPUTextureFormat, wgpu::TextureFormat) surface_format
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= wgpu::TextureFormat::Undefined
#endif
	;
} WAYLIB_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC);

// Texture
typedef struct {
	// WAYLIB_MANAGEABLE(WAYLIB_NULLABLE(image*)) image;
	WAYLIB_C_OR_CPP_TYPE(WGPUTexture, wgpu::Texture) gpu_data;
	WAYLIB_C_OR_CPP_TYPE(WGPUTextureView, wgpu::TextureView) view;
	WAYLIB_NULLABLE(WAYLIB_C_OR_CPP_TYPE(WGPUSampler, wgpu::Sampler)) sampler
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	;
} WAYLIB_PREFIXED_C_CPP_TYPE(texture, textureC);

// General Purpose GPU Buffer
typedef struct gpu_bufferC {
	size_t size;
	size_t offset;
	WAYLIB_MANAGEABLE(WAYLIB_NULLABLE(uint8_t*)) cpu_data;
	WAYLIB_C_OR_CPP_TYPE(WGPUBuffer, wgpu::Buffer) gpu_data;
	WAYLIB_MANAGEABLE(WAYLIB_NULLABLE(const char*)) label;
} WAYLIB_PREFIXED_C_CPP_TYPE(gpu_buffer, gpu_bufferC);

// Gbuffer
typedef struct {
	WAYLIB_PREFIXED_C_CPP_TYPE(texture, textureC) color, depth, normal;
} WAYLIB_PREFIXED_C_CPP_TYPE(Gbuffer, GbufferC);

// Drawing State
typedef struct {
	WAYLIB_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)* state;
	WAYLIB_NULLABLE(WAYLIB_PREFIXED_C_CPP_TYPE(Gbuffer, GbufferC)*) gbuffer;
	WAYLIB_NULLABLE(WAYLIB_PREFIXED_C_CPP_TYPE(gpu_buffer, gpu_bufferC)*) utility_buffer;
	WAYLIB_C_OR_CPP_TYPE(WGPUCommandEncoder, wgpu::CommandEncoder) pre_encoder, render_encoder;
	WAYLIB_C_OR_CPP_TYPE(WGPURenderPassEncoder, wgpu::RenderPassEncoder) render_pass;
	WAYLIB_PREFIXED(finalizer_list)* finalizers;
} WAYLIB_PREFIXED_C_CPP_TYPE(drawing_state, drawing_stateC);

// Shader
typedef struct {
	WAYLIB_MANAGEABLE(WAYLIB_NULLABLE(const char*)) compute_entry_point;
	WAYLIB_MANAGEABLE(WAYLIB_NULLABLE(const char*)) vertex_entry_point;
	WAYLIB_MANAGEABLE(WAYLIB_NULLABLE(const char*)) fragment_entry_point;
	WAYLIB_C_OR_CPP_TYPE(WGPUShaderModule, wgpu::ShaderModule) module;
} WAYLIB_PREFIXED_C_CPP_TYPE(shader, shaderC);

// Computer
typedef struct {
	index_t buffer_count;
	WAYLIB_MANAGEABLE(WAYLIB_NULLABLE(WAYLIB_PREFIXED_C_CPP_TYPE(gpu_buffer, gpu_bufferC)*)) buffers;
	index_t texture_count;
	WAYLIB_MANAGEABLE(WAYLIB_NULLABLE(WAYLIB_PREFIXED_C_CPP_TYPE(texture, textureC)*)) textures;

	WAYLIB_MANAGEABLE(WAYLIB_PREFIXED_C_CPP_TYPE(shader, shaderC)*) shader;
	WAYLIB_C_OR_CPP_TYPE(WGPUComputePipeline, wgpu::ComputePipeline) pipeline;
} WAYLIB_PREFIXED_C_CPP_TYPE(computer, computerC);

// Material
typedef struct {
	WAYLIB_MANAGEABLE(WAYLIB_PREFIXED_C_CPP_TYPE(shader, shaderC)*) shaders;
	WAYLIB_PREFIXED(index_t) shader_count;
	WAYLIB_C_OR_CPP_TYPE(WGPURenderPipeline, wgpu::RenderPipeline) pipeline;
} WAYLIB_PREFIXED_C_CPP_TYPE(material, materialC);

// Mesh, vertex data
// From: raylib.h
typedef struct {
	WAYLIB_PREFIXED(index_t) triangle_count;	// Number of triangles stored (indexed or not)
	WAYLIB_PREFIXED(index_t) vertex_count;		// Number of vertices stored (length of each of the following arrays)

	// Vertex attributes data
	WAYLIB_MANAGEABLE(WAYLIB_PREFIXED(vec4f)*) positions;						// Vertex position (xyz) mask 1 (w) (shader-location = 0) NOTE: Not nullable
	WAYLIB_MANAGEABLE(WAYLIB_NULLABLE(WAYLIB_PREFIXED(vec4f)*)) normals;		// Vertex normals (xyz) mask 2 (w) (shader-location = 1)
	WAYLIB_MANAGEABLE(WAYLIB_NULLABLE(WAYLIB_PREFIXED(vec4f)*)) tangents;		// Vertex tangents (xyz) and sign (w) to compute bitangent (shader-location = 2)
	WAYLIB_MANAGEABLE(WAYLIB_NULLABLE(WAYLIB_PREFIXED(vec4f)*)) uvs;			// Vertex texture coordinates (xy) and (zw) (shader-location = 3)
	WAYLIB_MANAGEABLE(WAYLIB_NULLABLE(WAYLIB_PREFIXED(vec4f)*)) cta;			// Vertex curvature (x), thickness (y), and associated faces begin (z) and length (z) (shader-location = 4)

	WAYLIB_MANAGEABLE(WAYLIB_NULLABLE(WAYLIB_PREFIXED(WAYLIB_PREFIXED_C_CPP_TYPE(color, colorC))*)) colors;		// Vertex colors (shader-location = 5)
	WAYLIB_MANAGEABLE(WAYLIB_NULLABLE(WAYLIB_PREFIXED(vec4u)*)) bones;			// Bone IDs (shader-location = 6)
	WAYLIB_MANAGEABLE(WAYLIB_NULLABLE(WAYLIB_PREFIXED(vec4f)*)) bone_weights;	// Bone weights associated with each ID (shader-location = 7)

	WAYLIB_MANAGEABLE(WAYLIB_NULLABLE(WAYLIB_PREFIXED(index_t)*)) indices;		// Vertex indices (in case vertex data comes indexed)


	// Animation vertex data
	// vec3f* anim_vertices;      // Animated vertex positions (after bones transformations)
	// vec3f* anim_normals;       // Animated normals (after bones transformations)
	// vec4u* bone_ids;			// Vertex bone ids, max 255 bone ids, up to 4 bones influence by vertex (skinning)
	// vec4f* bone_weights;		// Vertex bone weight, up to 4 bones influence by vertex (skinning)

	WAYLIB_C_OR_CPP_TYPE(WGPUBuffer, wgpu::Buffer) vertex_buffer, transformed_vertex_buffer; // Pointer to the data on the gpu
	WAYLIB_NULLABLE(WAYLIB_C_OR_CPP_TYPE(WGPUBuffer, wgpu::Buffer)) index_buffer, transformed_index_buffer; // Pointer to the index data on the gpu
} WAYLIB_PREFIXED_C_CPP_TYPE(mesh, meshC);

// Model, meshes, materials and animation data
// From: raylib.h
typedef struct {
	WAYLIB_PREFIXED(mat4x4f) transform;			// Local transform matrix

	WAYLIB_PREFIXED(index_t) mesh_count;											// Number of meshes
	WAYLIB_MANAGEABLE(WAYLIB_PREFIXED_C_CPP_TYPE(mesh, meshC)*) meshes;				// Meshes array
	WAYLIB_PREFIXED(index_t) material_count;										// Number of materials
	WAYLIB_MANAGEABLE(WAYLIB_PREFIXED_C_CPP_TYPE(material, materialC)*) materials;	// Materials array
	WAYLIB_MANAGEABLE(WAYLIB_NULLABLE(WAYLIB_PREFIXED(index_t)*)) mesh_materials;	// Mesh to material mapping (when null and )

	// Animation data
	// WAYLIB_PREFIXED(index_t) bone_count;			// Number of bones
	// bone_info* bones;			// Bones information (skeleton)
} WAYLIB_PREFIXED_C_CPP_TYPE(model, modelC);

// Model Instance Data



//////////////////////////////////////////////////////////////////////
// # GPU Synchronized Structures (Generated by https://eliemichel.github.io/WebGPU-AutoLayout/)
//////////////////////////////////////////////////////////////////////


// Mesh Metadata
typedef struct {
	bool32 is_indexed;
	uint32_t vertex_count;
	uint32_t triangle_count;


	uint32_t position_start;
	uint32_t normals_start;
	uint32_t tangents_start;
	uint32_t uvs_start;
	uint32_t cta_start;

	uint32_t colors_start;
	uint32_t bones_start;
	uint32_t bone_weights_start;

	uint32_t vertex_buffer_size;
} WAYLIB_PREFIXED(mesh_metadata);

typedef struct {
	float since_start // at byte offset 0
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= 0
#endif
	; float delta; // at byte offset 4
	float average_delta; // at byte offset 8
	float _pad0;
} WAYLIB_PREFIXED_C_CPP_TYPE(time, timeC);

typedef struct {
	WAYLIB_PREFIXED(mat4x4f) view_matrix; // at byte offset 0
	WAYLIB_PREFIXED(mat4x4f) projection_matrix; // at byte offset 64
	WAYLIB_PREFIXED(vec3f) position; // at byte offset 128
	float _pad0;
	WAYLIB_PREFIXED(vec3f) target_position; // at byte offset 144
	float _pad1;
	WAYLIB_PREFIXED(vec3f) up; // at byte offset 160
	float field_of_view // at byte offset 172
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= 90
#endif
	; float near_clip_distance; // at byte offset 176
	float far_clip_distance // at byte offset 180
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= 1000
#endif
	; uint32_t orthographic; // at byte offset 184
	float _pad2;
} WAYLIB_PREFIXED_C_CPP_TYPE(camera3D, camera3DC);

typedef struct {
	WAYLIB_PREFIXED(mat4x4f) view_matrix; // at byte offset 0
	WAYLIB_PREFIXED(mat4x4f) projection_matrix; // at byte offset 64
	WAYLIB_PREFIXED(vec3f) offset; // at byte offset 128
	float _pad0;
	WAYLIB_PREFIXED(vec3f) target_position; // at byte offset 144
	float rotation; // at byte offset 156
	float near_clip_distance; // at byte offset 160
	float far_clip_distance // at byte offset 164
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= 10
#endif
	; float zoom // at byte offset 168
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= 1
#endif
	; uint32_t pixel_perfect; // at byte offset 172
	uint32_t padding0; // at byte offset 176
	uint32_t padding1; // at byte offset 180
	uint32_t padding2; // at byte offset 184
	float _pad1;
} WAYLIB_PREFIXED_C_CPP_TYPE(camera2D, camera2DC);

enum light_type {
	Ambient = 0,
	Directional,
	Point,
	Spot,
	Force32 = 0x7FFFFFFF
};

typedef struct {
	enum light_type light_type; // at byte offset 0
	float intensity; // at byte offset 4
	float cutoff_start_angle; // at byte offset 8
	float cutoff_end_angle; // at byte offset 12
	float falloff; // at byte offset 16
	float _pad0[3];
	WAYLIB_PREFIXED(vec3f) position; // at byte offset 32
	float _pad1;
	WAYLIB_PREFIXED(vec3f) direction; // at byte offset 48
	float _pad2;
	WAYLIB_PREFIXED_C_CPP_TYPE(color, colorC) color; // at byte offset 64
} WAYLIB_PREFIXED_C_CPP_TYPE(light, lightC);

typedef struct {
	WAYLIB_PREFIXED(mat4x4f) transform;
	WAYLIB_PREFIXED_C_CPP_TYPE(color, colorC) tint
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= {1, 1, 1, 1}
#endif
	;
} WAYLIB_PREFIXED(model_instance);


//////////////////////////////////////////////////////////////////////
// # Core Configuration
//////////////////////////////////////////////////////////////////////


typedef struct {
	WAYLIB_OPTIONAL(WGPUPresentMode) presentation_mode
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= {}
#endif
	; WGPUCompositeAlphaMode alpha_mode
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= wgpu::CompositeAlphaMode::Auto
#endif
	; bool automatic_should_configure_now
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	;
} WAYLIB_PREFIXED(surface_configuration);

typedef struct {
	WGPUTextureFormat color_format
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= wgpu::TextureFormat::RGBA8Unorm
#endif
	; WGPUTextureFormat depth_format
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= wgpu::TextureFormat::Depth24Plus
#endif
	; WGPUTextureFormat normal_format
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= wgpu::TextureFormat::RGBA16Float
#endif
	;
} WAYLIB_PREFIXED(Gbuffer_config);

typedef struct {
	bool remove_comments
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	; bool remove_whitespace
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	; bool support_pragma_once
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; const char* path
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	;
} WAYLIB_PREFIXED(preprocess_shader_config);

typedef struct {
	const char* compute_entry_point
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	; const char* vertex_entry_point
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	; const char* fragment_entry_point
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	; const char* name
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	; shader_preprocessor* preprocessor
	; preprocess_shader_config preprocess_config
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= {}
#endif
	;
} WAYLIB_PREFIXED(create_shader_configuration);

typedef struct {
	bool double_sided // Determines if the material should be culled from the back or if it should be visible from both sides
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	; WAYLIB_OPTIONAL(WGPUCompareFunction) depth_function
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= wgpu::CompareFunction::Less // Disables writing depth if not provided
#endif
	; // TODO: Add stencil support
	shader_preprocessor* preprocessor
#ifdef WAYLIB_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	;
} WAYLIB_PREFIXED(material_configuration);

#endif // WAYLIB_INTERFACES_f9641d712fbea9630f1ce0a0613269d7