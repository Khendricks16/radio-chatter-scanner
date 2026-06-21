cmake_minimum_required(VERSION 3.14)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


enable_testing()

# Include all of the C files that our GTest test files will use
add_library(
  rcs_test_source_files 
  ${CMAKE_SOURCE_DIR}/src/input.c
)
target_include_directories(rcs_test_source_files PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Executable for tests
add_executable(
  rcs_test
  test/input.cc
)
target_include_directories(rcs_test PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(
  rcs_test
  GTest::gtest_main
  rcs_test_source_files
)


include(GoogleTest)
gtest_discover_tests(rcs_test)
