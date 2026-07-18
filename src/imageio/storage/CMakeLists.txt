include_directories("${DARKTABLE_BINDIR}")

include(manage-symbol-visibility)

add_definitions(-include common/module_api.h)
add_definitions(-include imageio/storage/imageio_storage_api.h)

set(MODULES disk email gallery latex piwigo)

foreach(module ${MODULES})
	add_library(${module} MODULE "${module}.c")
endforeach(module)

foreach(module ${MODULES})
    target_link_libraries(${module} lib_darktable)
    set_target_properties(${module}
                          PROPERTIES
                          INSTALL_RPATH ${RPATH_ORIGIN}/../../..)
    install(TARGETS ${module} DESTINATION ${CMAKE_INSTALL_LIBDIR}/darktable/plugins/imageio/storage COMPONENT DTApplication)
endforeach(module)
