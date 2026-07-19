option(USE_COLORD "Enable colord support" ON) # TO REMOVE FUNCTION
option(USE_OPENCL "Use OpenCL support." ON) # ALWAYS ENABLED
option(USE_GRAPHICSMAGICK "Use GraphicsMagick library for image import." ON) # TO REMOVE FUNCTION
option(USE_IMAGEMAGICK "Use ImageMagick library for image import." OFF) # TO REMOVE FUNCTION
option(USE_DARKTABLE_PROFILING OFF) # TO REMOVE FUNCTION
option(USE_XMLLINT "Run xmllint to test if darktableconfig.xml is valid" ON) # ALWAYS ENABLED
option(USE_OPENJPEG "Enable JPEG 2000 support" ON) # TO REMOVE FUNCTION
option(USE_JXL "Enable JPEG XL support" ON) # TO REMOVE FUNCTION
option(USE_WEBP "Enable WebP support" ON) # TO REMOVE FUNCTION
option(USE_AVIF "Enable AVIF support" ON) # TO REMOVE FUNCTION
option(USE_HEIF "Enable HEIF/HEIC support" ON) # ALWAYS ENABLED
option(USE_XCF "Enable XCF support" ON) # TO REMOVE FUNCTION
option(USE_ISOBMFF "Enable ISOBMFF support" ON) # ALWAYS ENABLED
option(USE_LIBRAW "Enable LibRaw support" ON) # ALWAYS ENABLED
option(USE_AI "Enable AI support" OFF) # TO REMOVE FUNCTION
option(BUILD_CMSTEST "Build a test program to check your system's color management setup" ON) # TO REMOVE FUNCTION
option(USE_OPENEXR "Enable OpenEXR support" ON) # TO REMOVE FUNCTION
option(USE_GMIC "Use G'MIC image processing framework." ON) # TO REMOVE FUNCTION
option(USE_ICU "Use ICU - International Components for Unicode." ON) # ALWAYS ENABLED
option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." OFF) # TO REMOVE FUNCTION

if (USE_OPENCL)
    option(TESTBUILD_OPENCL_PROGRAMS "Test-compile OpenCL programs" ON) # ALWAYS ENABLED
else ()
    set(TESTBUILD_OPENCL_PROGRAMS OFF)
endif ()
