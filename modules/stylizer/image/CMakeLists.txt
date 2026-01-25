add_library(stylizer_image)
target_sources(stylizer_image PUBLIC
	FILE_SET stylizer_image TYPE CXX_MODULES
	# BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
	FILES 
		image.cppm
		api.cppm
)
target_link_libraries(stylizer_image PUBLIC stylizer)

add_library(stylizer::image ALIAS stylizer_image)