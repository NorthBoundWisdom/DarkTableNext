find_package(GTest 1.17 CONFIG REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
include(GoogleTest)

function(ravo_add_phase0_check name script)
  add_test(
    NAME ${name}
    COMMAND "${Python3_EXECUTABLE}" "${PROJECT_SOURCE_DIR}/tools/${script}"
      --repository-root "${PROJECT_SOURCE_DIR}/.." ${ARGN})
  set_tests_properties(${name} PROPERTIES LABELS "ravo-contract")
endfunction()

add_executable(ravo_unit_tests
  foundation_test.cpp
  recipe_test.cpp)
target_link_libraries(ravo_unit_tests PRIVATE GTest::gtest_main ravo_recipe)
target_include_directories(ravo_unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
ravo_configure_target(ravo_unit_tests)
gtest_discover_tests(ravo_unit_tests PROPERTIES LABELS "ravo-unit")

add_executable(ravo_contract_tests
  cli_test.cpp
  engine_test.cpp)
target_link_libraries(ravo_contract_tests PRIVATE GTest::gtest_main PNG::PNG ravo_cli)
target_include_directories(ravo_contract_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(ravo_contract_tests PRIVATE
  RAVO_REPOSITORY_ROOT="${PROJECT_SOURCE_DIR}/..")
ravo_configure_target(ravo_contract_tests)
ravo_deploy_qt_core(ravo_contract_tests)
gtest_discover_tests(ravo_contract_tests PROPERTIES LABELS "ravo-contract")

ravo_add_phase0_check(ravo_fixture_manifest freeze_legacy_manifest.py)
ravo_add_phase0_check(ravo_capability_inventory check_capability_inventory.py)
ravo_add_phase0_check(ravo_dependency_boundary check_ravo_dependency_boundary.py)
ravo_add_phase0_check(ravo_vertical_slice_plan check_vertical_slice_plan.py)
ravo_add_phase0_check(ravo_fixture_classification_ledger check_fixture_classification_ledger.py)
ravo_add_phase0_check(ravo_freeze_reference check_freeze_reference.py)
