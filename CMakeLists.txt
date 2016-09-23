cmake_minimum_required(VERSION 2.6)
project(hyperread_example C CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(hyperread_example main.cpp readimage.cpp)
target_link_libraries(hyperread_example boost_regex)
