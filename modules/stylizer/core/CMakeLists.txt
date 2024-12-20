add_subdirectory(thirdparty/embed)

add_library(stylizer_core core.cpp)
target_link_libraries(stylizer_core PUBLIC stylizer::api)
target_include_directories(stylizer_core PUBLIC ../../ thirdparty/hlslpp/include) # /modules
add_library(stylizer::core ALIAS stylizer_core)

function(stylizer_embed TARGET FILENAME)
	b_embed(${TARGET} ${FILENAME})
endfunction(stylizer_embed)

stylizer_embed(stylizer_core shaders/embeded/stylizer.slang)
stylizer_embed(stylizer_core shaders/embeded/stylizer.default.slang)