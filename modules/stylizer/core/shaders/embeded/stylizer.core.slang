import stylizer;

module stylizer_core;

#include "stylizer.core.types.slang"

namespace stylizer::core {

	[[vk::binding(0, 0)]] public StructuredBuffer<stylizer::instance_data> instances;
	[[vk::binding(1, 0)]] public ConstantBuffer<stylizer::utility_data> utility;
	[[vk::binding(2, 0)]] public StructuredBuffer<stylizer::light_data> lights;
	[[vk::binding(3, 0)]] public ConstantBuffer<stylizer::mesh_metadata_t> mesh_metadata;
	[[vk::binding(4, 0)]] public StructuredBuffer<float> mesh_vertecies;
	[[vk::binding(5, 0)]] public StructuredBuffer<uint> mesh_indicies;

	public void ensure_vertex_layout() {
		var trashf = instances[0].color.x;
		trashf = utility.time.delta;
		var trashu = lights[0].light_type;
		trashu = mesh_metadata.is_indexed;
		trashf = mesh_vertecies[0];
		trashu = mesh_indicies[0];
	}

}

// Make the core namespace inline!
namespace stylizer { using namespace stylizer::core; }