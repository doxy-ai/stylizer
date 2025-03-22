cmake_minimum_required(VERSION 3.20)
project(stylizer CXX)

set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")

add_subdirectory(modules/stylizer/api)
add_subdirectory(modules/stylizer/hardware)
add_subdirectory(modules/stylizer/core)
add_subdirectory(modules/stylizer/img)
add_subdirectory(modules/stylizer/obj)
add_subdirectory(modules/stylizer/window)

add_executable(tst "test.cpp")
target_link_libraries(tst PUBLIC stylizer::core stylizer::img stylizer::obj stylizer::window)

