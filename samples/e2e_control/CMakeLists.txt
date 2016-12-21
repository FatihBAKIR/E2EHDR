cmake_minimum_required(VERSION 3.6)
project(e2e_control)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../cmake_modules/")
LIST(APPEND CMAKE_MODULE_PATH "/usr/local/cmake_mod/")

find_package(CURL REQUIRED)

set(CMAKE_CXX_STANDARD 14)

link_directories(../libs)
link_directories(/usr/local/lib)

set(SOURCE_FILES camera_control.cpp camera_control.h)
#add_executable(e2e_control ${SOURCE_FILES})
add_library(e2e_control STATIC ${SOURCE_FILES})

target_include_directories(e2e_control PUBLIC ${CURL_INCLUDE_DIRS})
target_include_directories(e2e_control PUBLIC "../../3rd_party/cpr/include")

target_link_libraries(e2e_control cpr)
target_link_libraries(e2e_control ${CURL_LIBRARIES})