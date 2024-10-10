cmake_minimum_required(VERSION 3.21)

project(OPALSDL)

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})

add_subdirectory(opal)

add_executable(OPALSDL main.cpp)

target_link_libraries(OPALSDL ${SDL2_LIBRARIES} opal)
