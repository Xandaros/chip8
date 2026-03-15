file(GLOB_RECURSE CORE_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/chip8_core/*.cpp")

add_library(libchip8)
target_link_libraries(libchip8 PRIVATE SDL3::SDL3)
target_sources(libchip8 PUBLIC ${CORE_FILES})
target_include_directories(libchip8 PUBLIC "${PROJECT_SOURCE_DIR}/src/chip8_core")
target_compile_options(libchip8 PRIVATE -Wall -Wold-style-cast)


file(GLOB_RECURSE APP_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/chip8_app/*.cpp")

add_executable(chip8)
target_link_libraries(chip8 PRIVATE libchip8)
target_sources(chip8 PRIVATE "${APP_FILES}")
target_compile_options(chip8 PRIVATE -Wall -Wold-style-cast)
