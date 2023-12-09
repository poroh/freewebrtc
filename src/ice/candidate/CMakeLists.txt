#
# Copyright (c) 2023 Dmitry Poroh
# All rights reserved.
# Distributed under the terms of the MIT License. See the LICENSE file.
#

set(SOURCES
  ice_candidate_error.cpp
  ice_candidate_sdp.cpp
  ice_candidate_type.cpp
  ice_candidate_foundation.cpp
  ice_candidate_component_id.cpp
)
file(GLOB HEADERS "*.hpp")

list(TRANSFORM SOURCES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

target_sources(freewebrtc PRIVATE ${SOURCES} ${HEADERS})

install(FILES ${HEADERS} DESTINATION include/freewebrtc/ice/candidate)

