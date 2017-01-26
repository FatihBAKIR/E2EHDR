cmake_minimum_required(VERSION 3.6)
project(mpeghdr)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")

find_package(x264 REQUIRED)
find_package(FFmpeg REQUIRED)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp X264Encoder.cpp X264Encoder.h)
add_executable(mpeghdr ${SOURCE_FILES})

target_include_directories(mpeghdr PUBLIC ${X264_INCLUDE_DIR})
target_include_directories(mpeghdr PUBLIC ${FFMPEG_INCLUDE_DIRS})

target_link_libraries(mpeghdr ${X264_LIBRARIES})
target_link_libraries(mpeghdr ${FFMPEG_LIBRARIES})