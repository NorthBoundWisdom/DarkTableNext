add_library(ravo_foundation STATIC)

target_sources(ravo_foundation
  PRIVATE
    src/cancellation.cpp
    src/error.cpp
    src/json.cpp)

target_include_directories(ravo_foundation
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

ravo_configure_target(ravo_foundation)
