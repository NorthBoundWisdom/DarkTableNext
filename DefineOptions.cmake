option(USE_OPENCL "Use OpenCL support." ON) # ALWAYS ENABLED
option(USE_DARKTABLE_PROFILING OFF) # TO REMOVE FUNCTION
option(USE_XMLLINT "Run xmllint to test if darktableconfig.xml is valid" ON) # ALWAYS ENABLED
option(USE_ISOBMFF "Enable ISOBMFF support" ON) # ALWAYS ENABLED
option(USE_LIBRAW "Enable LibRaw support" ON) # ALWAYS ENABLED
option(USE_AI "Enable AI support" OFF) # TO REMOVE FUNCTION
option(USE_ICU "Use ICU - International Components for Unicode." ON) # ALWAYS ENABLED
option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." OFF) # TO REMOVE FUNCTION

if (USE_OPENCL)
    option(TESTBUILD_OPENCL_PROGRAMS "Test-compile OpenCL programs" ON) # ALWAYS ENABLED
else ()
    set(TESTBUILD_OPENCL_PROGRAMS OFF)
endif ()
