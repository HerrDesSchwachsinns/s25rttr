################################################################################
### $Id: windows.common.cmake 6473 2010-06-03 12:21:19Z FloSoft $
################################################################################

# set compiler flags
FORCE_ADD_FLAGS(CMAKE_C_FLAGS -malign-double -ffast-math -mmmx -msse -mfpmath=sse -fomit-frame-pointer)
FORCE_ADD_FLAGS(CMAKE_CXX_FLAGS -malign-double -ffast-math -mmmx -msse -mfpmath=sse -fomit-frame-pointer)

# bugfix for cygwin
#ADD_DEFINITIONS(-D_WIN32 -D__USE_W32_SOCKETS)

FORCE_ADD_FLAGS(CMAKE_C_FLAGS -D_WIN32 -D__USE_W32_SOCKETS -DNOMINMAX)
FORCE_ADD_FLAGS(CMAKE_CXX_FLAGS -D_WIN32 -D__USE_W32_SOCKETS -DNOMINMAX)
