
add_library(stylizer_core surface.cpp)
target_include_directories(stylizer_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../..)
target_link_libraries(stylizer_core PUBLIC stylizer::api::current_backend reaction)

add_library(stylizer::core ALIAS stylizer_core)