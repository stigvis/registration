cmake_minimum_required(VERSION 2.6)
project(hyperspec_registration C CXX)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes)

add_executable(hyperspec_registration
                main.cpp
                src/hyperspec.cpp
                src/multispec.cpp
                src/readimage.cpp
                src/registration.cpp
                src/rigid.cpp
                src/similarity.cpp
                src/affine.cpp
                src/bspline.cpp )
target_link_libraries(hyperspec_registration boost_regex matio ${ITK_LIBRARIES})
