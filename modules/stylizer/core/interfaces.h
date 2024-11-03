#ifndef STYLIZER_INTERFACES_f9641d712fbea9630f1ce0a0613269d7
#define STYLIZER_INTERFACES_f9641d712fbea9630f1ce0a0613269d7

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

#ifndef STYLIZER_NO_SCALAR_ALIASES
	typedef uint8_t STYLIZER_PREFIXED(u8);
	typedef uint16_t STYLIZER_PREFIXED(u16);
	typedef uint32_t STYLIZER_PREFIXED(u32);
	typedef uint64_t STYLIZER_PREFIXED(u64);
	typedef int8_t STYLIZER_PREFIXED(i8);
	typedef int16_t STYLIZER_PREFIXED(i16);
	typedef int32_t STYLIZER_PREFIXED(i32);
	typedef int64_t STYLIZER_PREFIXED(i64);
	typedef float STYLIZER_PREFIXED(f32);
	typedef double STYLIZER_PREFIXED(f64);
#endif

typedef uint32_t STYLIZER_PREFIXED(bool32);
typedef uint32_t STYLIZER_PREFIXED(index_t);

typedef struct {
	uint32_t x, y;
} STYLIZER_PREFIXED_C_CPP_TYPE(vec2u, vec2uC);

#ifdef __cplusplus
	inline vec2uC& toC(const vec2u& v) { return *(vec2uC*)&v; }
	inline vec2u& fromC(const vec2uC& v) { return *(vec2u*)&v; }
#endif

typedef struct {
	int32_t x, y;
} STYLIZER_PREFIXED_C_CPP_TYPE(vec2i, vec2iC);

#ifdef __cplusplus
	inline vec2iC& toC(const vec2i& v) { return *(vec2iC*)&v; }
	inline vec2i& fromC(const vec2iC& v) { return *(vec2i*)&v; }
#endif

typedef struct {
	float x, y;
} STYLIZER_PREFIXED_C_CPP_TYPE(vec2f, vec2fC);

#ifdef __cplusplus
	inline vec2fC& toC(const vec2f& v) { return *(vec2fC*)&v; }
	inline vec2f& fromC(const vec2fC& v) { return *(vec2f*)&v; }
#endif

typedef struct {
	uint32_t x, y, z;
} STYLIZER_PREFIXED_C_CPP_TYPE(vec3u, vec3uC);

#ifdef __cplusplus
	inline vec3uC& toC(const vec3u& v) { return *(vec3uC*)&v; }
	inline vec3u& fromC(const vec3uC& v) { return *(vec3u*)&v; }
#endif

typedef struct {
	int32_t x, y, z;
} STYLIZER_PREFIXED_C_CPP_TYPE(vec3i, vec3iC);

#ifdef __cplusplus
	inline vec3iC& toC(const vec3i& v) { return *(vec3iC*)&v; }
	inline vec3i& fromC(const vec3iC& v) { return *(vec3i*)&v; }
#endif

typedef struct {
	float x, y, z;
} STYLIZER_PREFIXED_C_CPP_TYPE(vec3f, vec3fC);

#ifdef __cplusplus
	inline vec3fC& toC(const vec3f& v) { return *(vec3fC*)&v; }
	inline vec3f& fromC(const vec3fC& v) { return *(vec3f*)&v; }
#endif

typedef struct {
	uint32_t x, y, z, w;
} STYLIZER_PREFIXED_C_CPP_TYPE(vec4u, vec4uC);

#ifdef __cplusplus
	inline vec4uC& toC(const vec4u& v) { return *(vec4uC*)&v; }
	inline vec4u& fromC(const vec4uC& v) { return *(vec4u*)&v; }
#endif

typedef struct {
	int32_t x, y, z, w;
} STYLIZER_PREFIXED_C_CPP_TYPE(vec4i, vec4iC);

#ifdef __cplusplus
	inline vec4iC& toC(const vec4i& v) { return *(vec4iC*)&v; }
	inline vec4i& fromC(const vec4iC& v) { return *(vec4i*)&v; }
