include(CheckCompilerFlagAndEnableIt)
include(CheckCCompilerFlagAndEnableIt)
include(CheckCXXCompilerFlagAndEnableIt)

CHECK_COMPILER_FLAG_AND_ENABLE_IT(-Wall)

CHECK_COMPILER_FLAG_AND_ENABLE_IT(-Wformat)
CHECK_COMPILER_FLAG_AND_ENABLE_IT(-Wformat-security)

CHECK_COMPILER_FLAG_AND_ENABLE_IT(-Wshadow)

CHECK_COMPILER_FLAG_AND_ENABLE_IT(-Wtype-limits)

CHECK_COMPILER_FLAG_AND_ENABLE_IT(-Wvla)

CHECK_C_COMPILER_FLAG_AND_ENABLE_IT(-Wold-style-declaration)

CHECK_COMPILER_FLAG_AND_ENABLE_IT(-Wthread-safety)

CHECK_COMPILER_FLAG_AND_ENABLE_IT(-Wmaybe-uninitialized)

# may be our bug :(
CHECK_COMPILER_FLAG_AND_ENABLE_IT(-Wno-error=varargs)

# Fixed-size path buffers still produce conservative truncation diagnostics
# with current GCC. Keep the diagnostic visible without making it fatal.
CHECK_COMPILER_FLAG_AND_ENABLE_IT(-Wno-error=format-truncation)

# needed to deal with warnings from some macOS SDK headers
CHECK_C_COMPILER_FLAG_AND_ENABLE_IT(-Wno-typedef-redefinition)

# minimal main thread's stack/frame stack size.
# 2 MiB seems to work.
# MUST be a multiple of the system page size !!!
# see  $ getconf PAGESIZE
math(EXPR WANTED_STACK_SIZE 512*4*1024)

# minimal pthread stack/frame stack size.
# 2 MiB seems to work and is default on Linux.
# MUST be a multiple of the system page size !!!
# see  $ getconf PAGESIZE
math(EXPR WANTED_THREADS_STACK_SIZE 512*4*1024)

###### GTK+3 ######
#
#  Do not include individual headers
#
add_definitions(-DGTK_DISABLE_SINGLE_INCLUDES)

#
#  Do not use deprecated symbols
#
add_definitions(-DGDK_DISABLE_DEPRECATED)
add_definitions(-DGTK_DISABLE_DEPRECATED)
###### GTK+3 port ######
