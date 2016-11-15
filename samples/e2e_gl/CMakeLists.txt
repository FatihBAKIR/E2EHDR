cmake_minimum_required(VERSION 3.5)
project(gl)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -O3 -pthread")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/local/cmake_modules/")

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

set(SOURCE_FILES
    shaders/hdr.frag
    shaders/hdr.vert
    pipeline.cpp
    pipeline.h
    quad.cpp
    quad.h Window.cpp Window.h
    #    main.cpp
        )

add_library(e2e_gl STATIC ${SOURCE_FILES})
#add_executable(e2e_gl ${SOURCE_FILES})

target_link_libraries(e2e_gl ${OPENGL_LIBRARIES})
target_link_libraries(e2e_gl ${GLEW_LIBRARIES})
target_link_libraries(e2e_gl glfw)
target_link_libraries(e2e_gl ${Boost_LIBRARIES})

target_include_directories(e2e_gl PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(e2e_gl PUBLIC ${GLEW_INCLUDE_DIR})
target_include_directories(e2e_gl PUBLIC ${Boost_INCLUDE_DIRS})