#endif

typedef struct {
	float x, y, z, w;
} STYLIZER_PREFIXED_C_CPP_TYPE(vec4f, vec4fC);

#ifdef __cplusplus
	inline vec4fC& toC(const vec4f& v) { return *(vec4fC*)&v; }
	inline vec4f& fromC(const vec4fC& v) { return *(vec4f*)&v; }
#endif

typedef STYLIZER_PREFIXED_C_CPP_TYPE(vec4f, vec4fC) STYLIZER_PREFIXED_C_CPP_TYPE(color, colorC);

#ifdef __cplusplus
	inline WGPUColor toWGPU(const colorC& c) { return {c.x, c.y, c.z, c.w}; }
#endif

typedef struct {
	uint8_t r, g, b, a;
} STYLIZER_PREFIXED_C_CPP_TYPE(color8, color8C);

#ifdef __cplusplus
	inline WGPUColor toWGPU(const color8C& c) { return {c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f}; }
#endif

typedef struct {
	float m0, m4, m8, m12;      // Matrix first row (4 components)
	float m1, m5, m9, m13;      // Matrix second row (4 components)
	float m2, m6, m10, m14;     // Matrix third row (4 components)
	float m3, m7, m11, m15;     // Matrix fourth row (4 components)
} STYLIZER_PREFIXED_C_CPP_TYPE(mat4x4f, mat4x4fC);

#ifdef __cplusplus
	inline mat4x4fC& toC(const mat4x4f& mat) { return *(mat4x4fC*)&mat; }
	inline mat4x4f& fromC(const mat4x4fC& mat) { return *(mat4x4f*)&mat; }
#endif


//////////////////////////////////////////////////////////////////////
// # Opaque Types
//////////////////////////////////////////////////////////////////////


struct STYLIZER_PREFIXED(window);
struct STYLIZER_PREFIXED(shader_preprocessor);
struct STYLIZER_PREFIXED(finalizer_list);


//////////////////////////////////////////////////////////////////////
// # Interfaces
//////////////////////////////////////////////////////////////////////


// WGPU State
typedef struct {
	STYLIZER_C_OR_CPP_TYPE(WGPUInstance, wgpu::Instance) instance;
	STYLIZER_C_OR_CPP_TYPE(WGPUAdapter, wgpu::Adapter) adapter;
	STYLIZER_C_OR_CPP_TYPE(WGPUDevice, wgpu::Device) device;
	STYLIZER_NULLABLE(STYLIZER_C_OR_CPP_TYPE(WGPUSurface, wgpu::Surface)) surface
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	; STYLIZER_C_OR_CPP_TYPE(WGPUTextureFormat, wgpu::TextureFormat) surface_format
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= wgpu::TextureFormat::Undefined
#endif
	;
} STYLIZER_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC);

STYLIZER_ENUM image_format {
	C_PREPEND(IMAGE_FORMAT_, Invalid) = 0,
	C_PREPEND(IMAGE_FORMAT_, RGBA),
	C_PREPEND(IMAGE_FORMAT_, RGBA8),
	C_PREPEND(IMAGE_FORMAT_, Gray),
	C_PREPEND(IMAGE_FORMAT_, Gray8),
	// TODO: Support compressed formats
};

// Image (CPU texture data)
typedef struct {
	image_format format;
	union {
		STYLIZER_MANAGEABLE(STYLIZER_PREFIXED_C_CPP_TYPE(color, colorC)*) rgba;
		STYLIZER_MANAGEABLE(STYLIZER_PREFIXED_C_CPP_TYPE(color8, color8C)*) rgba8;
		STYLIZER_MANAGEABLE(float*) gray;
		STYLIZER_MANAGEABLE(uint8_t*) gray8;
		STYLIZER_MANAGEABLE(void*) data
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
			= nullptr
#endif
		;
	};
	STYLIZER_PREFIXED(vec2u) size;
	size_t frames;
} STYLIZER_PREFIXED_C_CPP_TYPE(image, imageC);

