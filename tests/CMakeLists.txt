
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest
  GIT_TAG main
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


add_executable(
  branch_test
  branch_test.cpp
)
target_link_libraries(
  branch_test
  GTest::gtest_main
  branch
)

include(GoogleTest)
gtest_discover_tests(branch_test)