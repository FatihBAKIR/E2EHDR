cmake_minimum_required(VERSION 3.6)
project(hdr_camera)

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../cmake_modules/")
LIST(APPEND CMAKE_MODULE_PATH "/usr/local/cmake_mod/")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

include_directories(${CMAKE_SOURCE_DIR})

link_directories(../libs/)

set(SOURCE_FILES main.cpp configuration.cpp configuration.h Application.cpp Application.h shader_stuff.h shader_stuff.cpp camera_struct.cpp camera_struct.h calibration.h calibration.cpp)
add_executable(e2e_camera ${SOURCE_FILES})

target_link_libraries(e2e_camera e2e_gl)
target_link_libraries(e2e_camera e2e_utils)
target_link_libraries(e2e_camera e2e_ffmpeg)
target_link_libraries(e2e_camera e2e_control)

target_include_directories(e2e_camera PUBLIC ../e2e_gl)
target_include_directories(e2e_camera PUBLIC ../e2e_utils)
target_include_directories(e2e_camera PUBLIC ../e2e_ffmpeg)
target_include_directories(e2e_camera PUBLIC ../e2e_control)

if (E2E_GPHOTO)
    target_link_libraries(e2e_camera e2e_gphoto)
    target_include_directories(e2e_camera PUBLIC ../e2e_gphoto)
endif()

# 3rd PARTY

target_include_directories(e2e_camera PUBLIC ../../3rd_party/GSL)
target_include_directories(e2e_camera PUBLIC ../../3rd_party/tfm)
target_include_directories(e2e_camera PUBLIC ../../3rd_party/json/src)
target_include_directories(e2e_camera PUBLIC ../../3rd_party/spdlog/include)

#GL LIBRARIES

find_package(OpenGL REQUIRED)
find_package(GLFW REQUIRED)
find_package(GLEW REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)

target_link_libraries(e2e_camera ${OPENGL_LIBRARIES})
target_link_libraries(e2e_camera ${GLEW_LIBRARIES})
target_link_libraries(e2e_camera ${GLFW_LIBRARIES})
target_link_libraries(e2e_camera ${Boost_LIBRARIES})
target_link_libraries(e2e_camera imgui)

target_include_directories(e2e_camera PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(e2e_camera PUBLIC ${GLEW_INCLUDE_DIR})
target_include_directories(e2e_camera PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(e2e_camera PUBLIC "../../3rd_party/imgui")

# GPHOTO LIBRARIES

if (E2E_GPHOTO)
find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)

pkg_check_modules(PC_LibGPhoto REQUIRED libgphoto2)

target_link_libraries(e2e_camera ${PC_LibGPhoto_LIBRARIES})
target_link_libraries(e2e_camera ${Boost_LIBRARIES})

target_include_directories(e2e_camera PUBLIC ${PC_LibGPhoto_INCLUDE_DIRS})
target_include_directories(e2e_camera PUBLIC ${Boost_INCLUDE_DIRS})

target_compile_options(e2e_camera PUBLIC ${PC_LibGPhoto_CFLAGS_OTHER})
endif()

# UTIL LIBRARIES

if (E2E_DECODE_JPEG)
    find_package(JPEG REQUIRED)
    target_link_libraries(e2e_camera ${JPEG_LIBRARIES})
    target_include_directories(e2e_camera PUBLIC ${JPEG_INCLUDE_DIR})
endif()

find_package(Boost REQUIRED COMPONENTS system thread)
find_package(OpenCV REQUIRED)

target_link_libraries(e2e_camera ${Boost_LIBRARIES})
target_link_libraries(e2e_camera ${OpenCV_LIBRARIES})

target_include_directories(e2e_camera PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(e2e_camera PUBLIC ${OpenCV_INCLUDE_DIRS})

# FFMPEG LIBRARIES

find_package(FFmpeg REQUIRED)

target_link_libraries(e2e_camera ${FFMPEG_LIBRARIES})

target_include_directories(e2e_camera PUBLIC ${FFMPEG_INCLUDE_DIRS})

# CONTROL LIBS

find_package(CURL REQUIRED)

target_include_directories(e2e_camera PUBLIC ${CURL_INCLUDE_DIRS})
target_include_directories(e2e_camera PUBLIC "../../3rd_party/cpr/include")

target_link_libraries(e2e_camera cpr)
target_link_libraries(e2e_camera ${CURL_LIBRARIES})