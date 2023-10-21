#
# Copyright (c) 2023 Dmitry Poroh
# All rights reserved.
# Distributed under the terms of the MIT License. See the LICENSE file.
#

function (is_valid_openssl_path path result)
    if(EXISTS "${path}/include/openssl/ssl.h" AND EXISTS "${path}/lib/libssl.dylib")
        set(${result} TRUE PARENT_SCOPE)
    else()
        set(${result} FALSE PARENT_SCOPE)
    endif()
endfunction()

set(OPENSSL_SEARCH_PATHS
    # Default installation of openssl
    "/usr"
    # Local installation of openssl
    "/usr/local"
    # Brew-installed openssl:
    "/usr/local/opt/openssl"
    "/usr/local/opt/openssl@1.1"
    "/usr/local/opt/openssl@3.0"
)

if (NOT DEFINED OPENSSL_ROOT_DIR)
    set(default_openssl_path_is_valid FALSE)
else()
    is_valid_openssl_path(${OPENSSL_ROOT_DIR} default_openssl_path_is_valid)
endif()

if (NOT default_openssl_path_is_valid)
    foreach (path IN LISTS OPENSSL_SEARCH_PATHS)
        is_valid_openssl_path(${path} path_is_valid)
        if (path_is_valid)
            set(OPENSSL_ROOT_DIR "${path}")
            break()
        endif()
    endforeach()
endif()

find_package(OpenSSL QUIET)

if(OPENSSL_FOUND)
   set(SOURCES
       openssl_hash.cpp
       openssl_error.cpp
   )

   file(GLOB HEADERS "*.hpp")

   add_library(freewebrtc_openssl STATIC ${SOURCES} ${HEADERS})
   target_include_directories(freewebrtc_openssl PUBLIC ${OPENSSL_ROOT_DIR}/include)
   target_include_directories(freewebrtc_openssl PRIVATE ${CMAKE_SOURCE_DIR}/src)
else()
   message(STATUS "OpenSSL not found bindings building omitted")
endif()



