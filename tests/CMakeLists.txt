
set(TEST_NAME freewebrtc_tests)

set(TEST_SOURCES
    rtp_parse_tests.cpp
    rtp_timestamp_tests.cpp
    crypto_hmac_openssl_tests.cpp
    stun_parse_tests.cpp
    stun_build_tests.cpp
    stun_server_stateless_tests.cpp
    stun_client_udp_tests.cpp
    util_return_value_tests.cpp
    util_intrusive_list_tests.cpp
    util_token_stream_tests.cpp
    net_fqdn_tests.cpp
    net_port_tests.cpp
    clock_timepoint_tests.cpp
    ice_candidate_type_tests.cpp
    ice_candidate_foundation_tests.cpp
    ice_candidate_component_id_tests.cpp
    ice_candidate_transport_type_tests.cpp
    ice_candidate_priority_tests.cpp
    ice_candidate_address_tests.cpp
    ice_candidate_sdp_tests.cpp
)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
)
FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

add_executable(${TEST_NAME} ${TEST_SOURCES})
target_include_directories(${TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Link test executable with library and testing framework
target_link_libraries(${TEST_NAME} PRIVATE freewebrtc)
target_link_libraries(${TEST_NAME} PRIVATE gtest_main)
target_link_libraries(${TEST_NAME} PRIVATE freewebrtc_openssl)

find_package(OpenSSL REQUIRED)
target_link_libraries(${TEST_NAME} PUBLIC OpenSSL::Crypto)

include(GoogleTest)
gtest_discover_tests(${TEST_NAME})
