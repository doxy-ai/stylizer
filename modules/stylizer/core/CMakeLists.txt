set(GLM_ENABLE_CXX_20 ON CACHE BOOL "Enable C++ 20")
set(GLM_ENABLE_LANG_EXTENSIONS ON CACHE BOOL "Enable language extensions")
add_subdirectory(thirdparty/glm)
add_subdirectory(thirdparty/embed)

add_library(stylizer_core core.cpp)
target_link_libraries(stylizer_core PUBLIC stylizer::api stylizer::hardware glm::glm)
target_include_directories(stylizer_core PUBLIC ../../)
add_library(stylizer::core ALIAS stylizer_core)

function(stylizer_embed TARGET FILENAME)
	b_embed(${TARGET} ${FILENAME})
endfunction(stylizer_embed)

stylizer_embed(stylizer_core shaders/embeded/stylizer.slang)
stylizer_embed(stylizer_core shaders/embeded/stylizer.core.types.slang)
stylizer_embed(stylizer_core shaders/embeded/stylizer.core.slang)