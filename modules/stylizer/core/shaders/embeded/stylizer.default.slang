module stylizer_default;

public struct vertex_input {
	[[vk::location(0)]] public float4 position;      // Location 0 -> position
	[[vk::location(1)]] public float4 normal;        // Location 1 -> normal
	[[vk::location(2)]] public float4 tangent;       // Location 2 -> tangent
	[[vk::location(3)]] public float4 uv;            // Location 3 -> uv
	[[vk::location(4)]] public float4 cta;           // Location 4 -> cta
	[[vk::location(5)]] public float4 color;         // Location 5 -> color
	[[vk::location(6)]] public uint4 bones;          // Location 6 -> bones (using uint4)
	[[vk::location(7)]] public float4 bone_weights;  // Location 7 -> bone_weights
};

public struct fragment_output {
	public float4 color : SV_Target;
};