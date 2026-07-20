cmake_minimum_required(VERSION 3.26)

# CMake 3.27+ recognizes upper-case <PACKAGENAME>_ROOT hints.  Keep the
# behavior consistent when configuring with newer CMake releases.
if(POLICY CMP0144)
  cmake_policy(SET CMP0144 NEW)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

### Add defaults for cmake
# These defaults need to be included before the project() call.
include(DefineCMakeDefaults)

If(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source builds are not permitted. Make a separate folder for building:\nmkdir build; cd build; cmake ..\nBefore that, remove the files already created:\nrm -rf CMakeCache.txt CMakeFiles")
endif(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)

if(DEFINED PROJECT_VERSION)
  project(darktable VERSION ${PROJECT_VERSION} LANGUAGES CXX C)
else()
  project(darktable VERSION 0.9.0 LANGUAGES CXX C)
endif()

# Tests are opt-in: application presets must not pull test-only dependencies
# or assets into a normal configure/build.
option(BUILD_TESTING "Build DarkTableNext tests" OFF)

# Loadable modules validate their own DT_MODULE_VERSION at runtime. Re-linking every
# module for a private lib_darktable implementation change only rewrites the module
# files and turns the next launch into a cold dynamic-loader pass. Keep that legacy
# behaviour available for conservative ABI investigations, but leave normal builds
# to rebuild modules when their own sources or ABI headers change.
option(DT_MODULE_RELINK_ON_CORE_CHANGE
       "Relink loadable modules whenever a linked shared library changes" OFF)

function(dt_module_configure_link_dependencies target)
  if(NOT DT_MODULE_RELINK_ON_CORE_CHANGE)
    set_property(TARGET ${target} PROPERTY LINK_DEPENDS_NO_SHARED TRUE)
  endif()
endfunction()

if(NOT APPLE)
  message(FATAL_ERROR "DarkTableNext supports macOS only.")
endif()

if(NOT CMAKE_C_COMPILER_ID MATCHES "^(AppleClang|Clang|GNU)$")
  message(FATAL_ERROR "Unsupported C compiler: ${CMAKE_C_COMPILER_ID}. Use Clang or GCC.")
endif()
if(NOT CMAKE_CXX_COMPILER_ID MATCHES "^(AppleClang|Clang|GNU)$")
  message(FATAL_ERROR "Unsupported C++ compiler: ${CMAKE_CXX_COMPILER_ID}. Use Clang or GCC.")
endif()

include(DefineOptions.cmake)

# Include GNUInstallDirs, which sets sensible defaults for install directories.
# See https://cmake.org/cmake/help/v3.0/module/GNUInstallDirs.html for further information.
# These values can be easily overridden if required.
# Some defaults are set for OpenBSD as well (info and man pages).

include(GNUInstallDirs)
include(FeatureSummary)

# resolve symbolic links into real path?
get_filename_component(CMAKE_INSTALL_BINDIR_ABS "${CMAKE_INSTALL_FULL_BINDIR}" REALPATH "${CMAKE_INSTALL_PREFIX}")
# get relative paths specific to the system
file(RELATIVE_PATH REL_BIN_TO_LIBDIR    ${CMAKE_INSTALL_FULL_BINDIR} "${CMAKE_INSTALL_FULL_LIBDIR}/darktable")
file(RELATIVE_PATH REL_BIN_TO_SHAREDIR  ${CMAKE_INSTALL_FULL_BINDIR} "${CMAKE_INSTALL_FULL_DATADIR}")
set(REL_BIN_TO_DATADIR  "${REL_BIN_TO_SHAREDIR}/darktable")
set(DARKTABLE_BINDIR    "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(DARKTABLE_LIBDIR    "${DARKTABLE_BINDIR}/${REL_BIN_TO_LIBDIR}")
set(DARKTABLE_SHAREDIR  "${DARKTABLE_BINDIR}/${REL_BIN_TO_SHAREDIR}")
set(DARKTABLE_DATADIR   "${DARKTABLE_BINDIR}/${REL_BIN_TO_DATADIR}") # DATAROOTDIR == DATADIR # darktablerc, themes, icons, ... Cannot use ${CMAKE_BINDIR}/data because not everything resides there. Like icons (pixmaps)

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

# macOS uses loader-relative install names for bundled libraries and modules.
set(CMAKE_MACOSX_RPATH ON)
set(RPATH_ORIGIN @loader_path)

check_c_compiler_flag("-fopenmp-version=51" DT_C_HAS_OPENMP_51_FLAG)
if(DT_C_HAS_OPENMP_51_FLAG)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp-version=51")
endif()
check_cxx_compiler_flag("-fopenmp-version=51" DT_CXX_HAS_OPENMP_51_FLAG)
if(DT_CXX_HAS_OPENMP_51_FLAG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp-version=51")
endif()

find_package(OpenMP 4.5 REQUIRED)

# Check for base threading library
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)

# Fail if returned library is not pthread
if (NOT CMAKE_USE_PTHREADS_INIT)
    message(FATAL_ERROR "POSIX threads: not found")
endif()

#
# Set platform defaults...
#
message("-- macOS build")
find_program(HOMEBREW_EXISTS "brew")
if(NOT HOMEBREW_EXISTS)
  message(FATAL_ERROR "DarkTableNext currently requires Homebrew on macOS.")
endif()
execute_process(COMMAND brew --prefix OUTPUT_VARIABLE HOMEBREW_PREFIX)
string(STRIP "${HOMEBREW_PREFIX}" HOMEBREW_PREFIX)
message("-- Homebrew detected, setting CMAKE_PREFIX_PATH to ${HOMEBREW_PREFIX}")
list(APPEND CMAKE_PREFIX_PATH ${HOMEBREW_PREFIX} ${HOMEBREW_PREFIX}/opt/llvm ${HOMEBREW_PREFIX}/opt/icu4c)
set(CMAKE_FIND_FRAMEWORK "LAST")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_DARWIN_C_SOURCE")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_DARWIN_C_SOURCE")

#
# Set package version
#
file(MAKE_DIRECTORY "${DARKTABLE_BINDIR}/") # the bin/ subdirectory won't exist yet

# Adds a custom command to generate the version source from PROJECT_VERSION.
function(generate_version_gen_c)
  set(_VERSION "${ARGV0}")
  set(_TYPE "${ARGV1}")

  add_custom_target(
    create_version_gen ALL
    COMMAND sh ${CMAKE_SOURCE_DIR}/tools/create_version_c.sh ${DARKTABLE_BINDIR}/version_gen.c ${_VERSION}
    DEPENDS ${CMAKE_SOURCE_DIR}/tools/create_version_c.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Updating version string (${_TYPE})"
    VERBATIM # else might break when export-subst was needed but did not happen
  )
endfunction(generate_version_gen_c)

generate_version_gen_c(${PROJECT_VERSION} "project version")

# needed to make sure that version string is actually updated.
add_custom_command(
  OUTPUT ${DARKTABLE_BINDIR}/version_gen.c
  COMMAND ${CMAKE_COMMAND} -E echo
  DEPENDS create_version_gen
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# WARNING: no target should reference version_gen.c directly. instead, they should add_dependencies(yourtarget generate_version)
add_custom_target(
  generate_version ALL
  DEPENDS ${DARKTABLE_BINDIR}/version_gen.c
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Add a sensible build type default and warning because empty means no optimization and no debug info.
if(NOT CMAKE_BUILD_TYPE)
	message("WARNING: CMAKE_BUILD_TYPE is not defined!\n         Defaulting to CMAKE_BUILD_TYPE=RelWithDebInfo. Use ccmake to set a proper value.")
	SET(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

include(compiler-warnings)

# we need some external programs for building darktable
message(STATUS "Looking for external programs")

# we need perl for introspection
find_program(perl_BIN perl REQUIRED)
message(STATUS "Found perl")

if(TESTBUILD_OPENCL_PROGRAMS)
  set(TESTBUILD_OPENCL_PROGRAMS OFF)

  find_package(LLVM CONFIG)

  if(LLVM_FOUND)
    if(${LLVM_VERSION} VERSION_LESS 7)
      set(LLVM_FOUND FALSE)
    endif()
  endif()

  if (LLVM_FOUND)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

    find_program(CLANG_OPENCL_COMPILER
      NAMES clang-${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR} clang-${LLVM_PACKAGE_VERSION} clang${LLVM_VERSION_MAJOR}${LLVM_VERSION_MINOR} clang-${LLVM_VERSION_MAJOR} clang${LLVM_VERSION_MAJOR}
    )

    if (NOT ${CLANG_OPENCL_COMPILER} STREQUAL "CLANG_OPENCL_COMPILER-NOTFOUND")
      message(STATUS "Found clang compiler - ${CLANG_OPENCL_COMPILER}")

      find_path(CLANG_OPENCL_INCLUDE_DIR opencl-c.h
        HINTS ${LLVM_INSTALL_PREFIX}/lib/clang ${LLVM_INSTALL_PREFIX}/lib64/clang
        PATH_SUFFIXES include ${LLVM_PACKAGE_VERSION}/include ${LLVM_VERSION_MAJOR}/include
        NO_DEFAULT_PATH
      )

      if (NOT ${CLANG_OPENCL_INCLUDE_DIR} STREQUAL "CLANG_OPENCL_INCLUDE_DIR-NOTFOUND")
        message(STATUS "Found clang opencl-c.h header in ${CLANG_OPENCL_INCLUDE_DIR}")
        set(TESTBUILD_OPENCL_PROGRAMS ON)
      else()
        message(WARNING "Could not find clang opencl-c.h header include dir")
        message(WARNING "Test-compilation of OpenCL programs can not be done.")
      endif()
    else()
      message(WARNING "Could not find appropriate clang compiler")
      message(WARNING "Test-compilation of OpenCL programs can not be done.")
    endif()
  else()
    message(WARNING "Could not find LLVM 7 or above")
    message(WARNING "Test-compilation of OpenCL programs can not be done.")
  endif()
endif()

if(TESTBUILD_OPENCL_PROGRAMS)
  message(STATUS "Will be able to test-compile OpenCL programs. Nice.")
else()
  message(STATUS "Test-compilation of OpenCL programs is unavailable.")
endif()

# we need jsonschema to check noiseprofiles.json
find_program(jsonschema_BIN jsonschema)
if(${jsonschema_BIN} STREQUAL "jsonschema_BIN-NOTFOUND")
  message(STATUS "Missing jsonschema, problems in noiseprofiles.json might go unnoticed")
  set(VALIDATE_JSON 0)
else(${jsonschema_BIN} STREQUAL "jsonschema_BIN-NOTFOUND")
  message(STATUS "Found jsonschema")
  set(VALIDATE_JSON 1)
endif(${jsonschema_BIN} STREQUAL "jsonschema_BIN-NOTFOUND")

# xsltproc generates the checked build artifacts from the configuration schema.
find_program(Xsltproc_BIN xsltproc REQUIRED)

# xmllint validates the generated configuration schema during every build.
find_program(Xmllint_BIN xmllint REQUIRED)
message(STATUS "Found xmllint")

# done with looking for programs
message(STATUS "All external programs found")

# OpenCL is a required runtime capability.
SET(HAVE_OPENCL 1)

if(BUILD_TESTING)
  find_package(cmocka 1.1.0 CONFIG REQUIRED)
  include(AddCMockaTest)
  include_directories(${CMOCKA_INCLUDE_DIR} include)
endif(BUILD_TESTING)

# macOS uses the POSIX 2008 interfaces throughout the core.
add_definitions("-D_XOPEN_SOURCE=700")

# Set default component name - that way external modules like RawSpeed will install their
# materials under the default component and not under 'Unspecified'
set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME DTApplication)

# lets continue into build directories
include(data/supported_extensions.cmake) # this file needs to be included first as it gets amended in bin/
add_subdirectory(src "${DARKTABLE_BINDIR}") # directory needs to be there before data directory so that the correct CSS file gets installed
add_subdirectory(data "${DARKTABLE_DATADIR}")

# FreeCM owns the final macOS deployment step.  Prepare a clean staging prefix
# through the normal install graph first, then hand its native application
# bundle to the generic (non-Qt) macOS packager.
set(DARKTABLE_PACKAGE_ROOT "${CMAKE_BINARY_DIR}/package")
set(DARKTABLE_PACKAGE_STAGE "${DARKTABLE_PACKAGE_ROOT}/stage")
set(DARKTABLE_PACKAGE_DIST_DIR "${DARKTABLE_PACKAGE_ROOT}/dist")
set(DT_MACOS_CODESIGN_IDENTITY "-" CACHE STRING
    "codesign identity for the macOS package ('-' keeps developer builds ad hoc signed)")
find_program(DT_PKG_CONFIG_EXECUTABLE pkg-config REQUIRED)
execute_process(
    COMMAND "${DT_PKG_CONFIG_EXECUTABLE}" --variable=gdk_pixbuf_moduledir gdk-pixbuf-2.0
    OUTPUT_VARIABLE DARKTABLE_GDK_PIXBUF_MODULE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY)
if(NOT IS_DIRECTORY "${DARKTABLE_GDK_PIXBUF_MODULE_DIR}")
  message(FATAL_ERROR
      "gdk-pixbuf loader directory was not found: ${DARKTABLE_GDK_PIXBUF_MODULE_DIR}")
endif()
get_filename_component(DARKTABLE_GDK_PIXBUF_MODULE_ROOT
    "${DARKTABLE_GDK_PIXBUF_MODULE_DIR}" DIRECTORY)
set(DARKTABLE_GDK_PIXBUF_MODULE_CACHE
    "${DARKTABLE_GDK_PIXBUF_MODULE_ROOT}/loaders.cache")
if(NOT EXISTS "${DARKTABLE_GDK_PIXBUF_MODULE_CACHE}")
  message(FATAL_ERROR
      "gdk-pixbuf loader cache was not found: ${DARKTABLE_GDK_PIXBUF_MODULE_CACHE}")
endif()
if(CMAKE_OSX_ARCHITECTURES)
  list(JOIN CMAKE_OSX_ARCHITECTURES "-" DARKTABLE_PACKAGE_ARCH)
else()
  set(DARKTABLE_PACKAGE_ARCH "${CMAKE_HOST_SYSTEM_PROCESSOR}")
endif()
set(DARKTABLE_PACKAGE_DMG
    "${CMAKE_BINARY_DIR}/darktable-${PROJECT_VERSION}-macos-${DARKTABLE_PACKAGE_ARCH}.dmg")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/packaging/macos/package.json.in"
    "${DARKTABLE_PACKAGE_ROOT}/package.json"
    @ONLY)

add_custom_target(prepare-macos-package
    COMMAND "${CMAKE_COMMAND}" -E remove_directory "${DARKTABLE_PACKAGE_STAGE}"
    COMMAND "${CMAKE_COMMAND}" --install "${CMAKE_BINARY_DIR}" --prefix "${DARKTABLE_PACKAGE_STAGE}"
    DEPENDS darktable
    COMMENT "Preparing the darktable macOS package staging bundle"
    VERBATIM
    USES_TERMINAL)

include("${CMAKE_SOURCE_DIR}/FreeCM/repomgrcpp/cmake/CppKitPackage.cmake")
cppkit_add_package_deploy(prepare-macos-package
    CONFIG "${DARKTABLE_PACKAGE_ROOT}/package.json"
    PLATFORM mac
    TARGET_NAME package-macos)

# The native app target is installed by src/CMakeLists.txt.  All project
# libraries, modules and data have been installed by this point, so the final
# script can replace the build-tree convenience links with real bundle
# contents.  FreeCM then adds the third-party GTK/Homebrew dependency closure.
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/macos_bundle_install.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/macos_bundle_install.cmake"
    @ONLY)
install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/macos_bundle_install.cmake"
        COMPONENT DTApplication)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    @ONLY)

if(NOT TARGET uninstall)
  add_custom_target(uninstall
      COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

feature_summary(WHAT ALL)
