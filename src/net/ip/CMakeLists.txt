#
# Copyright (c) 2023 Dmitry Poroh
# All rights reserved.
# Distributed under the terms of the MIT License. See the LICENSE file.
#

set(SOURCES
    ip_address.cpp
    ip_address_hash.cpp
)
file(GLOB HEADERS "*.hpp")
file(GLOB DETAILS_HEADERS "details/*.hpp")

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

target_sources(freewebrtc PRIVATE ${SOURCES} ${HEADERS})

install(FILES ${HEADERS} DESTINATION include/freewebrtc/net/ip)
install(FILES ${DETAILS_HEADERS} DESTINATION include/freewebrtc/net/ip/details)

