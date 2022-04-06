cmake_minimum_required(VERSION 3.21)
project(union)

ADD_LIBRARY(LibsModule
        rational.cpp
        helpers.cpp)

set(CMAKE_CXX_STANDARD 14)
include_directories(include)
link_directories(lib)

target_link_libraries(LibsModule -lgurobi_g++5.2)
target_link_libraries(LibsModule -lgurobi91)

add_executable(union main.cpp helpers.cpp model.cpp model.h)
target_link_libraries(union LibsModule)
