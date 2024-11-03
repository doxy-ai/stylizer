add_library(stylizer_img img.cpp)
target_link_libraries(stylizer_img PUBLIC stylizer::core)
add_library(stylizer::img ALIAS stylizer_img)