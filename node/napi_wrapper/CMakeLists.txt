#
# Copyright (c) 2023 Dmitry Poroh
# All rights reserved.
# Distributed under the terms of the MIT License. See the LICENSE file.
#

set(SOURCES
  napi_error.cpp
  napi_wrapper.cpp
)

file(GLOB HEADERS "*.hpp")

add_library(freewebrtc_napi_wrapper STATIC ${SOURCES} ${HEADERS})
target_include_directories(freewebrtc_napi_wrapper PUBLIC ${NODE_JS_ROOT_DIR}/include/node)
target_include_directories(freewebrtc_napi_wrapper PRIVATE ${CMAKE_SOURCE_DIR}/src)



