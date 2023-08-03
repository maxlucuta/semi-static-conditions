# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/main.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(
  payload_unittest
  payload_unittest.cpp
)
target_link_libraries(
  payload_unittest
  GTest::gtest_main
  payload
)

include(GoogleTest)
gtest_discover_tests(payload_unittest)