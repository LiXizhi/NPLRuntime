# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2014.7.29
# Note: NPLMono2 is a ParaEngine plugin (dll) that exposes ParaEngine API to the C# mono scripting interface. It is upgraded version for NPLMono.
# Desc:
# ---+++ Build in Windows:

# see http://www.mono-project.com/Embedding_Mono

# NPLMono2 needs mono in order to compile. In windows, please install mono binary(default is D:/Program Files (x86)/Mono-3.2.3) and Set MONO_ROOT variable, please use forward slash / in file name.
# Currently mono path is hard coded in source code using mono_set_dirs() function during compilation in MSVC.
# One can deploy as follow and use mono_set_dirs("mono\\lib", "mono\\etc");  if one wants to deploy mono with the game engine. NPLMono can be used with both client and server. However it is not recommended to use on client, because one needs to redistribute mono runtime which is pretty big.
# NPLMono2.dll
# mono-2.0.dll (rename from bin/libmonosgen-2.0.dll)
# libglib-2.0-0.dll
# libgthread-2.0-0.dll
# mono\etc  <- copy of MONO_ROOT\etc
# mono\lib  <- copy of MONO_ROOT\lib
#
# Upgrade Note: One needs to upgrade \trunk\NPLMono\NPLMono\lib\mono.def(lib) if mono api changes, this is only required on windows. Linux uses ELF and this step is not needed.
#
# ---+++ Build in Linux:
# it is much easier to build on linux and NPLMono is currently recommended to use only on server side. Install Mono from source in the official way.  Make sure pkg-config is installed.
# Run the default cmake, the NPLMono2 will automatically find all mono libraries installed on the machine.
include(${PROJECT_SOURCE_DIR}/cmake/ConfigBoost.cmake)
set(BUILD_NPLMONO2 true)

if(WIN32)
	# the mono installation directory in windows, modify this to match yours
	set(MONO_ROOT "C:/Program Files (x86)/Mono")  # this is mono 5.0 in mine
	# set(MONO_ROOT "D:/Program Files (x86)/Mono-3.2.3")
	# set(MONO_ROOT "D:/Program Files (x86)/Mono-2.10.9")
	# 3.4.0 from source
	# set(MONO_ROOT "D:/cygwin/opt/mymono")

	# we can locate the mono root by following command in windows
	FIND_PROGRAM (MONO_EXECUTABLE mono
		"${MONO_ROOT}/bin"
	)

	# if mono is not found, remove this project from build.
	IF(EXISTS ${MONO_ROOT})
		set(BUILD_NPLMONO2 true)
	ELSE()
		set(BUILD_NPLMONO2 false)
	ENDIF()
else()
	FIND_PROGRAM (MONO_EXECUTABLE mono)

	if(NOT MONO_EXECUTABLE)
		message("Note: Could not find Mono, please install mono-runtime first! Skip building NPLMono2 plugin")
		set(BUILD_NPLMONO2 false)
	else()
		message("Mono executable found, we will try to build NPLMono plugin")
	endif()

	# The following is to use pkg-config to generate compile and link time flags.
	find_package(PkgConfig QUIET)
	if (PKG_CONFIG_FOUND)
		pkg_check_modules(_MONO mono-2)
	endif()

	find_path(MONO_INCLUDE_DIR
		NAMES mono/jit/jit.h
		HINTS
			${_MONO_INCLUDE_DIRS}
		PATHS
			/usr/include /usr/local/include /opt/local/include)

	find_library(MONO_LIB
		NAMES mono-2.0
		HINTS
			${_MONO_LIBRARY_DIRS}
		PATHS
			/usr/lib /usr/local/lib /opt/local/lib)

	if (MONO_INCLUDE_DIR)
		set(MONO_INCLUDE_DIRS ${MONO_INCLUDE_DIR}
			CACHE PATH "MONO include dir")
		set(MONO_LIBRARIES "${MONO_LIB}"
			CACHE STRING "MONO libraries")
		include(FindPackageHandleStandardArgs)
		find_package_handle_standard_args(MONO DEFAULT_MSG MONO_LIB
			MONO_INCLUDE_DIR)
	else()
		set(BUILD_NPLMONO2 false)
	endif()

    if(BUILD_NPLMONO2)
        SET(MONO_VERSION_MAJOR)
        SET(MONO_VERSION_MINOR)
        SET(MONO_VERSION_PATCH)

        EXEC_PROGRAM("mono" ARGS --version OUTPUT_VARIABLE MONO_VERSION)
        STRING(REGEX REPLACE "[^\\.]+([0-9]+)\\.([0-9]+)\\.([0-9]+).*" "\\1 \\2 \\3" MONO_VERSION ${MONO_VERSION})
        SEPARATE_ARGUMENTS(MONO_VERSION)
        LIST(LENGTH MONO_VERSION MONO_VERSION_LENGTH)

        IF(MONO_VERSION_LENGTH EQUAL 3)
            LIST(GET MONO_VERSION 0 MONO_VERSION_MAJOR)
            LIST(GET MONO_VERSION 1 MONO_VERSION_MINOR)
            LIST(GET MONO_VERSION 2 MONO_VERSION_PATCH)

            MESSAGE(STATUS "MONO version: " ${MONO_VERSION})
            if (MONO_VERSION_MAJOR GREATER 3)
                MESSAGE(STATUS "MAJOR:" ${MONO_VERSION_MAJOR})
            else()
                MESSAGE(STATUS "MONO version 5.0 or above is required")
                set(BUILD_NPLMONO2 false)
            endif()
        ENDIF(MONO_VERSION_LENGTH EQUAL 3)

    endif()
