#
# Copyright (c) 2023 Dmitry Poroh
# All rights reserved.
# Distributed under the terms of the MIT License. See the LICENSE file.
#

set(SOURCES
    util_hash_murmur.cpp
    util_binary_view.cpp
    util_token_stream.cpp
    util_error.cpp
    util_error_code.cpp
)
file(GLOB HEADERS "*.hpp")

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

target_sources(freewebrtc PRIVATE ${SOURCES} ${HEADERS})

install(FILES ${HEADERS} DESTINATION include/freewebrtc/util)
