################################################################################
### $Id: apple.common.cmake 6185 2010-03-24 20:14:07Z FloSoft $
################################################################################

IF ( "${CMAKE_OSX_DEPLOYMENT_TARGET}" STREQUAL "" )
	SET(CMAKE_OSX_DEPLOYMENT_TARGET "10.4" CACHE STRING "OSX-Deployment-Target" FORCE)
ENDIF ( "${CMAKE_OSX_DEPLOYMENT_TARGET}" STREQUAL "" )

IF ( NOT "${COMPILEARCH}" STREQUAL "" )
	IF ( "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "" )
        	SET(CMAKE_OSX_ARCHITECTURES "${COMPILEARCHS}" CACHE STRING "OSX-Architectures" FORCE)
	ENDIF ( "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "" )

	IF ( "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "" )
        	MESSAGE(FATAL_ERROR "woooops: this should not happen! your system has no known architecture!")
	ENDIF ( "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "" )
	
	SET(APPLE_CFLAGS "")

	IF ( "${CMAKE_OSX_ARCHITECTURES}" MATCHES "i386" )
		MESSAGE(STATUS "Building architecture i386")
		SET(APPLE_CFLAGS ${APPLE_CFLAGS} -arch i386)
		SET(APPLE_LDFLAGS ${APPLE_LDFLAGS} -arch i386)
	ENDIF ( "${CMAKE_OSX_ARCHITECTURES}" MATCHES "i386" )

	IF ( "${CMAKE_OSX_ARCHITECTURES}" MATCHES "x86_64" )
		MESSAGE(STATUS "Building architecture x86_64")
		SET(APPLE_CFLAGS ${APPLE_CFLAGS} -arch x86_64)
		SET(APPLE_LDFLAGS ${APPLE_LDFLAGS} -arch x86_64)
	ENDIF ( "${CMAKE_OSX_ARCHITECTURES}" MATCHES "x86_64" )
	
	IF ( "${CMAKE_OSX_ARCHITECTURES}" MATCHES "ppc" )
		MESSAGE(STATUS "Building architecture ppc")
		SET(APPLE_CFLAGS ${APPLE_CFLAGS} -arch ppc)
		SET(APPLE_LDFLAGS ${APPLE_LDFLAGS} -arch ppc)
	ENDIF ( "${CMAKE_OSX_ARCHITECTURES}" MATCHES "ppc" )

	# i386 only?
	IF ( NOT "${CMAKE_OSX_ARCHITECTURES}" MATCHES "x86_64" AND NOT "${CMAKE_OSX_ARCHITECTURES}" MATCHES "ppc" )
		ADD_FLAGS(APPLE_CFLAGS -mtune=prescott -mmmx -mfpmath=sse -malign-double)
	ENDIF ( NOT "${CMAKE_OSX_ARCHITECTURES}" MATCHES "x86_64" AND NOT "${CMAKE_OSX_ARCHITECTURES}" MATCHES "ppc" )

	# ppc only?
	IF ( NOT "${CMAKE_OSX_ARCHITECTURES}" MATCHES "x86_64" AND NOT "${CMAKE_OSX_ARCHITECTURES}" MATCHES "i386" )
		ADD_FLAGS(APPLE_CFLAGS -faltivec)
	ENDIF ( NOT "${CMAKE_OSX_ARCHITECTURES}" MATCHES "x86_64" AND NOT "${CMAKE_OSX_ARCHITECTURES}" MATCHES "i386" )

	ADD_FLAGS(APPLE_CFLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} -fexceptions -ffast-math -msse -fomit-frame-pointer)
	ADD_FLAGS(APPLE_LDFLAGS -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} -fexceptions)
	
	# set compiler flags
	FORCE_ADD_FLAGS(CMAKE_C_FLAGS ${APPLE_CFLAGS})
	FORCE_ADD_FLAGS(CMAKE_CXX_FLAGS ${APPLE_CFLAGS})
	
	# set linker flags  -lstdc++ -lgcc_s.1 -lgcc_eh
	FORCE_ADD_FLAGS(CMAKE_EXE_LINKER_FLAGS ${APPLE_LDFLAGS})
	FORCE_ADD_FLAGS(CMAKE_MODULE_LINKER_FLAGS ${APPLE_LDFLAGS} )
	FORCE_ADD_FLAGS(CMAKE_SHARED_LINKER_FLAGS ${APPLE_LDFLAGS} )

ENDIF ( NOT "${COMPILEARCH}" STREQUAL "" )

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
