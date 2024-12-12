include(../api/cmake/glfw.cmake)

add_library(stylizer_window window.cpp)
target_link_libraries(stylizer_window PUBLIC stylizer::core)
target_setup_glfw_for_stylizer(stylizer_window)
add_library(stylizer::window ALIAS stylizer_window)

if(NOT TARGET glfw)
	# Try to find with cmake
	find_package(glfw3 CONFIG)

	if(glfw3_FOUND)
		target_link_libraries(stylizer_window PUBLIC glfw)
	else()
		# Try to find with pkg-config if cmake config files are missing
		find_package(PkgConfig REQUIRED)
		pkg_check_modules(GLFW IMPORTED_TARGET glfw3)

		if(GLFW_FOUND)
			target_link_libraries(stylizer_window PUBLIC glfw)
		else()
			# Build glfw from source if needed
			include(FetchContent)
			FetchContent_Declare(
				glfw
				GIT_REPOSITORY https://github.com/glfw/glfw.git
				GIT_TAG 3.3.8)

			# Disable docs, tests, and examples for the build
			set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
			set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
			set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

			FetchContent_MakeAvailable(glfw)
			target_link_libraries(stylizer_window PUBLIC glfw)
		endif()
	endif()
else()
	target_link_libraries(stylizer_window PUBLIC glfw)
endif()