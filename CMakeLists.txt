cmake_minimum_required(VERSION 3.22)
project(branch-changer)
set(CMAKE_CXX_STANDARD 17)

add_library (branch STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/branch_misc.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/branch_utilities.cpp
)

target_include_directories(branch PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include/build
)

enable_testing()
add_subdirectory(tests)

option(DISABLE_GITHUB_WORKFLOW_WINDOWS "Enable GitHub Workflow for Windows" OFF)

if (DISABLE_GITHUB_WORKFLOW_WINDOWS)
    target_compile_definitions(branch PUBLIC GITHUB_WORKFLOW_WINDOWS)
endif()

