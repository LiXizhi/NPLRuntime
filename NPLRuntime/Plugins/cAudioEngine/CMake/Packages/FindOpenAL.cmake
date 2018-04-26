# - Try to find OPENAL
# Once done, this will define
#
#  OPENAL_FOUND - system has OPENAL
#  OPENAL_INCLUDE_DIRS - the OPENAL include directories 
#  OPENAL_LIBRARIES - link these to use OPENAL

include(FindPkgMacros)
findpkg_begin(OPENAL)

# Get path, convert backslashes as ${ENV_${var}}
getenv_path(OPENAL_HOME)

# construct search paths
set(OPENAL_PREFIX_PATH 	${OPENAL_HOME} 
										${ENV_OPENAL_HOME} 
										${CAUDIO_DEPENDENCIES_DIR}
										~/Library/Frameworks
										/Library/Frameworks
										/usr/local
										/usr
										/sw # Fink
										/opt/local # DarwinPorts
										/opt/csw # Blastwave
										/opt)

create_search_paths(OPENAL)
# redo search if prefix path changed
clear_if_changed(OPENAL_PREFIX_PATH
  OPENAL_LIBRARY_REL
  OPENAL_LIBRARY_DBG
  OPENAL_INCLUDE_DIR
)


set(OPENAL_LIBRARY_NAMES OpenAL32 OpenAL al openal)

get_debug_names(OPENAL_LIBRARY_NAMES)

find_path(OPENAL_INCLUDE_DIR NAMES al.h HINTS ${OPENAL_INC_SEARCH_PATH} PATH_SUFFIXES include/AL include/OpenAL include)
find_library(OPENAL_LIBRARY_REL NAMES ${OPENAL_LIBRARY_NAMES} HINTS ${OPENAL_LIB_SEARCH_PATH} ${OPENAL_PKGC_LIBRARY_DIRS} PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64 release relwithdebinfo minsizerel)
find_library(OPENAL_LIBRARY_DBG NAMES ${OPENAL_LIBRARY_NAMES_DBG} HINTS ${OPENAL_LIB_SEARCH_PATH} ${OPENAL_PKGC_LIBRARY_DIRS} PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64 debug)

make_library_set(OPENAL_LIBRARY)

findpkg_finish(OPENAL)