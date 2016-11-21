cmake_minimum_required(VERSION 3.6)
project(e2e_crf)

find_package(OpenCV REQUIRED)
find_package(JPEG REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O0 -g")

set(SOURCE_FILES main.cpp e2e_crf.cpp e2e_crf.h)
#add_library(e2e_crf STATIC ${SOURCE_FILES})

link_directories(../libs/)

add_executable(e2e_crf ${SOURCE_FILES})

target_link_libraries(e2e_crf ${OpenCV_LIBS})
target_link_libraries(e2e_crf ${JPEG_LIBRARIES})

target_link_libraries(e2e_crf e2e_utils)

target_include_directories(e2e_crf PUBLIC "../e2e_utils/")
target_include_directories(e2e_crf PUBLIC "../../3rd_party/eigen/")
target_include_directories(e2e_crf PUBLIC "../../3rd_party/GSL/")
target_include_directories(e2e_crf PUBLIC ${JPEG_INCLUDES})
