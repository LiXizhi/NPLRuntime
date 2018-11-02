# Note: Modified by LiXizhi to build only required libs for NPLRuntime
# Date: 2016.10.16

cmake_minimum_required( VERSION 2.6 )
PROJECT( Assimp )


# Define here the needed parameters
set (ASSIMP_VERSION_MAJOR 3)
set (ASSIMP_VERSION_MINOR 1)
set (ASSIMP_VERSION_PATCH 1) # subversion revision?
set (ASSIMP_VERSION ${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}.${ASSIMP_VERSION_PATCH})
set (ASSIMP_SOVERSION 3)
set (PROJECT_VERSION "${ASSIMP_VERSION}")

set(ASSIMP_PACKAGE_VERSION "0" CACHE STRING "the package-specific version used for uploading the sources")

if(NOT GIT_COMMIT_HASH)
  set(GIT_COMMIT_HASH 0)
endif(NOT GIT_COMMIT_HASH)

configure_file(
  ${PROJECT_SOURCE_DIR}/revision.h.in
  ${PROJECT_BINARY_DIR}/revision.h
)

include_directories(${PROJECT_BINARY_DIR})

option(ASSIMP_OPT_BUILD_PACKAGES "Set to ON to generate CPack configuration files and packaging targets" OFF)
set(LIBASSIMP_COMPONENT     "libassimp${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}.${ASSIMP_VERSION_PATCH}" )
set(LIBASSIMP-DEV_COMPONENT "libassimp${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}.${ASSIMP_VERSION_PATCH}-dev" )
set(CPACK_COMPONENTS_ALL assimp-bin ${LIBASSIMP_COMPONENT} ${LIBASSIMP-DEV_COMPONENT} assimp-dev)
set(ASSIMP_LIBRARY_SUFFIX "" CACHE STRING "Suffix to append to library names")

if((CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX) AND NOT CMAKE_COMPILER_IS_MINGW)
  add_definitions(-fPIC) # this is a very important switch and some libraries seem now to have it....
  # hide all not-exported symbols
  add_definitions( -fvisibility=hidden -Wall )
elseif(MSVC)
  # enable multi-core compilation with MSVC
  add_definitions(/MP)
endif()

INCLUDE (FindPkgConfig)
INCLUDE_DIRECTORIES( include )


# Cache these to allow the user to override them manually.
SET( ASSIMP_LIB_INSTALL_DIR "lib" CACHE PATH
	"Path the built library files are installed to." )
SET( ASSIMP_INCLUDE_INSTALL_DIR "include" CACHE PATH
	"Path the header files are installed to." )
SET( ASSIMP_BIN_INSTALL_DIR "bin" CACHE PATH
	"Path the tool executables are installed to." )
SET ( ASSIMP_BUILD_STATIC_LIB ON CACHE BOOL
    "Build a static (.a) version of the library" )

SET(ASSIMP_DEBUG_POSTFIX "d" CACHE STRING "Debug Postfitx for lib, samples and tools")


# Generate a pkg-config .pc for the Assimp library.
CONFIGURE_FILE( "${PROJECT_SOURCE_DIR}/assimp.pc.in" "${PROJECT_BINARY_DIR}/assimp.pc" @ONLY )
INSTALL( FILES "${PROJECT_BINARY_DIR}/assimp.pc" DESTINATION ${ASSIMP_LIB_INSTALL_DIR}/pkgconfig/ COMPONENT ${LIBASSIMP-DEV_COMPONENT})


# Globally enable Boost resp. the Boost workaround �?it is also needed by the
# tools which include the Assimp headers.
SET ( ASSIMP_ENABLE_BOOST_WORKAROUND OFF CACHE BOOL
	"If a simple implementation of the used Boost functions is used. Slightly reduces functionality, but enables builds without Boost available."
)
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIRS} )

