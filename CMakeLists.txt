cmake_minimum_required(VERSION 3.31)
project(cache_traits)

set(CMAKE_CXX_STANDARD 23)

include_directories(include)

add_executable(cache_traits main.cpp
        src/Workload.cpp)
