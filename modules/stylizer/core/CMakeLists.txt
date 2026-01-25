add_subdirectory(thirdparty/embed)

add_library(stylizer util/reaction.cpp)
target_sources(stylizer PUBLIC
	FILE_SET stylizer TYPE CXX_MODULES
	# BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		util/maybe_owned.cppm
		api.cppm
		context.cppm
		time.cppm
		camera.cppm
		texture.cppm
		surface.cppm
		buffer.cppm
		buffer.utility.cppm
		frame_buffer.cppm
		frame_buffer.color_depth.cppm
		frame_buffer.single_texture.cppm
		material.cppm
		material.flat.cppm
)
target_link_libraries(stylizer PUBLIC stylizer::graphics::current_backend reaction)
target_compile_definitions(stylizer PUBLIC -DSTYLIZER_CORE_AVAILABLE)
target_include_directories(stylizer PUBLIC ../../)

function(stylizer_embed TARGET FILENAME)
	b_embed(${TARGET} ${FILENAME})
endfunction(stylizer_embed)

function(stylizer_embed_absolute TARGET ABSOLUTE_PATH)
	get_filename_component(CURRENT_DIR ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)
	file(RELATIVE_PATH RELATIVE_PATH ${CURRENT_DIR} ${ABSOLUTE_PATH})
	stylizer_embed(${TARGET} ${RELATIVE_PATH})
endfunction(stylizer_embed_absolute)