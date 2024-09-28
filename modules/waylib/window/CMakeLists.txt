set(GLFWPP_BUILD_EXAMPLES off CACHE BOOL "Should examples be built")
add_subdirectory(thirdparty/glfwpp)
add_subdirectory(thirdparty/glfw3webgpu)

add_library(waylib_window window.cpp)
target_link_libraries(waylib_window PUBLIC waylib::core GLFWPP glfw3webgpu)
add_library(waylib::window ALIAS waylib_window)