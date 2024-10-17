cmake_minimum_required(VERSION 3.21)

project(opalsdl)

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})

add_subdirectory(opal)

add_executable(opalsdl main.cpp)

target_link_libraries(opalsdl ${SDL2_LIBRARIES} opal)
