add_library(darktable_ai STATIC
  backend.h
  backend_common.c
  backend_onnx.c
)

# AI uses an ONNX Runtime installed by the active toolchain or package manager.
find_package(ONNXRuntime REQUIRED)

# Find Dependencies (inherited from main build if available, but explicit here for clarity)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB REQUIRED glib-2.0 gobject-2.0 gio-2.0 gmodule-2.0)
pkg_check_modules(JSON_GLIB REQUIRED json-glib-1.0)
# GTK3/RSVG needed for headers only (common/darktable.h -> utility.h -> gtk/gtk.h, librsvg/rsvg.h)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
pkg_check_modules(RSVG2 REQUIRED librsvg-2.0)
# lcms2 needed for headers (common/colorspaces.h included via darktable.h)
pkg_check_modules(LCMS2 REQUIRED lcms2)

# Include current directory for header
target_include_directories(darktable_ai PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
  $<INSTALL_INTERFACE:include>
  ${GLIB_INCLUDE_DIRS}
  ${JSON_GLIB_INCLUDE_DIRS}
  ${GTK3_INCLUDE_DIRS}
  ${RSVG2_INCLUDE_DIRS}
  ${LCMS2_INCLUDE_DIRS}
)

target_link_directories(darktable_ai PUBLIC
  ${GLIB_LIBRARY_DIRS}
  ${JSON_GLIB_LIBRARY_DIRS}
)

target_link_libraries(darktable_ai PUBLIC onnxruntime::onnxruntime)

target_link_libraries(darktable_ai PUBLIC
  ${GLIB_LIBRARIES}
  ${JSON_GLIB_LIBRARIES}
)
