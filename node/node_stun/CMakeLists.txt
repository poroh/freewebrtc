#
# Copyright (c) 2023 Dmitry Poroh
# All rights reserved.
# Distributed under the terms of the MIT License. See the LICENSE file.
#

set(SOURCES
  node_stun_header.cpp
  node_stun_message.cpp
  node_stun_server_stateless.cpp
  node_stun_client_udp.cpp
  node_stun_client_udp_settings.cpp
)

file(GLOB HEADERS "*.hpp")

add_library(freewebrtc_node_stun STATIC ${SOURCES} ${HEADERS})
target_include_directories(freewebrtc_node_stun PUBLIC ${NODE_JS_ROOT_DIR}/include/node)
target_include_directories(freewebrtc_node_stun PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(freewebrtc_node_stun PRIVATE ${CMAKE_SOURCE_DIR})
