# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2009.7.3
# Note: NPLMono is a ParaEngine plugin (dll) that exposes ParaEngine API to the C# mono scripting interface. 
# Desc: 
# ---+++ Build in Windows: 

# TODO: 2011.7.2: Mono API changes a bit since 2.8. But since it is not stable yet, I will not move to it now. see http://www.mono-project.com/Embedding_Mono

# NPLMono needs mono in order to compile. In windows, please install mono binary(default is E:/Program Files/Mono-2.4.2) and Set MONO_ROOT variable, please use forward slash / in file name. 
# Currently mono path is hard coded in source code using mono_set_dirs() function during compilation in MSVC. 
# One can deploy as follow and use mono_set_dirs("lib\\", "etc");  if one wants to deploy mono with the game engine. NPLMono can be used with both client and server. However it is not recommended to use on client, because one needs to redistribute mono runtime which is pretty big.  
# NPLMono.dll
# mono.dll 
# libgmodule-2.0-0.dll 
# libgthread-2.0-0.dll 
# etc\mono\  <- copy of E:/Program Files/Mono-2.4.2\etc\mono\ 
# lib\mono\  <- copy of E:/Program Files/Mono-2.4.2\lib\mono\ 
#
# Upgrade Note: One needs to upgrade \trunk\NPLMono\NPLMono\lib\mono.def(lib) if mono api changes, this is only required on windows. Linux uses ELF and this step is not needed. 
#
# ---+++ Build in Linux: 
# it is much easier to build on linux and NPLMono is currently recommended to use only on server side. Install Mono from source in the official way.  Make sure pkg-config is installed. 
# Run the default cmake, the NPLMono will automatically find all mono libraries installed on the machine. 

set(BUILD_NPLMONO true)

if(WIN32)
	# the mono installation directory in windows, modify this to match yours
	#set(MONO_ROOT "$ENV{PROGRAMFILES}/Mono-2.10.8")
	set(MONO_ROOT "D:/Program Files (x86)/Mono-3.2.3")
	
	# TODO: we can locate the mono root by following command in windows
	FIND_PROGRAM (MONO_EXECUTABLE mono
		#"$ENV{PROGRAMFILES}/Mono-2.10.8/bin"
		#"$ENV{PROGRAMFILES}/Mono-2.6.7/bin"
		"D:/Program Files (x86)/Mono-3.2.3/bin"
	)

	# TODO: if mono is not found, remove this project from build. 
	IF(EXISTS ${MONO_ROOT})
		set(BUILD_NPLMONO true)
	ELSE()	
		set(BUILD_NPLMONO false)
	ENDIF()
endif()

if(BUILD_NPLMONO)
PROJECT(NPLMono)

#####################################
##----- source files for NPLMono---##
SET(NPLMono_SRCS
	MonoClassDesc.cpp
	MonoScriptingState.cpp
	MonoScriptingState.h
	NPLMonoInterface.h		NPLMonoInterface.cpp		
	NPLMonoInterface_Bind.h NPLMonoInterface_Bind.cpp
	marshal_wrapper.h		marshal_wrapper.cpp
	NPLMono.cpp	
	NPLMono.h
	NPLMono.def
	readme.txt
)

##############################
# util files
SET(NPLMono_UTIL_FILES
	#util/Platforms.h
    #util/os_calls.h  util/os_calls.cpp
)
SOURCE_GROUP("Util Files" FILES ${NPLMono_UTIL_FILES})
list(APPEND NPLMono_SRCS ${NPLMono_UTIL_FILES})

#####################################
##--------- CS interface file ---------##
set(NPLMono_interface_Files
	NPLMonoInterface.cs
)

# custom build rules: cs interface files
foreach( src_file ${NPLMono_interface_Files} )
	get_filename_component(src_filename ${src_file} NAME_WE)
	set(NPLMono_interface_bin_Files ${NPLMono_interface_bin_Files} "${src_filename}.dll")
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
endforeach( src_file ${NPLMono_interface_Files} )
set_source_files_properties(${NPLMono_interface_bin_Files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties(${NPLMono_interface_Files} PROPERTIES HEADER_FILE_ONLY TRUE)
SOURCE_GROUP("CS interface Files\\Bin" FILES ${NPLMono_interface_bin_Files})
SOURCE_GROUP("CS interface Files" FILES ${NPLMono_interface_Files})
list(APPEND NPLMono_SRCS ${NPLMono_interface_bin_Files} ${NPLMono_interface_Files})

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${ParaEngineServer_SOURCE_DIR}")
include_directories("${Boost_INCLUDE_DIRS}")

IF(MSVC)
	# generate program database (PDB symbol files even in release build)
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
	# To turn size optimisations on again
	SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /opt:ref")
ENDIF()

if (WIN32)
	include_directories("${MONO_ROOT}\\include\\mono-2.0")
	include_directories("${MONO_ROOT}\\include\\glib-2.0")
	include_directories("${MONO_ROOT}\\lib\\glib-2.0\\include")
	add_definitions(-DMONO_ROOT="${MONO_ROOT}")
	
	link_directories ("${PROJECT_SOURCE_DIR}/lib")
	link_directories ("${ParaEngineServer_BINARY_DIR}/Release") 
	link_directories ("${MONO_ROOT}/lib") 
	set(LINK_LIBRARIES 	ParaEngineServer mono  glib-2.0 ${Boost_LIBRARIES})
  
else (WIN32)
	# The following is to use pkg-config to generate compile and link time flags. 
	INCLUDE( ${CMAKE_ROOT}/Modules/FindPkgConfig.cmake)
	
	IF ( PKG_CONFIG_EXECUTABLE )
		# set C_FLAGS and CXX_FLAGS
		execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --cflags mono
			OUTPUT_VARIABLE LIB_MONO_C_FLAGS
			OUTPUT_STRIP_TRAILING_WHITESPACE)
		
		execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --libs mono
			OUTPUT_VARIABLE LIB_MONO_LINK_FLAGS
			OUTPUT_STRIP_TRAILING_WHITESPACE)
         
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LIB_MONO_C_FLAGS}")
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LIB_MONO_C_FLAGS}")

		SET (CMAKE_SHARED_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} ${LIB_MONO_LINK_FLAGS})
		
	ELSE( PKG_CONFIG_EXECUTABLE )
		MESSAGE("warning: pkg-config is not found: NPLMono will not be compiled, please install mono first")
	ENDIF ( PKG_CONFIG_EXECUTABLE )
	
	set(LINK_LIBRARIES   		dl)
	
endif (WIN32)

link_libraries( ${LINK_LIBRARIES})

add_library(NPLMono SHARED ${NPLMono_SRCS})

if(MSVC)
ADD_CUSTOM_COMMAND(
   TARGET NPLMono
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/Release/NPLMono.dll ${PROJECT_SOURCE_DIR}/../../../../ParaWorld/NPLMono.dll
   COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/NPLMonoInterface.dll ${PROJECT_SOURCE_DIR}/../../../../ParaWorld/NPLMonoInterface.dll
)
endif(MSVC)
endif(BUILD_NPLMONO)