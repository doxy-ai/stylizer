add_library(stylizer_window)
target_sources(stylizer_window PUBLIC
	FILE_SET stylizer_window TYPE CXX_MODULES
	# BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
	FILES api.cppm
)
target_link_libraries(stylizer_window PUBLIC stylizer)
target_compile_definitions(stylizer_window PUBLIC -DSTYLIZER_WINDOW_AVAILABLE)

add_library(stylizer::window ALIAS stylizer_window)