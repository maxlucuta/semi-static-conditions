cmake_minimum_required(VERSION 3.22)
project(branch-changer)

add_library (branch STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/utilities.cpp
)
target_include_directories(branch
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

enable_testing()
add_subdirectory(tests)