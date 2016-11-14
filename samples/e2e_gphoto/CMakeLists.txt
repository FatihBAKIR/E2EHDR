cmake_minimum_required(VERSION 3.6)
project(camera)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -O3 -g -pthread")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "/usr/local/cmake_modules/")

find_package(PkgConfig REQUIRED)
find_package(JPEG REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)

pkg_check_modules(PC_LibGPhoto REQUIRED libgphoto2)

link_directories(../libs/)

set(SOURCE_FILES gphoto/gphoto.cpp gphoto/gphoto.h)
add_library(e2e_gphoto STATIC ${SOURCE_FILES})

target_link_libraries(e2e_gphoto ${PC_LibGPhoto_LIBRARIES})
target_link_libraries(e2e_gphoto ${Boost_LIBRARIES})

target_link_libraries(e2e_gphoto e2e_utils)

target_include_directories(e2e_gphoto PUBLIC ${PC_LibGPhoto_INCLUDE_DIRS})
target_include_directories(e2e_gphoto PUBLIC ${Boost_INCLUDE_DIRS})

target_include_directories(e2e_gphoto PUBLIC ../../3rd_party/GSL)
target_include_directories(e2e_gphoto PUBLIC ../../3rd_party/tfm)
target_include_directories(e2e_gphoto PUBLIC ../../3rd_party/spdlog/include)

target_include_directories(e2e_gphoto PUBLIC ../e2e_utils)

target_compile_options(e2e_gphoto PUBLIC ${PC_LibGPhoto_CFLAGS_OTHER})
