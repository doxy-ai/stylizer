add_library(stylizer)
target_sources(stylizer PUBLIC
	FILE_SET stylizer TYPE CXX_MODULES
	# BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		api.cppm
		context.cppm
)
target_link_libraries(stylizer PUBLIC stylizer::graphics::current_backend)
target_include_directories(stylizer PUBLIC ../../)