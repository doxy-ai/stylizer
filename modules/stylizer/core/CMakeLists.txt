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
)
target_link_libraries(stylizer PUBLIC stylizer::graphics::current_backend reaction)
target_include_directories(stylizer PUBLIC ../../)