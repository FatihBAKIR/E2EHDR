cmake_minimum_required(VERSION 3.6)
project(hdr_camera)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/local/cmake_modules/")

link_directories(../libs/)

set(SOURCE_FILES main.cpp)
add_executable(e2e_camera ${SOURCE_FILES})

target_link_libraries(e2e_camera e2e_gl)
target_link_libraries(e2e_camera e2e_utils)
target_link_libraries(e2e_camera e2e_gphoto)

target_include_directories(e2e_camera PUBLIC ../e2e_gl)
target_include_directories(e2e_camera PUBLIC ../e2e_utils)
target_include_directories(e2e_camera PUBLIC ../e2e_gphoto)

# 3rd PARTY

target_include_directories(e2e_camera PUBLIC ../../3rd_party/GSL)
target_include_directories(e2e_camera PUBLIC ../../3rd_party/tfm)
target_include_directories(e2e_camera PUBLIC ../../3rd_party/spdlog/include)

#GL LIBRARIES

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)

target_link_libraries(e2e_camera ${OPENGL_LIBRARIES})
target_link_libraries(e2e_camera ${GLEW_LIBRARIES})
target_link_libraries(e2e_camera glfw)
target_link_libraries(e2e_camera ${Boost_LIBRARIES})

target_include_directories(e2e_camera PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(e2e_camera PUBLIC ${GLEW_INCLUDE_DIR})
target_include_directories(e2e_camera PUBLIC ${Boost_INCLUDE_DIRS})

# GPHOTO LIBRARIES

find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)

pkg_check_modules(PC_LibGPhoto REQUIRED libgphoto2)

target_link_libraries(e2e_camera ${PC_LibGPhoto_LIBRARIES})
target_link_libraries(e2e_camera ${Boost_LIBRARIES})

target_include_directories(e2e_camera PUBLIC ${PC_LibGPhoto_INCLUDE_DIRS})
target_include_directories(e2e_camera PUBLIC ${Boost_INCLUDE_DIRS})

target_compile_options(e2e_camera PUBLIC ${PC_LibGPhoto_CFLAGS_OTHER})

# UTIL LIBRARIES

find_package(JPEG REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)

target_link_libraries(e2e_camera ${JPEG_LIBRARIES})
target_link_libraries(e2e_camera ${Boost_LIBRARIES})

target_include_directories(e2e_camera PUBLIC ${JPEG_INCLUDE_DIR})
target_include_directories(e2e_camera PUBLIC ${Boost_INCLUDE_DIRS})