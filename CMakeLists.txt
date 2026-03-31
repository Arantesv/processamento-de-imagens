cmake_minimum_required(VERSION 3.16)
project(proj1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_image REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)

add_executable(proj1
    src/main.cpp
    src/app.cpp
    src/image_processing.cpp
    src/ui.cpp
)

target_include_directories(proj1 PRIVATE include)

target_compile_options(proj1 PRIVATE -Wall -Wextra -Wpedantic)

target_link_libraries(proj1 PRIVATE
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_ttf::SDL3_ttf
)
