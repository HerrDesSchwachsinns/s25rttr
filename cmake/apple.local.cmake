################################################################################
### $Id: apple.local.cmake 6614 2010-07-23 11:28:57Z FloSoft $
################################################################################

# this one is important
EXECUTE_PROCESS(COMMAND "uname" OUTPUT_VARIABLE CMAKE_SYSTEM_NAME OUTPUT_STRIP_TRAILING_WHITESPACE)
EXECUTE_PROCESS(COMMAND "uname" "-m" OUTPUT_VARIABLE CMAKE_SYSTEM_PROCESSOR OUTPUT_STRIP_TRAILING_WHITESPACE)

SET_CCACHE(CMAKE_C_COMPILER   gcc)
SET_CCACHE(CMAKE_CXX_COMPILER g++)

INCLUDE(cmake/apple.common.cmake)

