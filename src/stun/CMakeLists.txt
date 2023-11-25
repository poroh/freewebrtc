#
# Copyright (c) 2023 Dmitry Poroh
# All rights reserved.
# Distributed under the terms of the MIT License. See the LICENSE file.
#

set(SOURCES
    stun_error.cpp
    stun_header.cpp
    stun_attribute.cpp
    stun_attribute_set.cpp
    stun_message.cpp
    stun_password.cpp
    stun_method.cpp
    stun_address.cpp
    stun_server_stateless.cpp
    stun_client_udp.cpp
    details/stun_fingerprint.cpp
    details/stun_client_udp_rto.cpp
)
file(GLOB HEADERS "*.hpp")
file(GLOB DETAILS_HEADERS "details/*.hpp")

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

target_sources(freewebrtc PRIVATE ${SOURCES} PUBLIC ${HEADERS} ${DETAILS_HEADERS})

install(FILES ${HEADERS} DESTINATION include/freewebrtc/stun)
install(FILES ${DETAILS_HEADERS} DESTINATION include/freewebrtc/stun/details)
