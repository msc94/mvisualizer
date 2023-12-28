cmake_minimum_required(VERSION 3.13)
project(mvisualizer)

find_package(spdlog CONFIG REQUIRED)
link_libraries(spdlog::spdlog spdlog::spdlog_header_only)

link_libraries(m soundio)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_COLOR_MAKEFILE ON)
SET(CMAKE_COLOR_DIAGNOSTICS ON)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_executable(mvisualizer
    main.cpp
    capture.cpp
    capture.h
    window.cpp
    window.h
)
