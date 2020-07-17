cmake_minimum_version(VERSION 3.10)

#set project name
project(IASLib VERSION 2.0)

configure_file(IASLibConfig.h.in IASLibConfig.h)

target_include_directories(IASLib PUBLIC "${PROJECT_BINARY_DIR}/inc" )

#add library file
add_library(IASLib)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
