cmake_minimum_required(VERSION 3.14)
project(task)

file(GLOB sources *.h *.cpp)

add_executable(task ${sources})
target_compile_features(task PRIVATE cxx_std_17)
target_compile_options(task PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-W -Wall>)
target_include_directories(task PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)