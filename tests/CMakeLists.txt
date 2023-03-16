
set(TEST_NAME FreeWebRTCTests)

# Add test source files
set(TEST_SOURCES
    RTPParseTests.cpp
)

# Download and setup Google Test framework
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)
FetchContent_MakeAvailable(googletest)

# Create the test executable
add_executable(${TEST_NAME} ${TEST_SOURCES})

# Link test executable with library and testing framework
target_link_libraries(${TEST_NAME} PRIVATE FreeWebRTC)
target_link_libraries(${TEST_NAME} PRIVATE gtest_main)

# Add test to CTest
include(GoogleTest)
gtest_discover_tests(${TEST_NAME})
