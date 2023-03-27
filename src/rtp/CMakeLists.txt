#
# Copyright (c) 2023 Dmitry Poroh
# All rights reserved.
# Distributed under the terms of the MIT License. See the LICENSE file.
#

set(SOURCES
    rtp_packet.cpp
    rtp_payload_map.cpp
)
file(GLOB HEADERS "*.hpp")

add_library(rtp ${SOURCES} ${HEADERS})

