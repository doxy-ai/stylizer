add_library(stylizer_input)
target_sources(stylizer_input PUBLIC
	FILE_SET stylizer_input TYPE CXX_MODULES
	# BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
	FILES api.cppm
)
target_link_libraries(stylizer_input PUBLIC stylizer)

add_library(stylizer::input ALIAS stylizer_input)