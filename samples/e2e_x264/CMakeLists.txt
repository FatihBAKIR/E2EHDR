cmake_minimum_required(VERSION 3.6)
project(mpeghdr)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules")

find_package(x264 REQUIRED)
find_package(FFmpeg REQUIRED)
find_package(Boost REQUIRED COMPONENTS system iostreams filesystem)
find_package(JPEG)

link_directories(../libs/)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp X264Encoder.cpp X264Encoder.h X264Decoder.cpp X264Decoder.hpp hdr_encode.cpp hdr_encode.hpp hdr_decode.cpp hdr_decode.hpp)
add_executable(mpeghdr ${SOURCE_FILES})

target_include_directories(mpeghdr PUBLIC ${X264_INCLUDE_DIR})
target_include_directories(mpeghdr PUBLIC ${FFMPEG_INCLUDE_DIRS})
target_include_directories(mpeghdr PUBLIC ../e2e_utils)
target_include_directories(mpeghdr PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries(mpeghdr ${X264_LIBRARIES})
target_link_libraries(mpeghdr ${FFMPEG_LIBRARIES})
target_link_libraries(mpeghdr ${Boost_LIBRARIES})

target_link_libraries(mpeghdr e2e_utils)

if (E2E_DECODE_JPEG)
    find_package(JPEG REQUIRED)
    target_link_libraries(mpeghdr ${JPEG_LIBRARIES})
    target_include_directories(mpeghdr PUBLIC ${JPEG_INCLUDE_DIR})
endif()
target_link_libraries(mpeghdr ${JPEG_LIBRARIES})
    target_include_directories(mpeghdr PUBLIC ${JPEG_INCLUDE_DIR})
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(OpenCV REQUIRED)

target_link_libraries(mpeghdr ${Boost_LIBRARIES})
target_link_libraries(mpeghdr ${OpenCV_LIBRARIES})

target_include_directories(mpeghdr PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(mpeghdr PUBLIC ${OpenCV_INCLUDE_DIRS})

