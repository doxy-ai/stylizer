

add_library(stylizer_core core.cpp)
target_link_libraries(stylizer_core PUBLIC stylizer::api)
target_include_directories(stylizer_core PUBLIC ../../) # /modules
add_library(stylizer::core ALIAS stylizer_core)