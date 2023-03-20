cmake_minimum_required(VERSION 3.0)

project(freewebrtc)

set(CMAKE_CXX_STANDARD 20)

set(freewebrtc_VERSION_MAJOR 1)
set(freewebrtc_VERSION_MINOR 0)

add_compile_options(-Werror -Wall -Wunused-variable -Wunused-parameter -pedantic)

set(RTP_SOURCES
    src/rtp/rtp_packet.cpp
    src/rtp/rtp_payload_map.cpp
)
file(GLOB RTP_HEADERS "src/rtp/*.hpp")


set(STUN_SOURCES
    src/stun/stun_message.cpp
    src/stun/stun_attribute.cpp
)
file(GLOB STUN_HEADERS "src/stun/*.hpp")

set(SOURCES
    ${RTP_SOURCES}
    ${STUN_SOURCES}
)

set(HEADERS
    ${RTP_HEADERS}
    ${STUN_HEADERS}
    )

add_library(freewebrtc STATIC ${SOURCES} ${HEADERS})

# Set include directories
target_include_directories(freewebrtc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Set library installation directory
install(TARGETS freewebrtc
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)

# Add tests
enable_testing()
add_subdirectory(tests)
