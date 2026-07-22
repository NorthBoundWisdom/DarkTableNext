add_library(ravo_engine STATIC)

target_sources(ravo_engine
  PRIVATE
    src/engine.cpp)

target_include_directories(ravo_engine
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_link_libraries(ravo_engine PUBLIC ravo_recipe)

ravo_configure_target(ravo_engine)
