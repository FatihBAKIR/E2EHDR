cmake_minimum_required(VERSION 3.6)
project(camera)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -O3")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/local/cmake_modules/")

find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
find_package(JPEG REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost REQUIRED COMPONENTS date_time filesystem system thread)

pkg_check_modules(PC_LibGPhoto REQUIRED libgphoto2)

set(SOURCE_FILES main.cpp gphoto.cpp gphoto_wrapper.h util.h jpeg_decode.cpp profiler.cpp profiler.h hash.h)
add_executable(camera ${SOURCE_FILES})

target_link_libraries(camera ${PC_LibGPhoto_LIBRARIES})
target_link_libraries(camera ${OPENGL_LIBRARIES})
target_link_libraries(camera ${OpenCV_LIBS})
target_link_libraries(camera ${JPEG_LIBRARIES})
target_link_libraries(camera ${Boost_LIBRARIES})

target_include_directories(camera PUBLIC ${PC_LibGPhoto_INCLUDE_DIRS})
target_include_directories(camera PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(camera PUBLIC ${OPENCV_INCLUDE_DIR})
target_include_directories(camera PUBLIC ${JPEG_INCLUDE_DIR})
target_include_directories(camera PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(camera PUBLIC ../../3rd_party/GSL)
target_include_directories(camera PUBLIC ../../3rd_party/tfm)
target_include_directories(camera PUBLIC ../../3rd_party/spdlog/include)

target_compile_options(camera PUBLIC ${PC_LibGPhoto_CFLAGS_OTHER})
