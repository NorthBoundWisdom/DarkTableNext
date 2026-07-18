set(DARKTABLE_SOURCE_ROOTS_DIR "${CMAKE_SOURCE_DIR}/build/dependency_source_roots" CACHE PATH
    "FreeCM materialized dependency source roots")

function(dt_resolve_freecm_source_root output_variable cache_variable environment_variable repository_name required_path)
  set(_source_root "")

  if(DEFINED ${cache_variable} AND NOT "${${cache_variable}}" STREQUAL "")
    set(_source_root "${${cache_variable}}")
  elseif(DEFINED ENV{${environment_variable}} AND NOT "$ENV{${environment_variable}}" STREQUAL "")
    set(_source_root "$ENV{${environment_variable}}")
  else()
    set(_source_root "${DARKTABLE_SOURCE_ROOTS_DIR}/${repository_name}")
  endif()

  if(NOT EXISTS "${_source_root}/${required_path}")
    message(FATAL_ERROR
      "FreeCM source root for ${repository_name} is unavailable at '${_source_root}'.\n"
      "Run: python3 configs/source_root_workflow.py --init && "
      "python3 configs/source_root_workflow.py --update\n"
      "Or configure with -D${cache_variable}=<source-root>.")
  endif()

  set(${output_variable} "${_source_root}" PARENT_SCOPE)
endfunction()

dt_resolve_freecm_source_root(LIBXCF_PATH LIBXCF_SOURCE_ROOT LIBXCF_SOURCE_ROOT libxcf CMakeLists.txt)
dt_resolve_freecm_source_root(WHEREAMI_PATH WHEREAMI_SOURCE_ROOT WHEREAMI_SOURCE_ROOT whereami src/whereami.c)
dt_resolve_freecm_source_root(OPENCL_PATH OPENCL_HEADERS_SOURCE_ROOT OPENCL_HEADERS_SOURCE_ROOT OpenCL CL/opencl.h)
dt_resolve_freecm_source_root(RAWSPEED_PATH RAWSPEED_SOURCE_ROOT RAWSPEED_SOURCE_ROOT rawspeed CMakeLists.txt)
set(OPENCL_PATH "${OPENCL_PATH}" PARENT_SCOPE)

if(USE_XCF)
  add_subdirectory("${LIBXCF_PATH}" "${CMAKE_CURRENT_BINARY_DIR}/libxcf")
endif()

add_library(whereami STATIC "${WHEREAMI_PATH}/src/whereami.c")
target_include_directories(whereami PUBLIC "${WHEREAMI_PATH}/src/")

################################################################################
# RawSpeed block

string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPERCASE)

set(RAWSPEED_${CMAKE_BUILD_TYPE_UPPERCASE}_BUILD 1)
if(RAWSPEED_RELWITHDEBINFO_BUILD)
  set(CMAKE_BUILD_TYPE "Release")
endif()

SET(COMPILER_SUPPORTS_DEBUG_INFO_COMPRESSION OFF)
SET(COMPILER_SUPPORTS_DEBUG_INFO_COMPRESSION_LINK OFF)
SET(COMPILER_SUPPORTS_SPLIT_DEBUG_INFO OFF)
SET(LINKER_SUPPORTS_GDB_INDEX OFF)

set(BUILD_TESTING_SAVE "${BUILD_TESTING}")
set(BUILD_BENCHMARKING_SAVE "${BUILD_BENCHMARKING}")

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_BENCHMARKING OFF CACHE BOOL "" FORCE)
set(BUILD_FUZZERS OFF CACHE BOOL "" FORCE)
set(WITH_OPENMP ${USE_OPENMP} CACHE BOOL "" FORCE)

set(RAWSPEED_BINARY_PREFIX "darktable")

if(NOT(EXISTS "${RAWSPEED_PATH}" AND IS_DIRECTORY "${RAWSPEED_PATH}" AND EXISTS "${RAWSPEED_PATH}/CMakeLists.txt"))
  message(FATAL_ERROR
    "RawSpeed FreeCM source root not found. Run: python3 configs/source_root_workflow.py --init && "
    "python3 configs/source_root_workflow.py --update")
endif()

set(CMAKE_INSTALL_DATAROOTDIR_SAVE "${CMAKE_INSTALL_DATAROOTDIR}")

set(CMAKE_C_FLAGS_SAVE "${CMAKE_C_FLAGS}")
set(CMAKE_CXX_FLAGS_SAVE "${CMAKE_CXX_FLAGS}")

set(CMAKE_INSTALL_DATAROOTDIR "${CMAKE_INSTALL_DATAROOTDIR}/darktable")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -w")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")

set(CMAKE_CXX_CLANG_TIDY_SAVE "${CMAKE_CXX_CLANG_TIDY}")
set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE_SAVE "${CMAKE_CXX_INCLUDE_WHAT_YOU_USE}")

unset(CMAKE_CXX_CLANG_TIDY)
unset(CMAKE_CXX_INCLUDE_WHAT_YOU_USE)

add_subdirectory("${RAWSPEED_PATH}" ${DARKTABLE_LIBDIR}/rawspeed)
# Copying rawspeed resources to where dt expects them
add_custom_target("deploy" ALL)
SET(RAWSPEED_DATADIR "${RAWSPEED_PATH}/data")
add_custom_command(TARGET "deploy" PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${DARKTABLE_DATADIR}/rawspeed/
  COMMAND ${CMAKE_COMMAND} -E copy ${RAWSPEED_DATADIR}/cameras.xml ${RAWSPEED_DATADIR}/showcameras.xsl ${DARKTABLE_DATADIR}/rawspeed/
)

set(CMAKE_INSTALL_DATAROOTDIR "${CMAKE_INSTALL_DATAROOTDIR_SAVE}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_SAVE}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_SAVE}")

set(CMAKE_CXX_CLANG_TIDY "${CMAKE_CXX_CLANG_TIDY_SAVE}")
set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "${CMAKE_CXX_INCLUDE_WHAT_YOU_USE_SAVE}")

set(BUILD_TESTING "${BUILD_TESTING_SAVE}" CACHE BOOL "" FORCE)
set(BUILD_BENCHMARKING "${BUILD_BENCHMARKING_SAVE}" CACHE BOOL "" FORCE)

###############################################################################
# lua-scripts section

if(USE_LUA)
#
# lua system scripts
#
FILE(COPY lua-scripts DESTINATION "${DARKTABLE_DATADIR}")
install(DIRECTORY "lua-scripts" DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/darktable COMPONENT DTApplication
  PATTERN "lua-scripts/.git/*" EXCLUDE
  PATTERN "lua-scripts/.git" EXCLUDE
  PATTERN "lua-scripts/.gitignore" EXCLUDE
)
endif(USE_LUA)
