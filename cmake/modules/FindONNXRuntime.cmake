# Find a locally installed ONNX Runtime.  DarkTableNext deliberately never
# downloads SDKs during configuration: install the package with the platform
# package manager, or set ONNXRUNTIME_ROOT to its installation prefix.

include(FindPackageHandleStandardArgs)

set(_ONNXRUNTIME_HINTS)
if(DEFINED ONNXRUNTIME_ROOT)
  list(APPEND _ONNXRUNTIME_HINTS "${ONNXRUNTIME_ROOT}")
endif()
if(DEFINED ONNXRuntime_ROOT)
  list(APPEND _ONNXRUNTIME_HINTS "${ONNXRuntime_ROOT}")
endif()
if(DEFINED ENV{ONNXRUNTIME_ROOT})
  list(APPEND _ONNXRUNTIME_HINTS "$ENV{ONNXRUNTIME_ROOT}")
endif()
if(ONNXRUNTIME_ROOT)
  list(APPEND _ONNXRUNTIME_HINTS "${ONNXRUNTIME_ROOT}")
endif()

find_path(ONNXRuntime_INCLUDE_DIR
  NAMES onnxruntime_c_api.h
  HINTS ${_ONNXRUNTIME_HINTS}
  PATH_SUFFIXES include include/onnxruntime
)
find_library(ONNXRuntime_LIBRARY
  NAMES onnxruntime
  HINTS ${_ONNXRUNTIME_HINTS}
  PATH_SUFFIXES lib lib64
)

find_package_handle_standard_args(ONNXRuntime
  REQUIRED_VARS ONNXRuntime_INCLUDE_DIR ONNXRuntime_LIBRARY
)

if(ONNXRuntime_FOUND)
  set(ONNXRuntime_INCLUDE_DIRS "${ONNXRuntime_INCLUDE_DIR}")
  set(ONNXRuntime_LIBRARIES "${ONNXRuntime_LIBRARY}")
  get_filename_component(ONNXRuntime_LIB_DIR "${ONNXRuntime_LIBRARY}" DIRECTORY)

  if(NOT TARGET onnxruntime::onnxruntime)
    add_library(onnxruntime::onnxruntime UNKNOWN IMPORTED)
    set_target_properties(onnxruntime::onnxruntime PROPERTIES
      IMPORTED_LOCATION "${ONNXRuntime_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${ONNXRuntime_INCLUDE_DIR}"
    )
  endif()
endif()

mark_as_advanced(ONNXRuntime_INCLUDE_DIR ONNXRuntime_LIBRARY)
