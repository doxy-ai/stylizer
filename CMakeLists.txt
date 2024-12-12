cmake_minimum_required(VERSION 3.20)
project(stylizer CXX)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(modules/stylizer/api)
add_subdirectory(modules/stylizer/core)
add_subdirectory(modules/stylizer/window)

add_executable(tst "test.cpp")
target_link_libraries(tst PUBLIC stylizer::core stylizer::window)

