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
    details/stun_fingerprint.cpp
)
file(GLOB HEADERS "*.hpp")

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

target_sources(freewebrtc PRIVATE ${SOURCES} ${HEADERS})


