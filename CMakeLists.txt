cmake_minimum_required(VERSION 3.21)
project(stylizer CXX)

set(STYLIZER_FETCH_REACTIVE true)

include(FetchContent)

if(STYLIZER_FETCH_REACTIVE)
	FetchContent_Declare(
		fetch_reactive
		GIT_REPOSITORY https://github.com/lumia431/reaction.git
	)
	FetchContent_MakeAvailable(fetch_reactive)
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