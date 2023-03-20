cmake_minimum_required(VERSION 3.0)

project(FreeWebRTC)

set(CMAKE_CXX_STANDARD 20)

set(FreeWebRTC_VERSION_MAJOR 1)
set(FreeWebRTC_VERSION_MINOR 0)

add_compile_options(-Werror -Wall -Wunused-variable -Wunused-parameter -pedantic)

set(RTP_SOURCES
    src/rtp/RTPPacket.cpp
    src/rtp/RTPPayloadMap.cpp
)
file(GLOB RTP_HEADERS "src/rtp/*.hpp")

set(SOURCES
    ${RTP_SOURCES}
)

set(HEADERS
    ${RTP_HEADERS}
    )

add_library(FreeWebRTC STATIC ${SOURCES} ${HEADERS})

# Set include directories
target_include_directories(FreeWebRTC PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Set library installation directory
install(TARGETS FreeWebRTC
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)

# Add tests
enable_testing()
add_subdirectory(tests)
