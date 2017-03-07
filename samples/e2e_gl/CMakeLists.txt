cmake_minimum_required(VERSION 3.5)
project(gl)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../cmake_modules/")
LIST(APPEND CMAKE_MODULE_PATH "/usr/local/cmake_mod/")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O0 -pthread")

set(CMAKE_CXX_STANDARD 14)
link_directories(../libs/)

find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(GLFW REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

set(SOURCE_FILES
        shaders/preview.frag
    shaders/hdr.vert
        quad.cpp
        src/glad.c
    quad.h Window.cpp Window.h glsl_program.h glsl_program.cpp gui.cpp gui.h
        imgui_wrapper.cpp imgui_wrapper.h texture.cpp texture.h
        Drawable.h merger.cpp merger.h framebuffer.cpp framebuffer.h
        projector_main.cpp)

#add_library(e2e_gl STATIC ${SOURCE_FILES})
add_executable(e2e_gl ${SOURCE_FILES})

target_link_libraries(e2e_gl ${OPENGL_LIBRARIES})
target_link_libraries(e2e_gl ${OpenCV_LIBRARIES})
target_link_libraries(e2e_gl ${GLFW_LIBRARIES})
target_link_libraries(e2e_gl ${Boost_LIBRARIES})
target_link_libraries(e2e_gl imgui)

target_include_directories(e2e_gl PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(e2e_gl PUBLIC ${OpenCV_INCLUDE_DIRS})
target_include_directories(e2e_gl PUBLIC ${GLFW_INCLUDE_DIR})
target_include_directories(e2e_gl PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(e2e_gl PUBLIC "../../3rd_party/imgui")
target_include_directories(e2e_gl PUBLIC "include")
