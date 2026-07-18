if (BUILD_CURVE_TOOLS)
  add_subdirectory(basecurve)
endif()
if (BUILD_NOISE_TOOLS)
  add_subdirectory(noise)
endif()

set(TOOLS common.sh extract_wb_from_images.sh purge_from_cache.sh purge_non_existing_images.sh purge_unused_tags.sh)
install(PROGRAMS ${TOOLS} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/darktable/tools)
