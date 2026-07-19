set(DARKTABLENEXT_QT_ROOT "" CACHE PATH
    "Qt 6 installation prefix used by the experimental QML shell")

if(DARKTABLENEXT_QT_ROOT)
  list(PREPEND CMAKE_PREFIX_PATH "${DARKTABLENEXT_QT_ROOT}")
endif()

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Svg)

qt_policy(SET QTP0004 NEW)

set(_darktablenext_geocontrols_root "")
if(DEFINED GEOCONTROLS_SOURCE_ROOT AND NOT "${GEOCONTROLS_SOURCE_ROOT}" STREQUAL "")
  set(_darktablenext_geocontrols_root "${GEOCONTROLS_SOURCE_ROOT}")
elseif(DEFINED ENV{GEOCONTROLS_SOURCE_ROOT} AND NOT "$ENV{GEOCONTROLS_SOURCE_ROOT}" STREQUAL "")
  set(_darktablenext_geocontrols_root "$ENV{GEOCONTROLS_SOURCE_ROOT}")
else()
  set(_darktablenext_geocontrols_root
      "${CMAKE_SOURCE_DIR}/build/dependency_source_roots/GeoControls")
endif()

if(NOT EXISTS "${_darktablenext_geocontrols_root}/CMakeLists.txt")
  message(FATAL_ERROR
    "GeoControls source root is unavailable at '${_darktablenext_geocontrols_root}'.\n"
    "Run: python3 configs/source_root_workflow.py --init && "
    "python3 configs/source_root_workflow.py --update\n"
    "Or configure with -DGEOCONTROLS_SOURCE_ROOT=<source-root>.")
endif()

if(NOT TARGET GeoControlsControls)
  set(GEOCONTROLS_BUILD_DEMO OFF CACHE BOOL
      "Build the GeoControls demo while embedded in DarkTableNext" FORCE)
  set(GEOCONTROLS_INSTALL_SDK OFF CACHE BOOL
      "Install the embedded GeoControls SDK" FORCE)
  add_subdirectory("${_darktablenext_geocontrols_root}"
                   "${CMAKE_CURRENT_BINARY_DIR}/geocontrols")
endif()

qt_add_executable(darktablenext_shell
  main.cpp
)

set_target_properties(darktablenext_shell PROPERTIES
  AUTOMOC ON
  AUTORCC ON
)

qt_add_qml_module(darktablenext_shell
  URI DarkTableNext.App
  VERSION 0.1
  RESOURCE_PREFIX /
  IMPORT_PATH
    "${CMAKE_CURRENT_BINARY_DIR}/geocontrols/Controls"
    "${CMAKE_CURRENT_BINARY_DIR}/geocontrols/AppShell"
  IMPORTS
    GeoControls
    GeoControls.AppShell
  QML_FILES
    qml/Main.qml
    qml/WelcomeScreen.qml
    qml/MainShell.qml
)

target_link_libraries(darktablenext_shell
  PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    GeoControlsControls
    GeoControlsAppShell
)

set_target_properties(darktablenext_shell PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  MACOSX_BUNDLE TRUE
  OUTPUT_NAME DarkTableNext
)
