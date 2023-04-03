#
# Copyright (c) 2023 Dmitry Poroh
# All rights reserved.
# Distributed under the terms of the MIT License. See the LICENSE file.
#

set(SUBLIBS
    ip
)

add_library(net INTERFACE)

foreach (sublib IN LISTS SUBLIBS)
    add_subdirectory(${sublib})
    if (TARGET ${sublib})
        target_link_libraries(net INTERFACE ${sublib})
        target_include_directories(${sublib} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
    endif()
endforeach()
