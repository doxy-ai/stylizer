find_package(SDL3 REQUIRED)
# If not found, fetch and build it
if (NOT SDL3_FOUND)
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        main # or a specific release tag like release-3.2.0
    )

    set(SDL_TEST OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(SDL3)
endif()

add_library(stylizer_sdl3 event.cpp)
target_link_libraries(stylizer_sdl3 PUBLIC stylizer::core)

include(../api/cmake/sdl3.cmake)
target_setup_sdl3_for_stylizer(stylizer_sdl3)

add_library(stylizer::sdl3 ALIAS stylizer_sdl3)