cmake_minimum_required(VERSION 3.6)
project(e2e_ffmpeg)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} /usr/local/cmake_mod/)
find_package(FFmpeg REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(SOURCE_FILES main.cpp)
add_executable(e2e_ffmpeg ${SOURCE_FILES})

target_link_libraries(e2e_ffmpeg ${FFMPEG_LIBRARIES})

target_include_directories(e2e_ffmpeg PUBLIC ${FFMPEG_INCLUDE_DIRS})