cmake_minimum_required(VERSION 3.31)
project(cache_traits)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 99)

include_directories(include)

add_compile_options(-O0)

add_executable(cache_traits main.cpp)

add_executable(calibrator calibrator.c)
