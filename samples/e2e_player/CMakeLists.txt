cmake_minimum_required(VERSION 3.5)
project(e2e_player)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O0")

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../cmake_modules/")
LIST(APPEND CMAKE_MODULE_PATH "/usr/local/cmake_mod/")
link_directories(../libs/)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenGL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(GLFW REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread filesystem iostreams)

set(SOURCE_FILES Player.cpp Player.h main.cpp)

add_executable(e2e_player ${SOURCE_FILES})

target_compile_definitions(e2e_player PRIVATE GLFW_INCLUDE_NONE=1)

target_link_libraries(e2e_player ${OPENGL_LIBRARIES})
target_link_libraries(e2e_player ${OpenCV_LIBRARIES})
target_link_libraries(e2e_player ${GLFW_LIBRARIES})
target_link_libraries(e2e_player ${Boost_LIBRARIES})

target_include_directories(e2e_player PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(e2e_player PUBLIC ${OpenCV_INCLUDE_DIRS})
target_include_directories(e2e_player PUBLIC ${GLFW_INCLUDE_DIR})
target_include_directories(e2e_player PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(e2e_player PUBLIC ../../3rd_party/GSL)
target_include_directories(e2e_player PUBLIC "include")

target_link_libraries(e2e_player e2e_gl)
target_link_libraries(e2e_player e2e_utils)

target_include_directories(e2e_player PUBLIC ../e2e_gl)
target_include_directories(e2e_player PUBLIC ../e2e_utils)