// Texture (GPU texture data)
typedef struct {
	STYLIZER_MANAGEABLE(STYLIZER_NULLABLE(STYLIZER_PREFIXED_C_CPP_TYPE(image, imageC)*)) cpu_data;
	STYLIZER_C_OR_CPP_TYPE(WGPUTexture, wgpu::Texture) gpu_data;
	STYLIZER_C_OR_CPP_TYPE(WGPUTextureView, wgpu::TextureView) view;
	STYLIZER_NULLABLE(STYLIZER_C_OR_CPP_TYPE(WGPUSampler, wgpu::Sampler)) sampler
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	;
} STYLIZER_PREFIXED_C_CPP_TYPE(texture, textureC);

// General Purpose GPU Buffer
typedef struct gpu_bufferC {
	size_t size;
	size_t offset;
	STYLIZER_MANAGEABLE(STYLIZER_NULLABLE(uint8_t*)) cpu_data;
	STYLIZER_C_OR_CPP_TYPE(WGPUBuffer, wgpu::Buffer) gpu_data;
	STYLIZER_MANAGEABLE(STYLIZER_NULLABLE(const char*)) label;
} STYLIZER_PREFIXED_C_CPP_TYPE(gpu_buffer, gpu_bufferC);

// Geometry Buffer
typedef struct STYLIZER_PREFIXED_C_CPP_TYPE(geometry_buffer, geometry_bufferC) {
	STYLIZER_NULLABLE(STYLIZER_PREFIXED_C_CPP_TYPE(geometry_buffer, geometry_bufferC)*) previous;
	STYLIZER_PREFIXED_C_CPP_TYPE(texture, textureC) color, depth, normal;
} STYLIZER_PREFIXED_C_CPP_TYPE(geometry_buffer, geometry_bufferC);

// Drawing State
typedef struct {
	STYLIZER_PREFIXED_C_CPP_TYPE(wgpu_state, wgpu_stateC)* state;
	STYLIZER_NULLABLE(STYLIZER_PREFIXED_C_CPP_TYPE(geometry_buffer, geometry_bufferC)*) gbuffer;
	STYLIZER_NULLABLE(STYLIZER_PREFIXED_C_CPP_TYPE(gpu_buffer, gpu_bufferC)*) utility_buffer;
	STYLIZER_C_OR_CPP_TYPE(WGPUCommandEncoder, wgpu::CommandEncoder) pre_encoder, render_encoder;
	STYLIZER_C_OR_CPP_TYPE(WGPURenderPassEncoder, wgpu::RenderPassEncoder) render_pass;
	STYLIZER_PREFIXED(finalizer_list)* finalizers;
} STYLIZER_PREFIXED_C_CPP_TYPE(drawing_state, drawing_stateC);

// Shader
typedef struct {
	STYLIZER_MANAGEABLE(STYLIZER_NULLABLE(const char*)) compute_entry_point;
	STYLIZER_MANAGEABLE(STYLIZER_NULLABLE(const char*)) vertex_entry_point;
	STYLIZER_MANAGEABLE(STYLIZER_NULLABLE(const char*)) fragment_entry_point;
	STYLIZER_C_OR_CPP_TYPE(WGPUShaderModule, wgpu::ShaderModule) module;
} STYLIZER_PREFIXED_C_CPP_TYPE(shader, shaderC);

// Computer
typedef struct {
	index_t buffer_count;
	STYLIZER_MANAGEABLE(STYLIZER_NULLABLE(STYLIZER_PREFIXED_C_CPP_TYPE(gpu_buffer, gpu_bufferC)*)) buffers;
	index_t texture_count;
	STYLIZER_MANAGEABLE(STYLIZER_NULLABLE(STYLIZER_PREFIXED_C_CPP_TYPE(texture, textureC)*)) textures;

	STYLIZER_MANAGEABLE(STYLIZER_PREFIXED_C_CPP_TYPE(shader, shaderC)*) shader;
	STYLIZER_C_OR_CPP_TYPE(WGPUComputePipeline, wgpu::ComputePipeline) pipeline;
} STYLIZER_PREFIXED_C_CPP_TYPE(computer, computerC);

