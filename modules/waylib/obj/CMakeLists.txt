add_library(stylizer_obj obj.cpp)
target_link_libraries(stylizer_obj PUBLIC stylizer::core)
add_library(stylizer::obj ALIAS stylizer_obj)