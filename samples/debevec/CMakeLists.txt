cmake_minimum_required(VERSION 2.8)
project(debevec)

find_package(OpenCV REQUIRED)
find_package(JPEG REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++14")

include_directories("Header")

set(SOURCE_FILES Source/debevec.cpp Header/debevec.h)
add_library(debevec STATIC ${SOURCE_FILES})

target_link_libraries(debevec ${OpenCV_LIBS})
target_link_libraries(debevec ${JPEG_LIBRARIES})

target_include_directories(debevec PUBLIC "../../3rd_party/eigen/")
target_include_directories(debevec PUBLIC "../../3rd_party/GSL/")
target_include_directories(debevec PUBLIC ${JPEG_INCLUDES})