// Material
typedef struct {
	index_t buffer_count;
	STYLIZER_MANAGEABLE(STYLIZER_NULLABLE(STYLIZER_PREFIXED_C_CPP_TYPE(gpu_buffer, gpu_bufferC)*)) buffers;
	index_t texture_count;
	STYLIZER_MANAGEABLE(STYLIZER_NULLABLE(STYLIZER_PREFIXED_C_CPP_TYPE(texture, textureC)*)) textures;

	STYLIZER_MANAGEABLE(STYLIZER_PREFIXED_C_CPP_TYPE(shader, shaderC)*) shaders;
	STYLIZER_PREFIXED(index_t) shader_count;
	STYLIZER_C_OR_CPP_TYPE(WGPURenderPipeline, wgpu::RenderPipeline) pipeline;
	STYLIZER_NULLABLE(STYLIZER_C_OR_CPP_TYPE(WGPUBindGroup, wgpu::BindGroup)) bind_group;
} STYLIZER_PREFIXED_C_CPP_TYPE(material, materialC);

// Mesh, vertex data
// From: raylib.h
typedef struct {
	STYLIZER_PREFIXED(index_t) triangle_count;	// Number of triangles stored (indexed or not)
	STYLIZER_PREFIXED(index_t) vertex_count;		// Number of vertices stored (length of each of the following arrays)

	// Vertex attributes data
	STYLIZER_MANAGEABLE(STYLIZER_PREFIXED(vec4f)*) positions;						// Vertex position (xyz) mask 1 (w) (shader-location = 0) NOTE: Not nullable
	STYLIZER_MANAGEABLE(STYLIZER_NULLABLE(STYLIZER_PREFIXED(vec4f)*)) normals;		// Vertex normals (xyz) mask 2 (w) (shader-location = 1)
	STYLIZER_MANAGEABLE(STYLIZER_NULLABLE(STYLIZER_PREFIXED(vec4f)*)) tangents;		// Vertex tangents (xyz) and sign (w) to compute bitangent (shader-location = 2)
	STYLIZER_MANAGEABLE(STYLIZER_NULLABLE(STYLIZER_PREFIXED(vec4f)*)) uvs;			// Vertex texture coordinates (xy) and (zw) (shader-location = 3)
	STYLIZER_MANAGEABLE(STYLIZER_NULLABLE(STYLIZER_PREFIXED(vec4f)*)) cta;			// Vertex curvature (x), thickness (y), and associated faces begin (z) and length (z) (shader-location = 4)

	STYLIZER_MANAGEABLE(STYLIZER_NULLABLE(STYLIZER_PREFIXED(STYLIZER_PREFIXED_C_CPP_TYPE(color, colorC))*)) colors;		// Vertex colors (shader-location = 5)
	STYLIZER_MANAGEABLE(STYLIZER_NULLABLE(STYLIZER_PREFIXED(vec4u)*)) bones;			// Bone IDs (shader-location = 6)
	STYLIZER_MANAGEABLE(STYLIZER_NULLABLE(STYLIZER_PREFIXED(vec4f)*)) bone_weights;	// Bone weights associated with each ID (shader-location = 7)

	STYLIZER_MANAGEABLE(STYLIZER_NULLABLE(STYLIZER_PREFIXED(index_t)*)) indices;		// Vertex indices (in case vertex data comes indexed)


	// Animation vertex data
	// vec3f* anim_vertices;      // Animated vertex positions (after bones transformations)
	// vec3f* anim_normals;       // Animated normals (after bones transformations)
	// vec4u* bone_ids;			// Vertex bone ids, max 255 bone ids, up to 4 bones influence by vertex (skinning)
	// vec4f* bone_weights;		// Vertex bone weight, up to 4 bones influence by vertex (skinning)

	STYLIZER_C_OR_CPP_TYPE(WGPUBuffer, wgpu::Buffer) vertex_buffer, transformed_vertex_buffer; // Pointer to the data on the gpu
	STYLIZER_NULLABLE(STYLIZER_C_OR_CPP_TYPE(WGPUBuffer, wgpu::Buffer)) index_buffer, transformed_index_buffer; // Pointer to the index data on the gpu
} STYLIZER_PREFIXED_C_CPP_TYPE(mesh, meshC);

