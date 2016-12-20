cmake_minimum_required(VERSION 3.6)
project(e2e_control)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../cmake_modules/")
LIST(APPEND CMAKE_MODULE_PATH "/usr/local/cmake_mod/")

find_package(CURL REQUIRED)

set(CMAKE_CXX_STANDARD 14)

link_directories(/usr/local/lib)

set(SOURCE_FILES main.cpp)
add_executable(e2e_control ${SOURCE_FILES})

target_include_directories(e2e_control PUBLIC "/usr/local/include")

target_link_libraries(e2e_control curlcpp)
target_link_libraries(e2e_control ${CURL_LIBRARIES})