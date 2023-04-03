#
# Copyright (c) 2023 Dmitry Poroh
# All rights reserved.
# Distributed under the terms of the MIT License. See the LICENSE file.
#

set(SOURCES
    stun_attribute.cpp
    stun_attribute_set.cpp
    stun_message.cpp
    stun_password.cpp
    stun_method.cpp
    stun_address.cpp
    details/stun_fingerprint.cpp
)
file(GLOB HEADERS "*.hpp")

add_library(stun ${SOURCES} ${HEADERS})