// Model, meshes, materials and animation data
// From: raylib.h
typedef struct {
	STYLIZER_PREFIXED(mat4x4f) transform;			// Local transform matrix

	STYLIZER_PREFIXED(index_t) mesh_count;											// Number of meshes
	STYLIZER_MANAGEABLE(STYLIZER_PREFIXED_C_CPP_TYPE(mesh, meshC)*) meshes;				// Meshes array
	STYLIZER_PREFIXED(index_t) material_count;										// Number of materials
	STYLIZER_MANAGEABLE(STYLIZER_PREFIXED_C_CPP_TYPE(material, materialC)*) materials;	// Materials array
	STYLIZER_MANAGEABLE(STYLIZER_NULLABLE(STYLIZER_PREFIXED(index_t)*)) mesh_materials;	// Mesh to material mapping (when null and )

	// Animation data
	// STYLIZER_PREFIXED(index_t) bone_count;			// Number of bones
	// bone_info* bones;			// Bones information (skeleton)
} STYLIZER_PREFIXED_C_CPP_TYPE(model, modelC);

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
} STYLIZER_PREFIXED(mesh_metadata);

typedef struct {
	float since_start // at byte offset 0
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= 0
#endif
	; float delta; // at byte offset 4
	float average_delta; // at byte offset 8
	float _pad0;
} STYLIZER_PREFIXED_C_CPP_TYPE(time, timeC);

typedef struct {
	STYLIZER_PREFIXED(mat4x4f) view_matrix; // at byte offset 0
	STYLIZER_PREFIXED(mat4x4f) projection_matrix; // at byte offset 64
	STYLIZER_PREFIXED(vec3f) position; // at byte offset 128
	float _pad0;
	STYLIZER_PREFIXED(vec3f) target_position; // at byte offset 144
	float _pad1;
	STYLIZER_PREFIXED(vec3f) up // at byte offset 160
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= {0, 1, 0}
#endif
	; float field_of_view // at byte offset 172
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= 90
#endif
	; float near_clip_distance // at byte offset 176
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= .01
#endif
	; float far_clip_distance // at byte offset 180
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= 1000
#endif
	; uint32_t orthographic; // at byte offset 184
	float _pad2;
} STYLIZER_PREFIXED_C_CPP_TYPE(camera3D, camera3DC);

typedef struct {
	STYLIZER_PREFIXED(mat4x4f) view_matrix; // at byte offset 0
	STYLIZER_PREFIXED(mat4x4f) projection_matrix; // at byte offset 64
	STYLIZER_PREFIXED(vec3f) offset; // at byte offset 128
	float _pad0;
	STYLIZER_PREFIXED(vec3f) target_position; // at byte offset 144
	float rotation; // at byte offset 156
	float near_clip_distance // at byte offset 160
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= .01
#endif
	; float far_clip_distance // at byte offset 164
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= 10
#endif
	; float zoom // at byte offset 168
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= 1
#endif
	; uint32_t pixel_perfect; // at byte offset 172
	uint32_t padding0; // at byte offset 176
	uint32_t padding1; // at byte offset 180
	uint32_t padding2; // at byte offset 184
	float _pad1;
} STYLIZER_PREFIXED_C_CPP_TYPE(camera2D, camera2DC);

STYLIZER_ENUM light_type {
	C_PREPEND(LIGHT_TYPE_, Ambient) = 0,
	C_PREPEND(LIGHT_TYPE_, Directional),
	C_PREPEND(LIGHT_TYPE_, Point),
	C_PREPEND(LIGHT_TYPE_, Spot),
	C_PREPEND(LIGHT_TYPE_, Force32) = 0x7FFFFFFF
};

