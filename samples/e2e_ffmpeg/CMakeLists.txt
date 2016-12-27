cmake_minimum_required(VERSION 3.6)
project(e2e_ffmpeg)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} /usr/local/cmake_mod/)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake_modules/)
find_package(FFmpeg REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)

set(CMAKE_CXX_STANDARD 14)

link_directories(../libs)

set(SOURCE_FILES e2e_ff/ffmpeg_wrapper.cpp e2e_ff/ffmpeg_wrapper.h)
add_library(e2e_ffmpeg STATIC ${SOURCE_FILES})

target_link_libraries(e2e_ffmpeg ${FFMPEG_LIBRARIES})
target_link_libraries(e2e_ffmpeg ${Boost_LIBRARIES})

target_include_directories(e2e_ffmpeg PUBLIC ${FFMPEG_INCLUDE_DIRS})
target_include_directories(e2e_ffmpeg PUBLIC ${Boost_INCLUDE_DIRS})

target_include_directories(e2e_ffmpeg PUBLIC ../e2e_utils)
target_link_libraries(e2e_ffmpeg e2e_utils)