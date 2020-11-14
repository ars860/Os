cmake_minimum_required(VERSION 3.10)
project(libs)

set(CMAKE_CXX_STANDARD 14)

add_library(hello STATIC hello.cpp)
add_library(world SHARED world.cpp)
add_library(exclamation SHARED exclamationMark.cpp)

add_executable(libs main.cpp)
target_link_libraries(libs hello world -ldl)