typedef struct {
	enum light_type light_type; // at byte offset 0
	float intensity; // at byte offset 4
	float cutoff_start_angle; // at byte offset 8
	float cutoff_end_angle; // at byte offset 12
	float falloff; // at byte offset 16
	float _pad0[3];
	STYLIZER_PREFIXED(vec3f) position; // at byte offset 32
	float _pad1;
	STYLIZER_PREFIXED(vec3f) direction; // at byte offset 48
	float _pad2;
	STYLIZER_PREFIXED_C_CPP_TYPE(color, colorC) color; // at byte offset 64
} STYLIZER_PREFIXED_C_CPP_TYPE(light, lightC);

typedef struct {
	STYLIZER_PREFIXED(mat4x4f) transform;
	STYLIZER_PREFIXED_C_CPP_TYPE(color, colorC) tint
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= {1, 1, 1, 1}
#endif
	;
} STYLIZER_PREFIXED(model_instance);


//////////////////////////////////////////////////////////////////////
// # Core Configuration
//////////////////////////////////////////////////////////////////////


typedef struct {
	STYLIZER_OPTIONAL(WGPUPresentMode) presentation_mode
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= {}
#endif
	; WGPUCompositeAlphaMode alpha_mode
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= wgpu::CompositeAlphaMode::Auto
#endif
	; bool automatic_should_configure_now
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	;
} STYLIZER_PREFIXED(surface_configuration);

STYLIZER_ENUM texture_create_sampler_type {
	C_PREPEND(TEXTURE_CREATE_SAMPLER_TYPE_, None) = 0,
	C_PREPEND(TEXTURE_CREATE_SAMPLER_TYPE_, Nearest),
	C_PREPEND(TEXTURE_CREATE_SAMPLER_TYPE_, Trilinear),
};

typedef struct {
	STYLIZER_OPTIONAL(WGPUTextureFormat) format
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= {}
#endif
	; WGPUTextureUsage usage
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::TextureBinding
#endif
	; bool with_view
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; texture_create_sampler_type sampler_type
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= texture_create_sampler_type::Nearest
#endif
	; uint32_t mip_levels
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= 1
#endif
	; 
} STYLIZER_PREFIXED(texture_create_configuration);

typedef struct {
	STYLIZER_NULLABLE(STYLIZER_PREFIXED_C_CPP_TYPE(geometry_buffer, geometry_bufferC)*) previous
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	; WGPUTextureFormat color_format
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= wgpu::TextureFormat::RGBA8Unorm
#endif
	; WGPUTextureFormat depth_format
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= wgpu::TextureFormat::Depth24Plus
#endif
	; WGPUTextureFormat normal_format
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= wgpu::TextureFormat::RGBA16Float
#endif
	;
} STYLIZER_PREFIXED(geometry_buffer_config);

typedef struct {
	bool remove_comments
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	; bool remove_whitespace
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	; bool support_pragma_once
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= true
#endif
	; const char* path
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	;
} STYLIZER_PREFIXED(preprocess_shader_config);

typedef struct {
	STYLIZER_NULLABLE(const char*) compute_entry_point
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	; STYLIZER_NULLABLE(const char*) vertex_entry_point
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	; STYLIZER_NULLABLE(const char*) fragment_entry_point
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	; STYLIZER_NULLABLE(const char*) name
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	; STYLIZER_NULLABLE(shader_preprocessor*) preprocessor
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	; preprocess_shader_config preprocess_config
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= {}
#endif
	;
} STYLIZER_PREFIXED(create_shader_configuration);

typedef struct {
	bool double_sided // Determines if the material should be culled from the back or if it should be visible from both sides
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= false
#endif
	; STYLIZER_OPTIONAL(WGPUCompareFunction) depth_function
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= wgpu::CompareFunction::Less // Disables writing depth if not provided
#endif
	; // TODO: Add stencil support
	shader_preprocessor* preprocessor
#ifdef STYLIZER_ENABLE_DEFAULT_PARAMETERS
		= nullptr
#endif
	;
} STYLIZER_PREFIXED(material_configuration);

#endif // STYLIZER_INTERFACES_f9641d712fbea9630f1ce0a0613269d7