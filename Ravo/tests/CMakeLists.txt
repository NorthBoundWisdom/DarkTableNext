find_package(GTest 1.17 CONFIG REQUIRED)
include(GoogleTest)

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
target_link_libraries(ravo_contract_tests PRIVATE GTest::gtest_main ravo_cli)
target_include_directories(ravo_contract_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
ravo_configure_target(ravo_contract_tests)
ravo_deploy_qt_core(ravo_contract_tests)
gtest_discover_tests(ravo_contract_tests PROPERTIES LABELS "ravo-contract")
