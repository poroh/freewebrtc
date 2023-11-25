#
# Copyright (c) 2023 Dmitry Poroh
# All rights reserved.
# Distributed under the terms of the MIT License. See the LICENSE file.
#

set(SUBLIBS
    ip
)

foreach (sublib IN LISTS SUBLIBS)
    add_subdirectory(${sublib})
endforeach()

set(SOURCES
    net_error.cpp
)
file(GLOB HEADERS "*.hpp")

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

target_sources(freewebrtc PRIVATE ${SOURCES} ${HEADERS})

install(FILES ${HEADERS} DESTINATION include/freewebrtc/net)

