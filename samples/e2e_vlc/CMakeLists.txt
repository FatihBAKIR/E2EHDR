cmake_minimum_required(VERSION 3.6)
project(e2e_vlc)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

link_directories(/usr/local/lib/vlc)
link_directories(../libs)

set(SOURCE_FILES main.cpp vlc/camera.cpp vlc/camera.h)
add_executable(e2e_vlc ${SOURCE_FILES})

target_include_directories(e2e_vlc PUBLIC /usr/local/include/vlc)
target_include_directories(e2e_vlc PUBLIC ../e2e_utils)

target_link_libraries(e2e_vlc vlc)
target_link_libraries(e2e_vlc e2e_utils)