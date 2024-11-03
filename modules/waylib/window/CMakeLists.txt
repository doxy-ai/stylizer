set(GLFWPP_BUILD_EXAMPLES off CACHE BOOL "Should examples be built")
add_subdirectory(thirdparty/glfwpp)
add_subdirectory(thirdparty/glfw3webgpu)

add_library(stylizer_window window.cpp)
target_link_libraries(stylizer_window PUBLIC stylizer::core GLFWPP glfw3webgpu)
add_library(stylizer::window ALIAS stylizer_window)