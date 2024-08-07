cmake_minimum_required(VERSION 3.0)

project(freewebrtc)

set(CMAKE_CXX_STANDARD 20)

option(CODE_COVERAGE "Enable coverage reporting" OFF)
option(ADDRESS_SANITIZER "Enable address sanitizer" OFF)

set(freewebrtc_VERSION_MAJOR 1)
set(freewebrtc_VERSION_MINOR 0)

add_compile_options(-Werror -Wall -Wunused-variable -Wunused-parameter -pedantic)

if(ADDRESS_SANITIZER)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -g")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
  endif()
endif()

if(CODE_COVERAGE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0  -fprofile-instr-generate -fcoverage-mapping")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0  -fprofile-instr-generate -fcoverage-mapping")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -O0  -fprofile-instr-generate -fcoverage-mapping")
  endif()
endif()

# ================================================================================
include(ExternalProject)
ExternalProject_Add(
  craftpp
  GIT_REPOSITORY https://github.com/poroh/craftpp.git
  GIT_TAG        main
  CMAKE_ARGS     -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/craftpp
)

include(ExternalProject)
ExternalProject_Add(
  craftnapi
  GIT_REPOSITORY https://github.com/poroh/craftnapi.git
  GIT_TAG        main
  CMAKE_ARGS     -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/craftnapi
)

# ================================================================================

add_subdirectory(src)
add_subdirectory(node)

target_include_directories(freewebrtc PRIVATE ${CMAKE_SOURCE_DIR}/src)

install(TARGETS freewebrtc
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

# Add tests
enable_testing()
add_subdirectory(tests)

