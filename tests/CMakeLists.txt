
set(TEST_NAME freewebrtc_tests)

set(TEST_SOURCES
    rtp_parse_tests.cpp
    rtp_timestamp_tests.cpp
    crypto_hmac_openssl_tests.cpp
    stun_parse_tests.cpp
)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(${TEST_NAME} ${TEST_SOURCES})

# Link test executable with library and testing framework
target_link_libraries(${TEST_NAME} PRIVATE freewebrtc)
target_link_libraries(${TEST_NAME} PRIVATE gtest_main)
target_link_libraries(${TEST_NAME} PRIVATE freewebrtc_openssl)

find_package(OpenSSL REQUIRED)
target_link_libraries(${TEST_NAME} PUBLIC OpenSSL::Crypto)

include(GoogleTest)
gtest_discover_tests(${TEST_NAME})
