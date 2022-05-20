cmake_minimum_required(VERSION 3.14)
project(game)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
    sdl2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL
    GIT_TAG release-2.0.20
)
FetchContent_MakeAvailable(sdl2)

file(GLOB_RECURSE SRC_IMGUI "imgui/*.h" "imgui/*.cpp")
source_group("imgui" FILES ${SRC_IMGUI})

add_executable(${PROJECT_NAME} WIN32
    main.cpp
    ${SRC_IMGUI})
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-W -Wall>)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 SDL2::SDL2main)

if (MSVC)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL2::SDL2>
        ${CMAKE_CURRENT_BINARY_DIR}
        VERBATIM)
endif()