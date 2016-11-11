cmake_minimum_required(VERSION 2.6)
project(hyperspec_registration C CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(hyperspec_registration main.cpp hyperspec_read.cpp readimage.cpp)
target_link_libraries(hyperspec_registration boost_regex matio tiff)
