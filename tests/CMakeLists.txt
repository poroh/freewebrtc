
set(TEST_NAME FreeWebRTCTests)

set(TEST_SOURCES
    RTPParseTests.cpp
    RTPTimestampTest.cpp
)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(${TEST_NAME} ${TEST_SOURCES})

# Link test executable with library and testing framework
target_link_libraries(${TEST_NAME} PRIVATE FreeWebRTC)
target_link_libraries(${TEST_NAME} PRIVATE gtest_main)

include(GoogleTest)
gtest_discover_tests(${TEST_NAME})
