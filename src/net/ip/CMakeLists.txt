#
# Copyright (c) 2023 Dmitry Poroh
# All rights reserved.
# Distributed under the terms of the MIT License. See the LICENSE file.
#

set(SOURCES
    ip_address.cpp
)
file(GLOB HEADERS "*.hpp")

add_library(ip ${SOURCES} ${HEADERS})

