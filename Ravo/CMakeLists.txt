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
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)

if(NOT DEFINED ENV{LIBRAW_SOURCE_ROOT})
  message(FATAL_ERROR "LIBRAW_SOURCE_ROOT is required; run the FreeCM source-root update")
endif()

file(TO_CMAKE_PATH "$ENV{LIBRAW_SOURCE_ROOT}" _ravo_libraw_root)
file(GLOB_RECURSE _ravo_libraw_sources CONFIGURE_DEPENDS
  "${_ravo_libraw_root}/src/*.cpp")
file(GLOB_RECURSE _ravo_libraw_placeholders CONFIGURE_DEPENDS
  "${_ravo_libraw_root}/src/*_ph.cpp")
list(REMOVE_ITEM _ravo_libraw_sources ${_ravo_libraw_placeholders})

add_library(ravo_libraw STATIC ${_ravo_libraw_sources})
target_include_directories(ravo_libraw SYSTEM PUBLIC "${_ravo_libraw_root}")
target_compile_definitions(ravo_libraw
  PUBLIC LIBRAW_NODLL
  PRIVATE LIBRAW_NOTHREADS USE_ZLIB)
target_compile_options(ravo_libraw PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/w>
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-w>)
target_link_libraries(ravo_libraw PRIVATE ZLIB::ZLIB)
set_target_properties(ravo_libraw PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(WIN32)
  target_compile_definitions(ravo_libraw PRIVATE
    LIBRAW_BUILDLIB
    _CRT_SECURE_NO_WARNINGS)
  target_link_libraries(ravo_libraw PRIVATE ws2_32)
elseif(UNIX)
  target_link_libraries(ravo_libraw PRIVATE m)
endif()

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
  TARGETS ravo_foundation ravo_recipe ravo_engine ravo_adapters ravo_cli ravo_libraw ravo
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

if(WIN32)
  install(
    DIRECTORY "$<TARGET_FILE_DIR:ravo>/"
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    FILES_MATCHING
      PATTERN "*.dll"
      PATTERN "CMakeFiles" EXCLUDE)
endif()
