add_library(stylizer_model dynamic_mesh.cpp)
target_link_libraries(stylizer_model PUBLIC stylizer::core)
target_compile_options(stylizer_model PUBLIC -DSTYLIZER_MODEL_AVAILABLE)

add_library(stylizer::model ALIAS stylizer_model)