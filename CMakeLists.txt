cmake_minimum_required(VERSION 3.30)
project(stylizer CXX)

set(CMAKE_CXX_STANDARD 26)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	add_compile_options(-stdlib=libc++)
	add_link_options(-stdlib=libc++)
endif()

set(STYLIZER_FETCH_REACTIVE true)

set(STYLIZER_FETCH_REACTION true)

include(FetchContent)

if(STYLIZER_FETCH_REACTION)
	FetchContent_Declare(
		fetch_reaction
		GIT_REPOSITORY https://github.com/doxy-ai/reaction.git
	)
	FetchContent_MakeAvailable(fetch_reaction)
endif()

set(STYLIZER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/modules/stylizer)
set(STYLIZER_EXTRA_MODULE_REPOSITORIES
	"https://github.com/doxy-ai/stylizer.sdl;https://github.com/doxy-ai/stylizer.imgui"
	CACHE STRING "Extra Stylizer module repositories"
)
foreach(repo ${STYLIZER_EXTRA_MODULE_REPOSITORIES})
    # Create a unique name and directory per repo
    get_filename_component(repo_name ${repo} NAME)
	string(REGEX REPLACE "^stylizer\." "" repo_name "${repo_name}")
	message("Fetching extra module: ${repo_name}")

    FetchContent_Declare(
        stylizer_extra_${repo_name}
        GIT_REPOSITORY ${repo}
		GIT_TAG main
        SOURCE_DIR ${STYLIZER_SOURCE_DIR}/${repo_name}
    )

    # FetchContent_MakeAvailable(stylizer_extra_${repo_name})
endforeach()

set(STYLIZER_MODULES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/modules/stylizer")
file(GLOB STYLIZER_MODULES_CHILDREN RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${STYLIZER_MODULES_DIR}/*")
foreach(child ${STYLIZER_MODULES_CHILDREN})
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${child}/CMakeLists.txt")
		message("Adding Module: ${child}")
		add_subdirectory("${child}")
	endif()
endforeach()

add_executable(hello hello_gui.cpp)
target_link_libraries(hello PUBLIC stylizer stylizer::sdl3 stylizer::image stylizer::model)