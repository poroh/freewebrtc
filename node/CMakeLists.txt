#
# Copyright (c) 2023 Dmitry Poroh
# All rights reserved.
# Distributed under the terms of the MIT License. See the LICENSE file.
#

function(is_valid_napi_path path result)
  message(STATUS "Node.js: find: ${path}")
  if(EXISTS "${path}/include/node/node_api.h" AND EXISTS "${path}/include/node/openssl/err.h")
    set(${result} TRUE PARENT_SCOPE)
  else()
    set(${result} FALSE PARENT_SCOPE)
  endif()
endfunction()

set(NODE_JS_SEARCH_PATHS
  # Default installation of openssl
  "/usr"
  # Local installation of openssl
  "/usr/local"
)

if(NOT DEFINED NODE_JS_ROOT_DIR)
  set(default_node_js_path_is_valid FALSE)
else()
  is_valid_napi_path(${NODE_JS_ROOT_DIR} default_node_js_path_is_valid)
endif()

if(NOT default_openssl_path_is_valid)
  foreach (path IN LISTS NODE_JS_SEARCH_PATHS)
    is_valid_napi_path(${path} path_is_valid)
    if (path_is_valid)
      set(NODE_JS_ROOT_DIR "${path}")
      set(NODE_JS_FOUND 1)
      break()
    endif()
  endforeach()
else()
  set(NODE_JS_FOUND 1)
endif()

if(NODE_JS_FOUND)
  message(STATUS "Use node.js found at ${NODE_JS_ROOT_DIR}")
  set(SUBLIBS
    napi_wrapper
    node_stun
    node_ice
    openssl
  )

  foreach(sublib IN LISTS SUBLIBS)
    add_subdirectory(${sublib})
  endforeach()

else()
  message(STATUS "node.js is not found bindings building omitted")
endif()
