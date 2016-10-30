cmake_minimum_required(VERSION 3.6)
project(gl)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -O3 -pthread")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/local/cmake_modules/")

find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

set(SOURCE_FILES
    shaders/hdr.frag
    shaders/hdr.vert
    main.cpp
    pipeline.cpp
    pipeline.h
    quad.cpp
    quad.h Window.cpp Window.h)

add_executable(gl ${SOURCE_FILES})

target_link_libraries(gl ${OPENGL_LIBRARIES})
target_link_libraries(gl ${GLEW_LIBRARIES})
target_link_libraries(gl glfw)
target_link_libraries(gl ${Boost_LIBRARIES})

target_include_directories(gl PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(gl PUBLIC ${GLEW_INCLUDE_DIR})
target_include_directories(gl PUBLIC ${Boost_INCLUDE_DIRS})