cmake_minimum_required(VERSION 3.5)
project(e2e_webcam)
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../cmake_modules/")
set(CMAKE_CXX_STANDARD 14)

find_package(OpenCV REQUIRED)
find_package(UVC REQUIRED)

set(SOURCE_FILES e2e_uvc/camera.cpp e2e_uvc/camera.hpp)
add_library(e2e_webcam STATIC ${SOURCE_FILES})
add_executable(webcam_test main.cpp)

target_include_directories(e2e_webcam PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(e2e_webcam ${OpenCV_LIBRARIES})

target_include_directories(e2e_webcam PUBLIC ${uvc_INCLUDE_DIRS})
target_link_libraries(e2e_webcam ${uvc_LIBRARIES})

target_include_directories(e2e_webcam PUBLIC ../e2e_utils)

target_link_libraries(webcam_test e2e_webcam)