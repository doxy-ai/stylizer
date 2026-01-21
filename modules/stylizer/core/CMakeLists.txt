add_subdirectory(thirdparty/embed)

add_library(stylizer_core texture.cpp surface.cpp flat_material.cpp frame_buffer.cpp instance_buffer.cpp util/load_file.cpp)
target_include_directories(stylizer_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(stylizer_core PUBLIC stylizer::api::current_backend reaction)

function(stylizer_embed TARGET FILENAME)
	b_embed(${TARGET} ${FILENAME})
endfunction(stylizer_embed)

function(stylizer_embed_absolute TARGET ABSOLUTE_PATH)
	get_filename_component(CURRENT_DIR ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)
	file(RELATIVE_PATH RELATIVE_PATH ${CURRENT_DIR} ${ABSOLUTE_PATH})
	stylizer_embed(${TARGET} ${RELATIVE_PATH})
endfunction(stylizer_embed_absolute)

add_library(stylizer::core ALIAS stylizer_core)