# cmake configuration files
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/assimp-config.cmake.in"         "${CMAKE_CURRENT_BINARY_DIR}/assimp-config.cmake" @ONLY IMMEDIATE)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/assimp-config-version.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/assimp-config-version.cmake" @ONLY IMMEDIATE)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/assimp-config.cmake"             "${CMAKE_CURRENT_BINARY_DIR}/assimp-config-version.cmake" DESTINATION "${ASSIMP_LIB_INSTALL_DIR}/cmake/assimp-${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}" COMPONENT ${LIBASSIMP-DEV_COMPONENT})

SET ( ASSIMP_NO_EXPORT OFF CACHE BOOL
	"Disable Assimp's export functionality." 
)

# Search for external dependencies, and build them from source if not found
# Search for zlib
if (ZLIB_FOUND)
	INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
endif (ZLIB_FOUND)
ADD_DEFINITIONS( -DASSIMP_BUILD_NO_OWN_ZLIB)

# Search for unzip
if (PKG_CONFIG_FOUND)
	PKG_CHECK_MODULES(UNZIP minizip)
endif (PKG_CONFIG_FOUND)

IF ( ASSIMP_NO_EXPORT )
	ADD_DEFINITIONS( -DASSIMP_BUILD_NO_EXPORT)
	MESSAGE( STATUS "Build an import-only version of Assimp." )
ENDIF( ASSIMP_NO_EXPORT )

# if(CMAKE_CL_64)
# 	set(ASSIMP_BUILD_ARCHITECTURE "amd64")
# else(CMAKE_CL_64)
# 	set(ASSIMP_BUILD_ARCHITECTURE "x86")
# endif(CMAKE_CL_64)
SET ( ASSIMP_BUILD_ARCHITECTURE "" CACHE STRING 
	"describe the current architecture."
)
IF    ( ASSIMP_BUILD_ARCHITECTURE STREQUAL "")
ELSE  ( ASSIMP_BUILD_ARCHITECTURE STREQUAL "")
	ADD_DEFINITIONS ( -D'ASSIMP_BUILD_ARCHITECTURE="${ASSIMP_BUILD_ARCHITECTURE}"' )
ENDIF ( ASSIMP_BUILD_ARCHITECTURE STREQUAL "")

# ${CMAKE_GENERATOR}
SET ( ASSIMP_BUILD_COMPILER "" CACHE STRING 
	"describe the current compiler."
)
IF    ( ASSIMP_BUILD_COMPILER STREQUAL "")
ELSE  ( ASSIMP_BUILD_COMPILER STREQUAL "")
	ADD_DEFINITIONS ( -D'ASSIMP_BUILD_COMPILER="${ASSIMP_BUILD_COMPILER}"' )
ENDIF ( ASSIMP_BUILD_COMPILER STREQUAL "")

MARK_AS_ADVANCED ( ASSIMP_BUILD_ARCHITECTURE ASSIMP_BUILD_COMPILER )

ADD_SUBDIRECTORY( code/ )
SET ( ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL
	"If the supplementary tools for Assimp are built in addition to the library."
)

SET ( ASSIMP_BUILD_SAMPLES OFF CACHE BOOL
	"If the official samples are built as well (needs Glut)."
)

IF(MSVC)
	SET ( ASSIMP_INSTALL_PDB OFF CACHE BOOL
		"Install MSVC debug files."
	)
ENDIF(MSVC)

