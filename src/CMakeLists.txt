#
# Copyright (c) 2023 Dmitry Poroh
# All rights reserved.
# Distributed under the terms of the MIT License. See the LICENSE file.
#

set(SUBLIBS
    rtp
    crypto
    stun
)

add_library(freewebrtc INTERFACE)

foreach (sublib IN LISTS SUBLIBS)
    add_subdirectory(${sublib})
    if (TARGET ${sublib})
        target_link_libraries(freewebrtc INTERFACE ${sublib})
        target_include_directories(${sublib} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
endforeach()

if (TARGET crypto_openssl)
    add_library(freewebrtc_openssl INTERFACE)
    target_link_libraries(freewebrtc_openssl INTERFACE crypto_openssl)
    target_include_directories(crypto_openssl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
else()
    message(STATUS "OpenSSL not found bindings building omitted")
endif()





