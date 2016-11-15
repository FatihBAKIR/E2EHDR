cmake_minimum_required(VERSION 3.6)
project(e2e_onvif)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(e2e_onvif ${SOURCE_FILES})