if(CMAKE_CPACK_COMMAND AND UNIX AND ASSIMP_OPT_BUILD_PACKAGES)
  # Packing information
  set(CPACK_PACKAGE_NAME                    "assimp{ASSIMP_VERSION_MAJOR}")
  set(CPACK_PACKAGE_CONTACT "" CACHE STRING "Package maintainer and PGP signer.")
  set(CPACK_PACKAGE_VENDOR                  "http://assimp.sourceforge.net/")
  set(CPACK_PACKAGE_DISPLAY_NAME            "Assimp ${ASSIMP_VERSION}")
  set(CPACK_PACKAGE_DESCRIPTION_SUMMARY     " - Open Asset Import Library ${ASSIMP_VERSION}")
  set(CPACK_PACKAGE_VERSION                 "${ASSIMP_VERSION}.${ASSIMP_PACKAGE_VERSION}" )
  set(CPACK_PACKAGE_VERSION_MAJOR           "${ASSIMP_VERSION_MAJOR}")
  set(CPACK_PACKAGE_VERSION_MINOR           "${ASSIMP_VERSION_MINOR}")
  set(CPACK_PACKAGE_VERSION_PATCH           "${ASSIMP_VERSION_PATCH}")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY       "assimp${ASSIMP_VERSION_MAJOR}.${ASSIMP_VERSION_MINOR}")
  #set(CPACK_PACKAGE_DESCRIPTION_FILE       "${CMAKE_CURRENT_SOURCE_DIR}/description")
  set(CPACK_RESOURCE_FILE_LICENSE           "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

  string(TOUPPER ${LIBASSIMP_COMPONENT}     "LIBASSIMP_COMPONENT_UPPER")
  string(TOUPPER ${LIBASSIMP-DEV_COMPONENT} "LIBASSIMP-DEV_COMPONENT_UPPER")

  set(CPACK_COMPONENT_ASSIMP-BIN_DISPLAY_NAME                       "tools")
  set(CPACK_COMPONENT_ASSIMP-BIN_DEPENDS                            "${LIBASSIMP_COMPONENT}" )
  set(CPACK_COMPONENT_${LIBASSIMP_COMPONENT_UPPER}_DISPLAY_NAME     "libraries")
  set(CPACK_COMPONENT_${LIBASSIMP-DEV_COMPONENT_UPPER}_DISPLAY_NAME "common headers and installs")
  set(CPACK_COMPONENT_${LIBASSIMP-DEV_COMPONENT_UPPER}_DEPENDS $    "{LIBASSIMP_COMPONENT}" )
  set(CPACK_COMPONENT_ASSIMP-DEV_DISPLAY_NAME                       "${CPACK_COMPONENT_${LIBASSIMP-DEV_COMPONENT}_DISPLAY_NAME}" )
  set(CPACK_COMPONENT_ASSIMP-DEV_DEPENDS                            "${LIBASSIMP-DEV_COMPONENT}" )
  set(CPACK_DEBIAN_BUILD_DEPENDS debhelper cmake libboost-dev libboost-thread-dev libboost-math-dev zlib1g-dev pkg-config)

  # debian
  set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
  set(CPACK_DEBIAN_CMAKE_OPTIONS    "-DBUILD_ASSIMP_SAMPLES:BOOL=${ASSIMP_BUILD_SAMPLES}")
  set(CPACK_DEBIAN_PACKAGE_SECTION  "libs" )
  set(CPACK_DEBIAN_PACKAGE_DEPENDS  "${CPACK_COMPONENTS_ALL}")
  set(CPACK_DEBIAN_PACKAGE_SUGGESTS)
  set(CPACK_DEBIAN_PACKAGE_NAME     "assimp")
  set(CPACK_DEBIAN_PACKAGE_REMOVE_SOURCE_FILES contrib/cppunit-1.12.1 contrib/cppunit_note.txt contrib/zlib workspaces test doc obj samples packaging)
  set(CPACK_DEBIAN_PACKAGE_SOURCE_COPY svn export --force)
  set(CPACK_DEBIAN_CHANGELOG)
  execute_process(COMMAND lsb_release -is
    OUTPUT_VARIABLE _lsb_distribution OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _lsb_release_failed)
  set(CPACK_DEBIAN_DISTRIBUTION_NAME ${_lsb_distribution} CACHE STRING "Name of the distrubiton")
  string(TOLOWER ${CPACK_DEBIAN_DISTRIBUTION_NAME} CPACK_DEBIAN_DISTRIBUTION_NAME)
  if( ${CPACK_DEBIAN_DISTRIBUTION_NAME} STREQUAL "ubuntu" )
    set(CPACK_DEBIAN_DISTRIBUTION_RELEASES lucid maverick natty oneiric precise CACHE STRING "Release code-names of the distrubiton release")
  endif()
  set(DPUT_HOST "" CACHE STRING "PPA repository to upload the debian sources")
  include(CPack)
  include(DebSourcePPA)
endif()
