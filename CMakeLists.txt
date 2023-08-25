cmake_minimum_required(VERSION 3.22)
project(branch-changer)


add_library (branch STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/branch_misc.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/branch_utilities.cpp
)

target_include_directories(branch
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/build
)

enable_testing()
add_subdirectory(tests)