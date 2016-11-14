cmake_minimum_required(VERSION 3.6)
project(e2e_utils)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

find_package(JPEG REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)

include_directories(.)

set(SOURCE_FILES
        profiler/profiler.h profiler/hash.h profiler/profiler.cpp
        jpeg/jpeg_decode.cpp jpeg/jpeg_decode.h
        Frame.h
        util.h
        spsc/spsc_queue.h)

add_library(e2e_utils STATIC ${SOURCE_FILES})

target_link_libraries(e2e_utils ${JPEG_LIBRARIES})
target_link_libraries(e2e_utils ${Boost_LIBRARIES})

target_include_directories(e2e_utils PUBLIC ${JPEG_INCLUDE_DIR})
target_include_directories(e2e_utils PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(e2e_utils PUBLIC ../../3rd_party/GSL)