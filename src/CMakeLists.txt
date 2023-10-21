#
# Copyright (c) 2023 Dmitry Poroh
# All rights reserved.
# Distributed under the terms of the MIT License. See the LICENSE file.
#

set(SUBLIBS
    rtp
    crypto
    stun
    net
)

add_library(freewebrtc)

foreach (sublib IN LISTS SUBLIBS)
    add_subdirectory(${sublib})
endforeach()




