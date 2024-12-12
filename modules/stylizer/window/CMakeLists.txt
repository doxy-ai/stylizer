set(GLFWPP_BUILD_EXAMPLES off CACHE BOOL "Should examples be built")
add_subdirectory(thirdparty/glfwpp)
include(../api/cmake/glfw.cmake)

add_library(stylizer_window window.cpp)
target_link_libraries(stylizer_window PUBLIC stylizer::core GLFWPP)
target_setup_glfw_for_stylizer(stylizer_window)
add_library(stylizer::window ALIAS stylizer_window)