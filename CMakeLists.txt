cmake_minimum_required(VERSION 3.12)
project(waylib CXX C)

set(CMAKE_CXX_STANDARD 20)

if(CMAKE_BUILD_TYPE MATCHES Debug)
	message("Debug Build... Some Optimizations Disabled")
else()
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

add_subdirectory(modules/waylib/core)
add_subdirectory(modules/waylib/window)

add_executable(waylib_tests test.cpp)
target_link_libraries(waylib_tests PUBLIC waylib::core waylib::window)

if(EMSCRIPTEN)
	set_target_properties(waylib_tests PROPERTIES SUFFIX ".html")
	# target_link_options(waylib_tests PUBLIC
	# 	-fexceptions
	# 	--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/resources@resources
	# )
endif()