cmake_minimum_required(VERSION 3.6)
project(e2e_calib)
find_package(OpenCV REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
set(STEREO_SOURCES stereo_calib.cpp)

add_executable(e2e_calib ${SOURCE_FILES})
add_executable(e2e_stereo ${STEREO_SOURCES})

target_link_libraries(e2e_calib ${OpenCV_LIBS})
target_link_libraries(e2e_stereo ${OpenCV_LIBS})