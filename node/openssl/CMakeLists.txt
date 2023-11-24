#
# Copyright (c) 2023 Dmitry Poroh
# All rights reserved.
# Distributed under the terms of the MIT License. See the LICENSE file.
#

set(SOURCES
  node_openssl_error.cpp
  node_openssl_hash.cpp
)

file(GLOB HEADERS "*.hpp")

add_library(freewebrtc_node_openssl STATIC ${SOURCES} ${HEADERS})
target_include_directories(freewebrtc_node_openssl PUBLIC ${NODE_JS_ROOT_DIR}/include/node)
target_include_directories(freewebrtc_node_openssl PRIVATE ${CMAKE_SOURCE_DIR}/src)
