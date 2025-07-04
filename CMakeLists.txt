cmake_minimum_required(VERSION 3.20)
project(schedulerApp VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(cppautomemoryleakfinder main.cpp)

include(CTest)
enable_testing()

