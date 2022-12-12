cmake_minimum_required(VERSION 3.14)
project(til)

foreach(item IN ITEMS
        asset-manage
        make-wav
        object-pool
        structured-bindings
        task-schedule)
    add_executable(${item} ${item}.cpp)
    target_compile_features(${item} PRIVATE cxx_std_17)
    target_compile_options(${item} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-W -Wall>)
    target_include_directories(${item} PRIVATE .)
endforeach()
