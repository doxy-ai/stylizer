add_library(stylizer_model)
target_sources(stylizer_model PUBLIC
	FILE_SET stylizer_model TYPE CXX_MODULES
	# BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		storage.cppm
		api.cppm
)
target_link_libraries(stylizer_model PUBLIC stylizer)

add_library(stylizer::model ALIAS stylizer_model)