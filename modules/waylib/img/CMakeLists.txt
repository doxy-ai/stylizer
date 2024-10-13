add_library(waylib_img img.cpp)
target_link_libraries(waylib_img PUBLIC waylib::core)
add_library(waylib::img ALIAS waylib_img)