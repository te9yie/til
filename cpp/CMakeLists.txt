cmake_minimum_required(VERSION 3.14)
project(til CXX)

set(CMAKE_CXX_STANDARD 17)
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-W -Wall)
endif()

add_executable(structured-bindings structured-bindings.cpp)
add_executable(task-schedule task-schedule.cpp)