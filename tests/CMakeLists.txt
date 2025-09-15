include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(TEST_FILES
    helloWorldTest.C
    test.C
)

# Now simply link against gtest or gtest_main as needed. Eg
add_executable(OpenFOAM_test ${TEST_FILES})
target_include_directories(OpenFOAM_test PRIVATE ./)
target_link_libraries(OpenFOAM_test gtest_main OpenFOAM)

add_test(NAME OpenFOAM_test_test COMMAND OpenFOAM_test)

include(GoogleTest)
gtest_discover_tests(OpenFOAM_test)
