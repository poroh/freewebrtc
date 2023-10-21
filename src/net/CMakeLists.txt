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
