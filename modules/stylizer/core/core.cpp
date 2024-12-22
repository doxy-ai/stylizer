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
		bool included = []{
			auto session = get_session();

			slcross::slang::inject_module_from_memory(session, b::embed<"shaders/embeded/stylizer.slang">().str(), "stylizer.slang", "stylizer");
			slcross::slang::inject_module_from_memory(session, b::embed<"shaders/embeded/stylizer.default.slang">().str(), "stylizer.default.slang", "stylizer_default");

			return true;
		}();
	}

}