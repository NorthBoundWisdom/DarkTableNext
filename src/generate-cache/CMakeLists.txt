include_directories(${DARKTABLE_BINDIR})
add_executable(darktable-generate-cache main.c)

set_target_properties(darktable-generate-cache PROPERTIES LINKER_LANGUAGE C)
target_link_libraries(darktable-generate-cache lib_darktable whereami)

set_target_properties(darktable-generate-cache
                      PROPERTIES
                      INSTALL_RPATH ${RPATH_ORIGIN}/${REL_BIN_TO_LIBDIR}
                      RUNTIME_OUTPUT_DIRECTORY ${DARKTABLE_BINDIR})

install(TARGETS darktable-generate-cache DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT DTApplication)
