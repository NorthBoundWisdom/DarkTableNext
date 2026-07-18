# macOS-only build
find_package(OpenMP REQUIRED)

    set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/modules")
    find_library(M_LIBRARY m)
    find_package(Exiv2 REQUIRED)
    include_directories(${Exiv2_INCLUDE_DIR})

    set(CMAKE_CXX_STANDARD 17)

    add_executable(darktable-curve-tool darktable-curve-tool.c exif-wrapper.cpp)
    set_target_properties(darktable-curve-tool PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
    if(OpenMP_CXX_INCLUDE_DIRS)
        include_directories(${OpenMP_CXX_INCLUDE_DIRS})
    endif()
    target_link_libraries(darktable-curve-tool ${Exiv2_LIBRARY} ${OpenMP_CXX_LIBRARIES} m)

    configure_file(README.cmake.md README.md)

    install(TARGETS
                darktable-curve-tool
            RUNTIME DESTINATION
                ${CMAKE_INSTALL_LIBEXECDIR}/darktable/tools)

    install(PROGRAMS
                darktable-curve-tool-helper
            DESTINATION
                ${CMAKE_INSTALL_LIBEXECDIR}/darktable/tools)


    install(FILES
                plot.basecurve plot.tonecurve
            DESTINATION
                ${CMAKE_INSTALL_DATAROOTDIR}/darktable/tools/basecurve/)
    install(FILES
                ${CMAKE_CURRENT_BINARY_DIR}/README.md
            RENAME
                README.tools.basecurve.md
            DESTINATION
                ${CMAKE_INSTALL_DOCDIR}/)
