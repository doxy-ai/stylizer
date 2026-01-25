add_library(stylizer_model)
target_sources(stylizer_model PUBLIC
	FILE_SET stylizer_model TYPE CXX_MODULES
	# BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
	FILES
		storage.cppm
		mesh.cppm
		dynamic_mesh.cppm
		instance_data.cppm
		model.cppm
		load.cppm
		api.cppm
)
target_link_libraries(stylizer_model PUBLIC stylizer stylizer::image)
target_compile_definitions(stylizer_model PUBLIC -DSTYLIZER_MODEL_AVAILABLE)

add_library(stylizer::model ALIAS stylizer_model)