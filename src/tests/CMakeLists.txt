include_directories("${CMAKE_CURRENT_BINARY_DIR}/../")

add_executable(darktable-test-variables variables.c)
target_link_libraries(darktable-test-variables lib_darktable)
add_test(NAME darktable-test-variables COMMAND darktable-test-variables)
set_tests_properties(
  darktable-test-variables
  PROPERTIES
    LABELS "unit;core"
    ENVIRONMENT
      "DARKTABLE_TEST_DATADIR=${CMAKE_BINARY_DIR}/share/darktable;DARKTABLE_TEST_MODULEDIR=${CMAKE_BINARY_DIR}/lib/darktable;DARKTABLE_TEST_CONFIGDIR=${CMAKE_CURRENT_BINARY_DIR}/runtime-config")

add_subdirectory(unittests)
