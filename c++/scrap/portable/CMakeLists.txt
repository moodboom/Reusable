cmake_minimum_required(VERSION 3.8)
project(portable)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        scrap.cpp
        scrap.hpp)

add_executable(portable ${SOURCE_FILES})





