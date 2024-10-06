add_library(waylib_obj obj.cpp)
target_link_libraries(waylib_obj PUBLIC waylib::core)
add_library(waylib::obj ALIAS waylib_obj)