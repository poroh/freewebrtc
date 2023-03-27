cmake_minimum_required(VERSION 3.0)

project(freewebrtc)

set(CMAKE_CXX_STANDARD 20)

set(freewebrtc_VERSION_MAJOR 1)
set(freewebrtc_VERSION_MINOR 0)

add_compile_options(-Werror -Wall -Wunused-variable -Wunused-parameter -pedantic)

add_subdirectory(src)

# Set library installation directory
install(TARGETS freewebrtc
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)

# Add tests
enable_testing()
add_subdirectory(tests)

