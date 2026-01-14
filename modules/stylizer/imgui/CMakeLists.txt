FetchContent_Declare(
	fetch_imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
)
FetchContent_MakeAvailable(fetch_imgui)

set(IMGUI_DIR ${fetch_imgui_SOURCE_DIR})

set(IMGUI_FILES ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp)

if(STYLIZER_API_ENABLE_WEBGPU)
	list(APPEND IMGUI_FILES ${IMGUI_DIR}/backends/imgui_impl_wgpu.cpp)
endif(STYLIZER_API_ENABLE_WEBGPU)

# TODO: We should probably gate this behind sdl somehow...
list(APPEND IMGUI_FILES ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp)

add_library(stylizer_imgui ${IMGUI_FILES} high_level.cpp imgui_impl_stylizer_api.cpp imgui_impl_stylizer_window.cpp)
target_include_directories(stylizer_imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(stylizer_imgui PUBLIC stylizer::sdl3)

stylizer_embed(stylizer_imgui fonts/InterVariable.ttf)

if(STYLIZER_API_ENABLE_WEBGPU)
	target_compile_options(stylizer_imgui PUBLIC -DIMGUI_IMPL_WEBGPU_BACKEND_DAWN)
endif(STYLIZER_API_ENABLE_WEBGPU)

add_library(stylizer::imgui ALIAS stylizer_imgui)
