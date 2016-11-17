cmake_minimum_required(VERSION 2.6)
project(hyperspec_registration C CXX)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes)

add_executable(hyperspec_registration
                main.cpp
                src/hyperspec_read.cpp
                src/readimage.cpp
                src/registration.cpp
                src/reg1.cpp
                src/reg2.cpp
                src/regAffine.cpp
                src/regMask.cpp )
target_link_libraries(hyperspec_registration boost_regex matio tiff ${ITK_LIBRARIES})
