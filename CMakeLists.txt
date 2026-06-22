cmake_minimum_required(VERSION 4.2)
project(image_viewer C)

set(CMAKE_C_STANDARD 11)

add_executable(image_viewer main.c
        netpbm_utils.c
        netpbm_utils.h
        image_viewer_utils.c
        image_viewer_utils.h)
target_compile_options(image_viewer PRIVATE -Wall -Wextra)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
target_link_libraries(image_viewer PRIVATE SDL3::SDL3)