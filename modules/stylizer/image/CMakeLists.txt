add_library(stylizer_image image.cpp)
target_link_libraries(stylizer_image PUBLIC stylizer::core)
target_compile_options(stylizer_image PUBLIC -DSTYLIZER_IMAGE_AVAILABLE)

add_library(stylizer::image ALIAS stylizer_image)