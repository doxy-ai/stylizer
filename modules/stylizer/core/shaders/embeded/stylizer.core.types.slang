namespace stylizer::core {

	public struct mesh_metadata_t {
		uint is_indexed;
		uint vertex_count;
		uint triangle_count;

		uint position_start;
		uint normals_start;
		uint tangents_start;
		uint uvs_start;
		uint cta_start;

		uint colors_start;
		uint bones_start;
		uint bone_weights_start;

		uint vertex_buffer_size;
	};

	public struct vertex_input {
		[[vk::location(0)]] public float4 position;      // Location 0 -> position
		[[vk::location(1)]] public float4 normal;        // Location 1 -> normal
		[[vk::location(2)]] public float4 tangent;       // Location 2 -> tangent
		[[vk::location(3)]] public float4 uvs;           // Location 3 -> uv
		[[vk::location(4)]] public float4 cta;           // Location 4 -> cta
		[[vk::location(5)]] public float4 color;         // Location 5 -> color
		[[vk::location(6)]] public uint4 bones;          // Location 6 -> bones (using uint4)
		[[vk::location(7)]] public float4 bone_weights;  // Location 7 -> bone_weights

		public stylizer::vertex_to_fragment unpack(uint vertex_index, uint instance_index) {
			return {
				float4(position.xyz, 1.0), // builtin_position
				position.xyz, // position
				position.w, // mask1
				normal.xyz, // normal
				normal.w, // mask2
				tangent.xyz, // tangent
				float3(tangent.w, 0.0, 0.0), // bitangent (placeholder)
				uvs.xy, // uv
				uvs.zw, // uv2
				cta.x, // curvature
				cta.y, // thickness
				reinterpret<uint2>(cta.zw), // adjacency
				vertex_index,
				instance_index,
				color, // color
				bones, // bones
				bone_weights, // bone_weights
			};
		}

		public stylizer::vertex_to_fragment unpack_full(uint vertex_index, uint instance_index, float4x4 model_matrix, float4x4 view_projection_matrix) {
			var out = unpack(vertex_index, instance_index);
			out.apply_model_view_projection_matrix(model_matrix, view_projection_matrix);
			out.generate_bitangent();
			return out;
		}
	};

	public struct fragment_output {
		public float4 color : SV_Target;
	};

	public struct fragment_output_with_depth {
		public float4 color : SV_Target;
		public float depth : SV_Depth;
	};

}