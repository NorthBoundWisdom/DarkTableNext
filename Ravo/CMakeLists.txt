cmake_minimum_required(VERSION 3.26)

project(Ravo VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)

# Ravo is configured as its own project so its headless targets never pull the
# frozen application's GTK, OpenCL, dynamic-module, or catalog build graph.
option(BUILD_TESTING "Build Ravo unit and contract tests" OFF)

find_package(Qt6 6.11 REQUIRED COMPONENTS Core)

function(ravo_configure_target target)
  target_compile_features(${target} PUBLIC cxx_std_20)
  target_compile_options(${target} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<CXX_COMPILER_ID:MSVC>:/EHsc>
    $<$<CXX_COMPILER_ID:MSVC>:/permissive->
    $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wextra>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wpedantic>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wconversion>
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wshadow>)
endfunction()

function(ravo_deploy_qt_core target)
  if(WIN32)
    add_custom_command(
      TARGET ${target}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:Qt6::Core>
        $<TARGET_FILE_DIR:${target}>
      VERBATIM)
  endif()
endfunction()

add_subdirectory(foundation)
add_subdirectory(recipe)
add_subdirectory(engine)
add_subdirectory(adapters)
add_subdirectory(cli)

if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(tests)
endif()

install(
  TARGETS ravo_foundation ravo_recipe ravo_engine ravo_adapters ravo_cli ravo
  EXPORT RavoTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(
  DIRECTORY
    foundation/include/
    recipe/include/
    engine/include/
    adapters/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  EXPORT RavoTargets
  FILE RavoTargets.cmake
  NAMESPACE Ravo::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Ravo)

if(WIN32)
  install(
    IMPORTED_RUNTIME_ARTIFACTS Qt6::Core
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