endif()

if(BUILD_NPLMONO2)
PROJECT(NPLMono2)


set(CLIENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../ParaEngine)

#####################################
##----- source files for NPLMono---##
SET(NPLMono2_SRCS
	MonoClassDesc.cpp
	MonoScriptingState.cpp
	MonoScriptingState.h
	NPLMonoInterface.h		NPLMonoInterface.cpp
	NPLMonoInterface_Bind.h NPLMonoInterface_Bind.cpp
	marshal_wrapper.h		marshal_wrapper.cpp
	NPLMono.cpp
	NPLMono.h
	NPLMono2.def
	readme.txt
)

##############################
# util files
SET(NPLMono2_UTIL_FILES
	#util/Platforms.h
    #util/os_calls.h  util/os_calls.cpp
)
SOURCE_GROUP("Util Files" FILES ${NPLMono2_UTIL_FILES})
list(APPEND NPLMono2_SRCS ${NPLMono2_UTIL_FILES})

#####################################
##--------- CS interface file ---------##
set(NPLMono2_interface_Files
	NPLMonoInterface.cs
)

# custom build rules: cs interface files
foreach( src_file ${NPLMono2_interface_Files} )
	get_filename_component(src_filename ${src_file} NAME_WE)
	set(NPLMono2_interface_bin_Files ${NPLMono2_interface_bin_Files} "${src_filename}.dll")
	if (WIN32)
		add_custom_command(
			OUTPUT "${src_filename}.dll"
			COMMAND csc /target:library /out:"${src_filename}.dll" ${src_file}
			DEPENDS ${src_file}
			COMMENT "csc compiling: ${src_file}"
			)
	else (WIN32)
		add_custom_command(
			OUTPUT "${src_filename}.dll"
			COMMAND mcs -target:library -out:"${src_filename}.dll" ${src_file}
			DEPENDS ${src_file}
			COMMENT "mcs compiling: ${src_file}"
			)
	endif (WIN32)
endforeach( src_file ${NPLMono2_interface_Files} )
set_source_files_properties(${NPLMono2_interface_bin_Files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties(${NPLMono2_interface_Files} PROPERTIES HEADER_FILE_ONLY TRUE)
SOURCE_GROUP("CS interface Files\\Bin" FILES ${NPLMono2_interface_bin_Files})
SOURCE_GROUP("CS interface Files" FILES ${NPLMono2_interface_Files})
list(APPEND NPLMono2_SRCS ${NPLMono2_interface_bin_Files} ${NPLMono2_interface_Files})

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${CLIENT_SOURCE_DIR}")
include_directories("${CLIENT_SOURCE_DIR}/Core")
include_directories("${CLIENT_SOURCE_DIR}/util")
include_directories("${CLIENT_SOURCE_DIR}/Framework/Interface")
include_directories("${Boost_INCLUDE_DIRS}")

IF(MSVC)
	# statically link MSVC to reduce dependancies
	foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
	 	if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
	 	endif(${flag_var} MATCHES "/MD")
	 	if(${flag_var} MATCHES "/MDd")
	 		string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
		endif(${flag_var} MATCHES "/MDd")
	endforeach(flag_var)

	# generate program database (PDB symbol files even in release build)
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
	# To turn size optimisations on again
	SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /opt:ref")
ENDIF()

if (WIN32)
	include_directories("${MONO_ROOT}\\include\\mono-2.0")
	add_definitions(-DMONO_ROOT="${MONO_ROOT}")
	link_directories ("${PROJECT_SOURCE_DIR}/lib")
	link_directories ("${MONO_ROOT}/lib")
	set(LINK_LIBRARIES 	mono)

	SET(CMAKE_DEBUG_POSTFIX "_d")

else (WIN32)
	include_directories("${MONO_INCLUDE_DIRS}")
	set(LINK_LIBRARIES 	${MONO_LIBRARIES})
endif (WIN32)

link_libraries( ${LINK_LIBRARIES})

add_library(NPLMono2 SHARED ${NPLMono2_SRCS})


ADD_CUSTOM_COMMAND(TARGET NPLMono2 POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:NPLMono2> ${OUTPUT_BIN_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/NPLMonoInterface.dll ${OUTPUT_BIN_DIR}
)

endif(BUILD_NPLMONO2)
