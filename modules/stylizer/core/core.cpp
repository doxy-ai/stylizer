#define IS_STYLIZER_CORE_CPP
#define STYLIZER_API_IMPLEMENTATION
#include "core.hpp"

#include <battery/embed.hpp>

namespace stylizer {
	context context::null = {};
	template<>
	const float4 mesh::vertex<true>::not_available = {};
	template<>
	const float4 mesh::vertex<false>::not_available = {};

	void shader_processor::inject_default_virtual_filesystem() {
		static bool included = []{
			utility_buffer_light_offset = utility_buffer_light_offset - utility_buffer_light_offset % buffer_alignment + buffer_alignment; // Ensure the utility_buffer_light_offset is divisible by the buffer alignment
			minimum_utility_buffer_size = utility_buffer_light_offset + sizeof(light_gpu_data);

			auto session = get_session();

			slcross::preprocessor p;
			p.process_from_memory_and_cache(b::embed<"shaders/embeded/stylizer.core.types.slang">().str(), "stylizer.core.types.slang", {});

			slcross::slang::inject_module_from_memory(session, b::embed<"shaders/embeded/stylizer.slang">().str(), "stylizer.slang", "stylizer");
			slcross::slang::inject_module_from_memory(session, p.process_from_memory(b::embed<"shaders/embeded/stylizer.core.slang">().str(), {}), "stylizer.core.slang", "stylizer_core");

			return true;
		}();
	}

}