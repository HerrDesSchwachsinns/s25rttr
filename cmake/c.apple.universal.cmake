################################################################################
### $Id: c.apple.universal.cmake 6122 2010-03-06 12:29:10Z FloSoft $
################################################################################

# this one is important
SET(CMAKE_SYSTEM_NAME Darwin)
#SET(CMAKE_SYSTEM_PROCESSOR universal)

SET(CMAKE_OSX_ARCHITECTURES "${COMPILEARCHS}" CACHE STRING "OSX-Architectures" FORCE)

INCLUDE(cmake/c.apple.common.cmake)
