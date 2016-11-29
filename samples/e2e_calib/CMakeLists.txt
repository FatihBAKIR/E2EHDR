cmake_minimum_required(VERSION 3.6)
project(e2e_calib)
find_package(OpenCV REQUIRED)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(e2e_calib ${SOURCE_FILES})
target_link_libraries(e2e_calib ${OpenCV_LIBS})