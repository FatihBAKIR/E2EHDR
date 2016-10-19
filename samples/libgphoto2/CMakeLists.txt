cmake_minimum_required(VERSION 3.6)
project(camera)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)

pkg_check_modules(PC_LibGPhoto REQUIRED libgphoto2)

set(SOURCE_FILES main.cpp)
add_executable(camera ${SOURCE_FILES})

target_link_libraries(camera ${PC_LibGPhoto_LIBRARIES})
target_link_libraries(camera ${OPENGL_LIBRARIES})

target_include_directories(camera PUBLIC ${PC_LibGPhoto_INCLUDE_DIRS})
target_include_directories(camera PUBLIC ${OPENGL_INCLUDE_DIR})

target_compile_options(camera PUBLIC ${PC_LibGPhoto_CFLAGS_OTHER})

