cmake_minimum_required(VERSION 3.20)
project(stylizer CXX)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(modules/api)

add_executable(tst "test.cpp")
target_link_libraries(tst PUBLIC stylizer::api)

include(modules/api/cmake/glfw.cmake)
target_setup_glfw_for_stylizer(tst)