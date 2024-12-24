module stylizer;

namespace stylizer::math {
	public static let identity_matrix = float4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

	public float4x4 operator*(float4x4 a, float4x4 b) {
		return mul(a, b);
	}
	public float4 operator*(float4x4 mat, float4 vec) {
		return mul(mat, vec);
	}

	public float3x3 operator*(float3x3 a, float3x3 b) {
		return mul(a, b);
	}
	public float3 operator*(float3x3 mat, float3 vec) {
		return mul(mat, vec);
	}

	public float2x2 operator*(float2x2 a, float2x2 b) {
		return mul(a, b);
	}
	public float2 operator*(float2x2 mat, float2 vec) {
		return mul(mat, vec);
	}
}


namespace stylizer {
	using namespace stylizer::math;

	public struct vertex_to_fragment {
		public float4 builtin_position : SV_Position;
		[[vk::location(0)]] public float3 position;
		[[vk::location(1)]] public float mask1;
		[[vk::location(2)]] public float3 normal;
		[[vk::location(3)]] public float mask2;
		[[vk::location(4)]] public float3 tangent;
		[[vk::location(5)]] public float3 bitangent;
		[[vk::location(6)]] public float2 uv;
		[[vk::location(7)]] public float2 uv2;
		[[vk::location(8)]] public float curvature;
		[[vk::location(9)]] public float thickness;
		[[vk::location(10)]] public uint2 adjacency;
		[[vk::location(11)]] public uint vertex_index;
		[[vk::location(12)]] public uint instance_index;
		[[vk::location(13)]] public float4 color;
		[[vk::location(14)]] public uint4 bones;
		[[vk::location(15)]] public float4 bone_weights;

		[[mutating]]
		public void apply_model_view_projection_matrix(float4x4 model_matrix, float4x4 view_projection_matrix) {
			float4 world_position = model_matrix * builtin_position;
			builtin_position = view_projection_matrix * world_position;
			position = world_position.xyz;
			normal = normalize((model_matrix *float4(normal, 0.0)).xyz);
		}

		[[mutating]]
		public void generate_bitangent() {
			normal = normalize(normal);
			tangent = normalize(tangent);
			bitangent = normalize(cross(normal, tangent) * bitangent.x);
		}

		public float3 normal_mapping(float3 tangent_space_normal) {
			float3x3 TBN = float3x3(tangent, bitangent, normal);
			return TBN * tangent_space_normal;
		}
	};

	public static const float4 imported_color = float4(1.0, 0.4, 1.0, 1.0);

}