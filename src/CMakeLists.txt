add_executable(chip8)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")

target_sources(chip8 PRIVATE ${SRC_FILES})

target_link_libraries(chip8 PRIVATE vendor)
