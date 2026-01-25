struct time {
	float total = 0, delta = 0, smoothed_delta = 0;
	uint frame = 0;
}

struct camera {
	float4x4 view, projection;
	float4x4 inverse_view, inverse_projection;
}

struct utility_buffer {
	camera camera;
	time time;
}

struct instance_data {
	float4x4 model;
}

[[vk::binding(0, 0)]] StructuredBuffer<instance_data> instances;
[[vk::binding(1, 0)]] StructuredBuffer<utility_buffer> utils;

struct config {
	float4 color;
	uint use_texture_;
	enum srgb_state : uint {
		InputLinearOutputLinear = 0,
		InputLinearOutputSRGB = (1 << 1),
		InputSRGBOutputLinear = (1 << 0),
		InputSRGBOutputSRGB = (1 << 1 | 1 << 0)
	} srgb;

	bool use_texture() { return use_texture_ != 0; }
	bool input_srgb() { return (srgb & (1 << 0)) > 0; }
	bool output_srgb() { return (srgb & (1 << 1)) > 0; }
}

[[vk::binding(0, 1)]] Texture2D<float4> texture;
[[vk::binding(1, 1)]] SamplerState sampler;
[[vk::binding(2, 1)]] StructuredBuffer<config> configs;

struct vertex_input {
	float3 pos : POSITIONS;
	float4 uv : UVS;
}

struct varryings {
	float4 pos : SV_Position;
	float2 uv : UVS;
}

[shader("vertex")]
varryings vertex(vertex_input vert, uint instance_id : SV_InstanceID) {
	var util = utils[0];
	var instance = instances[instance_id];
	var MVP = mul(util.camera.projection, mul(util.camera.view, instance.model));
	return { mul(MVP, float4(vert.pos, 1.0)), float2(vert.uv.x, 1 - vert.uv.y) }; // TODO: Why is the v axis inverted!?!?
}

float4 srgb_to_linear(float4 c) {
	return pow(c, 2.2);
}

float4 linear_to_srgb(float4 c) {
	return pow(c, 1 / 2.2);
}

float4 normalize_srgb(float4 c, bool input_srgb, bool output_srgb) {
	if(input_srgb == output_srgb) return c;
	float4 linear = c;
	if(input_srgb) linear = srgb_to_linear(linear);
	if(output_srgb) return linear_to_srgb(linear);
	return linear;
}

[shader("fragment")]
float4 fragment(varryings vert) : SV_Target {
	var sample = texture.Sample(sampler, vert.uv);
	var color = configs[0].color;
	if(configs[0].use_texture())
		color = sample;
	
	return normalize_srgb(color, configs[0].input_srgb(), configs[0].output_srgb());
	// return float4(231.0/255, 39.0/255, 37.0/255, 1.0) + sample * .001 + color * .001;
}