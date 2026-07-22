add_library(ravo_adapters STATIC)

target_sources(ravo_adapters
  PRIVATE
    src/legacy_xmp.cpp
    src/text_file.cpp)

target_include_directories(ravo_adapters
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(ravo_adapters
  PUBLIC ravo_foundation ravo_recipe
  PRIVATE Qt6::Core)

ravo_configure_target(ravo_adapters)
