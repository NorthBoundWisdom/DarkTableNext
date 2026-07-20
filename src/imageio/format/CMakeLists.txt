include_directories("${DARKTABLE_BINDIR}")

include_directories(SYSTEM "${PNG_PNG_INCLUDE_DIR}")

include(manage-symbol-visibility)

add_definitions(-include common/module_api.h)
add_definitions(-include imageio/format/imageio_format_api.h)

set(MODULES copy jpeg png tiff)

add_library(copy MODULE "copy.c")
add_library(jpeg MODULE "jpeg.c")
add_library(png MODULE "png.c")
add_library(tiff MODULE "tiff.c")

foreach(module ${MODULES})
    target_link_libraries(${module} PUBLIC lib_darktable)
    dt_module_configure_link_dependencies(${module})
    set_target_properties(${module}
                          PROPERTIES
                          INSTALL_RPATH ${RPATH_ORIGIN}/../../..)
    install(TARGETS ${module} DESTINATION ${CMAKE_INSTALL_LIBDIR}/darktable/plugins/imageio/format COMPONENT DTApplication)
endforeach(module)
