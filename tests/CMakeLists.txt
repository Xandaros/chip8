add_executable(tests)

file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/tests/*.cpp")
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM SRC_FILES "${PROJECT_SOURCE_DIR}/src/main.cpp")

target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)
target_link_libraries(tests PRIVATE libchip8)

target_sources(tests PRIVATE ${TEST_FILES})
