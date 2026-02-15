cmake_minimum_required(VERSION 3.24)
project(chip8)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(vendor EXCLUDE_FROM_ALL)
add_subdirectory(src)
add_subdirectory(tests)
