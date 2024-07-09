#
# Copyright (c) 2023 Dmitry Poroh
# All rights reserved.
# Distributed under the terms of the MIT License. See the LICENSE file.
#

set(SOURCES
  node_ice_candidate.cpp
)

file(GLOB HEADERS "*.hpp")

add_library(freewebrtc_node_ice STATIC ${SOURCES} ${HEADERS})
target_include_directories(freewebrtc_node_ice PUBLIC ${NODE_JS_ROOT_DIR}/include/node)
target_include_directories(freewebrtc_node_ice PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(freewebrtc_node_ice PRIVATE ${CMAKE_SOURCE_DIR})
target_include_directories(freewebrtc_node_ice PRIVATE ${CMAKE_BINARY_DIR}/craftpp/include)
target_include_directories(freewebrtc_node_ice PRIVATE ${CMAKE_BINARY_DIR}/craftnapi/include)

install(TARGETS freewebrtc_node_ice
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

install(FILES ${HEADERS} DESTINATION include/freewebrtc/node/node_ice)
