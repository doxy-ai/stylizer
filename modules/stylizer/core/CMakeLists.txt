set(GLM_ENABLE_CXX_20 ON CACHE BOOL "Enable C++ 20")
set(GLM_ENABLE_LANG_EXTENSIONS ON CACHE BOOL "Enable language extensions")
add_subdirectory(thirdparty/glm)
add_subdirectory(thirdparty/WebGPU-distribution)
add_subdirectory(thirdparty/embed)

add_library(stylizer_core core.cpp)
target_link_libraries(stylizer_core PUBLIC webgpu glm::glm)

set(MODULE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../)
cmake_path(ABSOLUTE_PATH MODULE_DIRECTORY NORMALIZE)
target_include_directories(stylizer_core PUBLIC ${MODULE_DIRECTORY})

if(EMSCRIPTEN)
	target_link_options(stylizer_core PUBLIC
		-sUSE_WEBGPU
		-sASYNCIFY # Required by WebGPU-C++
		-sALLOW_MEMORY_GROWTH
	)
endif()

add_library(stylizer::core ALIAS stylizer_core)

include(CheckCXXSourceCompiles)
check_cxx_source_compiles("#include <expected>
int main() {
	std::expected<int, int> works;
}" STD_EXPECTED_SUPPORTED)
if(NOT STD_EXPECTED_SUPPORTED)
	message("-- Fetching supplemental expected library...")

	include(FetchContent)
	FetchContent_Declare(fetch-expected GIT_REPOSITORY https://github.com/TartanLlama/expected.git)
	set(EXPECTED_BUILD_TESTS off CACHE BOOL "Enable tl::expected tests")
	set(EXPECTED_BUILD_PACKAGE_DEB off CACHE BOOL "Create a DEB")
	FetchContent_MakeAvailable(fetch-expected)
	target_link_libraries(stylizer_core PUBLIC expected)
endif()

function(stylizer_embed TARGET FILENAME)
	b_embed(${TARGET} ${FILENAME})
endfunction(stylizer_embed)

stylizer_embed(stylizer_core shaders/embeded/inverse.wgsl)
stylizer_embed(stylizer_core shaders/embeded/mesh_data.wgsl)
stylizer_embed(stylizer_core shaders/embeded/utility_data.wgsl)
stylizer_embed(stylizer_core shaders/embeded/vertex_data.wgsl)
stylizer_embed(stylizer_core shaders/embeded/default_gbuffer_data.wgsl)
stylizer_embed(stylizer_core shaders/embeded/default_gbuffer.wgsl)
stylizer_embed(stylizer_core shaders/embeded/default_blit.wgsl)