# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2009.4.10
# Desc: option modify PARAENGINE_COMPILE_LIB, define USE_HOARD if one wants to use hoard memory allocator in windows.

project (ParaEngineClient)

# The version number.
set (ParaEngineClient_VERSION_MAJOR 1)
set (ParaEngineClient_VERSION_MINOR 0)

if (WIN32)
# Note: if true, we will produce dll instead of exe, only under win32
set(PARAENGINE_COMPILE_LIB    true)
endif (WIN32)

# configure a header file to pass some of the CMake settings
# to the source code
if (WIN32)
configure_file (
  "${PROJECT_SOURCE_DIR}/ParaEngineClientConfig.h.in"
  "${PROJECT_SOURCE_DIR}/ParaEngineClientConfig.win32.h"
  )
SET(ParaEngineClient_HEADERS  ParaEngineClientConfig.win32.h)
else (WIN32)
configure_file (
  "${PROJECT_SOURCE_DIR}/ParaEngineClientConfig.h.in"
  "${PROJECT_SOURCE_DIR}/ParaEngineClientConfig.h"
  )
SET(ParaEngineClient_HEADERS  ParaEngineClientConfig.h)
endif (WIN32)

##############################
## Interface files that are usually included by external plugin dlls.
#SET(ParaEngineClient_API_FILES
#	Core/baseinterface.h		Core/PEtypes.h		Core/PluginAPI.h		Core/coreexport.h
#	Core/PluginLoader.hpp
#	Core/IParaEngineCore.h	Core/IParaEngineApp.h
#	Core/IParaPhysics.h
#	Core/IParaAudioEngine.h
#	Core/IParaWebService.h
#	Core/INPL.h	Core/NPLTypes.h Core/INPLRuntime.h Core/INPLRuntimeState.h	Core/INPLScriptingState.h	Core/NPLInterface.hpp
#	Core/InterprocessMsg.h Core/InterprocessQueue.hpp	Core/ipc_message_queue.hpp
#	Core/NPLMiniRuntime.hpp Core/ParaUtils.hpp
#	Core/IParaDebugDraw.h Core/IBatchedElementDraw.h Core/CommandLineParser.hpp
#	Core/IMovieCodec.h
#	# Core/IAppService.h	Core/ILogService.h	Core/IFileSystemWatcherService.h
#)
#SOURCE_GROUP("API Files" FILES ${ParaEngineClient_API_FILES})
#list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_API_FILES})

##############################
file (GLOB ParaEngineClient_Engine_FILES ${PROJECT_SOURCE_DIR}/Engine/*.cpp)
SOURCE_GROUP("Engine" FILES ${ParaEngineClient_Engine_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Engine_FILES})

##############################
file (GLOB ParaEngineClient_EngineHeader_FILES ${PROJECT_SOURCE_DIR}/Engine/*.h)
SOURCE_GROUP("Engine\\headers" FILES ${ParaEngineClient_EngineHeader_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_EngineHeader_FILES})

##############################
file (GLOB_RECURSE ParaEngineClient_DirMonitor_FILES ${PROJECT_SOURCE_DIR}/dir_monitor/*.*)
SOURCE_GROUP("Externals\\dir_monitor" FILES ${ParaEngineClient_DirMonitor_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_DirMonitor_FILES})

##############################
file (GLOB ParaEngineClient_Core_FILES ${PROJECT_SOURCE_DIR}/Core/*.cpp)
SOURCE_GROUP("Core" FILES ${ParaEngineClient_Core_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Core_FILES})

file (GLOB ParaEngineClient_CoreHeader_FILES ${PROJECT_SOURCE_DIR}/Core/*.h ${PROJECT_SOURCE_DIR}/Core/*.hpp)
SOURCE_GROUP("Core\\Headers" FILES ${ParaEngineClient_CoreHeader_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_CoreHeader_FILES})

##############################
#file (GLOB ParaEngineClient_MDXModel_FILES ${PROJECT_SOURCE_DIR}/mdxfile/*.*)
#SOURCE_GROUP("Engine\\mdxfile" FILES ${ParaEngineClient_MDXModel_FILES})
#list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_MDXModel_FILES})

##############################
file (GLOB ParaEngineClient_BMaxModel_FILES ${PROJECT_SOURCE_DIR}/BMaxModel/*.h ${PROJECT_SOURCE_DIR}/BMaxModel/*.cpp)
SOURCE_GROUP("Engine\\BMaxModel" FILES ${ParaEngineClient_BMaxModel_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_BMaxModel_FILES})

##############################
file (GLOB ParaEngineClient_renderer_FILES ${PROJECT_SOURCE_DIR}/renderer/*.h ${PROJECT_SOURCE_DIR}/renderer/*.cpp)
SOURCE_GROUP("renderer" FILES ${ParaEngineClient_renderer_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_renderer_FILES})

##############################
file (GLOB ParaEngineClient_pb_FILES ${PROJECT_SOURCE_DIR}/protocol/*.h ${PROJECT_SOURCE_DIR}/protocol/*.cpp)
SOURCE_GROUP("protocol" FILES ${ParaEngineClient_pb_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_pb_FILES})

##############################
file (GLOB ParaEngineClient_ParaXModel_FILES ${PROJECT_SOURCE_DIR}/ParaXModel/*.cpp ${PROJECT_SOURCE_DIR}/ParaXModel/*.h ${PROJECT_SOURCE_DIR}/ParaXModel/*.templates ${PROJECT_SOURCE_DIR}/ParaXModel/*.inl)
SOURCE_GROUP("Engine\\ParaXModel" FILES ${ParaEngineClient_ParaXModel_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_ParaXModel_FILES})

##############################
#file (GLOB ParaEngineClient_Physics_FILES ${PROJECT_SOURCE_DIR}/physics/*.*)
#SOURCE_GROUP("physics" FILES ${ParaEngineClient_Physics_FILES})
#list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Physics_FILES})

##############################
file (GLOB ParaEngineClient_VoxelMesh_FILES ${PROJECT_SOURCE_DIR}/VoxelMesh/*.*)
SOURCE_GROUP("Engine\\VoxelMesh" FILES ${ParaEngineClient_VoxelMesh_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_VoxelMesh_FILES})


##############################
file (GLOB ParaEngineClient_IO_FILES ${PROJECT_SOURCE_DIR}/IO/*.cpp ${PROJECT_SOURCE_DIR}/IO/*.h)
SOURCE_GROUP("IO" FILES ${ParaEngineClient_IO_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_IO_FILES})

##############################
file (GLOB ParaEngineClient_CommonFramework_FILES ${PROJECT_SOURCE_DIR}/CommonFramework/*.*)
SOURCE_GROUP("util\\CommonFramework" FILES ${ParaEngineClient_CommonFramework_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_CommonFramework_FILES})

##############################
file (GLOB ParaEngineClient_Common_FILES ${PROJECT_SOURCE_DIR}/common/*.*)
SOURCE_GROUP("Externals\\CommonDirectX" FILES ${ParaEngineClient_Common_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Common_FILES})

##############################
file (GLOB ParaEngineClient_CurlLua_FILES ${PROJECT_SOURCE_DIR}/curllua/*.*)
SOURCE_GROUP("Externals\\curllua" FILES ${ParaEngineClient_CurlLua_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_CurlLua_FILES})

##############################
file (GLOB ParaEngineClient_DebugTools_FILES ${PROJECT_SOURCE_DIR}/debugtools/*.*)
SOURCE_GROUP("Externals\\debugtools" FILES ${ParaEngineClient_DebugTools_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_DebugTools_FILES})

##############################
file (GLOB ParaEngineClient_InfoCenter_FILES ${PROJECT_SOURCE_DIR}/ic/*.h ${PROJECT_SOURCE_DIR}/ic/*.cpp)
SOURCE_GROUP("Externals\\InfoCenter" FILES ${ParaEngineClient_InfoCenter_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_InfoCenter_FILES})

##############################
file (GLOB ParaEngineClient_WebBrowser_FILES ${PROJECT_SOURCE_DIR}/WebBrowser/*.*)
SOURCE_GROUP("Externals\\WebBrowser" FILES ${ParaEngineClient_WebBrowser_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_WebBrowser_FILES})

##############################
file (GLOB ParaEngineClient_TERRAIN_FILES ${PROJECT_SOURCE_DIR}/terrain/*.h ${PROJECT_SOURCE_DIR}/terrain/*.cpp)
SOURCE_GROUP("terrain" FILES ${ParaEngineClient_TERRAIN_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_TERRAIN_FILES})

##############################
file (GLOB ParaEngineClient_Block_FILES ${PROJECT_SOURCE_DIR}/BlockEngine/*.cpp ${PROJECT_SOURCE_DIR}/BlockEngine/*.h)
SOURCE_GROUP("BlockEngine" FILES ${ParaEngineClient_Block_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Block_FILES})

##############################
file (GLOB ParaEngineClient_Math_FILES ${PROJECT_SOURCE_DIR}/math/*.cpp ${PROJECT_SOURCE_DIR}/math/*.h ${PROJECT_SOURCE_DIR}/math/*.inl)
SOURCE_GROUP("math" FILES ${ParaEngineClient_Math_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Math_FILES})

##############################
#file (GLOB ParaEngineClient_Jabber_FILES ${PROJECT_SOURCE_DIR}/jabber/*.*)
#SOURCE_GROUP("jabber" FILES ${ParaEngineClient_Jabber_FILES})
#list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Jabber_FILES})

##############################
file (GLOB ParaEngineClient_NPL_FILES ${PROJECT_SOURCE_DIR}/NPL/*.h ${PROJECT_SOURCE_DIR}/NPL/*.cpp ${PROJECT_SOURCE_DIR}/NPL/*.hpp ${PROJECT_SOURCE_DIR}/NPL/*.txt ${PROJECT_SOURCE_DIR}/NPL/*.xml)
SOURCE_GROUP("NPL" FILES ${ParaEngineClient_NPL_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_NPL_FILES})

##############################
file (GLOB ParaEngineClient_Scripting_FILES ${PROJECT_SOURCE_DIR}/ParaScriptBindings/*.cpp ${PROJECT_SOURCE_DIR}/ParaScriptBindings/*.h)
SOURCE_GROUP("ParaScriptBindings" FILES ${ParaEngineClient_Scripting_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Scripting_FILES})

##############################
file (GLOB ParaEngineClient_GUI_FILES ${PROJECT_SOURCE_DIR}/2dengine/*.cpp ${PROJECT_SOURCE_DIR}/2dengine/*.h)
SOURCE_GROUP("2dengine" FILES ${ParaEngineClient_GUI_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_GUI_FILES})

##############################
file (GLOB ParaEngineClient_painting_FILES ${PROJECT_SOURCE_DIR}/PaintEngine/*.cpp ${PROJECT_SOURCE_DIR}/PaintEngine/*.h)
SOURCE_GROUP("PaintEngine" FILES ${ParaEngineClient_painting_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_painting_FILES})

##############################
file (GLOB ParaEngineClient_3D_FILES ${PROJECT_SOURCE_DIR}/3dengine/*.cpp ${PROJECT_SOURCE_DIR}/3dengine/*.h ${PROJECT_SOURCE_DIR}/3dengine/*.cpp)
SOURCE_GROUP("3dengine" FILES ${ParaEngineClient_3D_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_3D_FILES})

#####################################
##--------- resource file ---------##
# winmain.rc is the only buildable file, but the rest are all dependencies of it.
file (GLOB ParaEngineClient_RESOURCE_FILES ${PROJECT_SOURCE_DIR}/res/*.*)
set_source_files_properties(${ParaEngineClient_RESOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
set(ParaEngineClient_RESOURCE_FILES     ParaEngineRes.rc 	${ParaEngineClient_RESOURCE_FILES}	)
SOURCE_GROUP("Externals\\res" FILES ${ParaEngineClient_RESOURCE_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_RESOURCE_FILES})

###################################
##------------shader-------------##
# Add shader files to the project.
file (GLOB ParaEngineClient_SHADER_FILES RELATIVE ${PROJECT_SOURCE_DIR} shaders/*.fx)

# custom build rules: effect FX files
foreach( src_file ${ParaEngineClient_SHADER_FILES} )
	get_filename_component(src_filename ${src_file} NAME_WE)
	set(ParaEngineClient_SHADER_BIN_FILES ${ParaEngineClient_SHADER_BIN_FILES} "shaders/${src_filename}.fxo")
	add_custom_command(
		OUTPUT "${PROJECT_SOURCE_DIR}/shaders/${src_filename}.fxo"
		COMMAND "${DIRECTX_INCLUDE_DIR}/../Utilities/bin/x86/fxc" /Tfx_2_0 /Gfp /nologo /Fo "${PROJECT_SOURCE_DIR}/shaders/${src_filename}.fxo" ${PROJECT_SOURCE_DIR}/${src_file}
		# uncomment the following line if you want to see the text output to debug shader instructions.
		# COMMAND fxc /Tfx_2_0 /Gfp /nologo /Fc "${PROJECT_SOURCE_DIR}/shaders/${src_filename}.txt" ${PROJECT_SOURCE_DIR}/${src_file}
		DEPENDS ${PROJECT_SOURCE_DIR}/${src_file}
		COMMENT "fxc compile: ${src_file}"
		)
endforeach( src_file ${ParaEngineClient_SHADER_FILES} )
set_source_files_properties(${ParaEngineClient_SHADER_BIN_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_SHADER_BIN_FILES})
SOURCE_GROUP("Shader Files\\Compiled Files" FILES ${ParaEngineClient_SHADER_BIN_FILES})

# header files
file (GLOB ParaEngineClient_SHADER_HEADER_FILES ${PROJECT_SOURCE_DIR}/shaders/*.h)
set_source_files_properties(${ParaEngineClient_SHADER_HEADER_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
SOURCE_GROUP("Shader Files" FILES ${ParaEngineClient_SHADER_FILES})
SOURCE_GROUP("Shader Files\\Headers Files" FILES ${ParaEngineClient_SHADER_HEADER_FILES})

list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_SHADER_FILES} ${ParaEngineClient_SHADER_HEADER_FILES})

##############################
# embedded resource 
set(EmbeddedResource_FILES)
list(APPEND EmbeddedResource_FILES ${ParaEngineClient_SOURCE_DIR}/ResourceEmbedded.cpp)
list(APPEND ParaEngineClient_SRCS ${EmbeddedResource_FILES})
SOURCE_GROUP("ResEmbedded" FILES ${EmbeddedResource_FILES})


##############################
# util files
file (GLOB ParaEngineClient_UTIL_FILES ${PROJECT_SOURCE_DIR}/util/*.h ${PROJECT_SOURCE_DIR}/util/*.cpp ${PROJECT_SOURCE_DIR}/util/*.hpp ${PROJECT_SOURCE_DIR}/util/*.c)
SOURCE_GROUP("util" FILES ${ParaEngineClient_UTIL_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_UTIL_FILES})

##############################
# Documentation
file (GLOB ParaEngineClient_DOCUMENT_FILES ${PROJECT_SOURCE_DIR}/doc/*.*)
SOURCE_GROUP("doc" FILES ${ParaEngineClient_DOCUMENT_FILES})
set_source_files_properties(${ParaEngineClient_DOCUMENT_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_DOCUMENT_FILES})

##############################
# WebSocket
file (GLOB ParaEngineClient_WebSocket_FILES ${ParaEngineClient_SOURCE_DIR}/WebSocket/*.cpp ${ParaEngineClient_SOURCE_DIR}/WebSocket/*.h)
SOURCE_GROUP("NPL\\WebSocket" FILES ${ParaEngineClient_WebSocket_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_WebSocket_FILES})

##############################
# Platform
file (GLOB ParaEngineClient_PLATFORM_FILES ${PROJECT_SOURCE_DIR}/platform/*.h ${PROJECT_SOURCE_DIR}/platform/*.cpp)
SOURCE_GROUP("platform" FILES ${ParaEngineClient_PLATFORM_FILES})
set_source_files_properties(${ParaEngineClient_PLATFORM_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_PLATFORM_FILES})

##############################
# config
SET(ParaEngineClient_CONFIG_FILES
	ParaEngineClientConfig.h.in
	PEClientInstall.cmake
	doc/config.txt
	#${CLIENT_SOURCE_DIR}/../../ParaEngine_SDK.nsi
)
source_group("Config Files" FILES ${ParaEngineClient_CONFIG_FILES})
set_source_files_properties(${ParaEngineClient_CONFIG_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_CONFIG_FILES})

##############################
file (GLOB ParaEngineClient_Cad_FILES ${PROJECT_SOURCE_DIR}/CadModel/*.*)
SOURCE_GROUP("Externals\\CadModel" FILES ${ParaEngineClient_Cad_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Cad_FILES})

##########################################################################################
#
#
#   Add precompiled header support:  all files added to ParaEngineClient_SRCS
#		after this macro will NOT have precompiled header
#
#
##########################################################################################
include(${PROJECT_SOURCE_DIR}/MSVCPCHSupport.cmake)
MSVC_PCH_SUPPORT(${PROJECT_NAME})

##############################
# add the binary tree to the search path for include files
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/Core")
include_directories("${PROJECT_SOURCE_DIR}/renderer")
include_directories("${PROJECT_SOURCE_DIR}/3dengine")
include_directories("${PROJECT_SOURCE_DIR}/Engine")
include_directories("${PROJECT_SOURCE_DIR}/IO")
include_directories("${PROJECT_SOURCE_DIR}/util")
include_directories("${PROJECT_SOURCE_DIR}/math")
include_directories("${PROJECT_SOURCE_DIR}/ParaScriptBindings")
include_directories("${PROJECT_SOURCE_DIR}/NPL")
include_directories("${PROJECT_SOURCE_DIR}/2dengine/Video")
include_directories("${PROJECT_SOURCE_DIR}/common")
include_directories("${DIRECTX_INCLUDE_DIR}")
include_directories("${sqlite_SOURCE_DIR}")
include_directories("${Boost_INCLUDE_DIRS}")
include_directories("${luabind_SOURCE_DIR}")
include_directories("${tinyxpath_SOURCE_DIR}")
include_directories("${CLIENT_SOURCE_DIR}/FreeImage_3120/Dist")
include_directories("${CURL_BINARY_DIR}/include/curl/")
include_directories("${CURL_SOURCE_DIR}/include/")
include_directories("${lua_SOURCE_DIR}/src/")
include_directories("${CLIENT_SOURCE_DIR}/dxsdk_feb2005_extras/DirectShow/Include")
#include_directories("${PCRE_SOURCE_DIR}")
#include_directories("${PhysX_SOURCE_DIR}/Foundation/include")
#include_directories("${PhysX_SOURCE_DIR}/PhysXLoader/include")
#include_directories("${PhysX_SOURCE_DIR}/Cooking/include")
#include_directories("${PhysX_SOURCE_DIR}/Physics/include")
#include_directories("${RakNetStaticLib_SOURCE_DIR}/Source")
#include_directories("${NPLNetworkLayer_SOURCE_DIR}/include")
#include_directories("${FCollada_SOURCE_DIR}/FCollada/LibXML/include/")
#include_directories("${FCollada_SOURCE_DIR}")
#include_directories("${jblib_SOURCE_DIR}/src/")




include_directories("${jsoncpp_SOURCE_DIR}/include")
IF(WIN32)
	include_directories("${ZLIB_SOURCE_DIR}")
	include_directories("${Assimp_SOURCE_DIR}/include")
	include_directories("${Assimp_BINARY_DIR}/include")
	ADD_DEFINITIONS(-DSUPPORT_FBX_MODEL_FILE)
ENDIF(WIN32)

IF(WIN32)
	ADD_DEFINITIONS(-DPLATFORM_WINDOWS)
	ADD_DEFINITIONS(-D_PERFORMANCE_MONITOR)
	ADD_DEFINITIONS(-DLUA_BUILD_AS_DLL)
	ADD_DEFINITIONS(-D_WIN32_WINNT=0x0501)
	ADD_DEFINITIONS(-DUSE_SCHANNEL)
	ADD_DEFINITIONS(-DUSE_WINDOWS_SSPI)
	ADD_DEFINITIONS(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
	ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
	if(MSVC)
		ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
	endif(MSVC)
ELSE (WIN32)
    IF(APPLE)
        ADD_DEFINITIONS(-DPLATFORM_MAC)
    ELSE(APPLE)
        ADD_DEFINITIONS(-DPLATFORM_LINUX)
    ENDIF(APPLE)
ENDIF(WIN32)

ADD_DEFINITIONS(-DBOOST_SIGNALS_NO_DEPRECATION_WARNING)
ADD_DEFINITIONS(-DPARAENGINE_CLIENT)
ADD_DEFINITIONS(-DTIXML_USE_STL)

if(NPLRUNTIME_STATIC_LIB)
	ADD_DEFINITIONS(-DNPLRUNTIME_STATICLIB)
	ADD_DEFINITIONS(-DSQLITE_STATICLIB)
	ADD_DEFINITIONS(-DCURL_STATICLIB)
else()
	ADD_DEFINITIONS(-DPE_CORE_EXPORTING)
endif()

INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
CHECK_INCLUDE_FILE(
  "unistd.h" HAVE_UNISTD_H)
IF(HAVE_UNISTD_H)
  ADD_DEFINITIONS(-DHAVE_UNISTD_H)
ENDIF(HAVE_UNISTD_H)

IF(WIN32)
	# Make sure the linker can find the third-party precompiled libraries.
	# link_directories ("${CLIENT_SOURCE_DIR}/libs/win32/lib")
	# link_directories (${PhysX_SOURCE_DIR}/lib/win32)
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	# 64 bits version
else()
	# 32 bits version.
	link_directories (${PROJECT_SOURCE_DIR}/flashplayer/f_in_box/lib)
	link_directories("${CLIENT_SOURCE_DIR}/FreeImage_3120/Dist")
endif()

	link_directories (${DIRECTX_LIBRARY_DIR})
ENDIF(WIN32)


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

	# enable multiprocessor build option /MP, this will greatly increase compile speed
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP")
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
	SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MP")

	# /GR- Remove RTTI to miminize the executable size
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GR-")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
ENDIF()

# NOTE: This generate smaller and faster exe/dll, but no other libs should to it statically.
option(USE_LINK_TIME_CODE_GENERATION	"true to enable link time code gen in release build" OFF)

IF(MSVC)
	if(USE_LINK_TIME_CODE_GENERATION)
		# /GL Link time code generation. Note turn this off if you have plugins that links to ParaEngineClient.
		SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")
		# /LTCG Link time code generation
		if (PARAENGINE_COMPILE_LIB)
			SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LTCG:STATUS")
		else(PARAENGINE_COMPILE_LIB)
			SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG:STATUS")
		endif(PARAENGINE_COMPILE_LIB)
	endif(USE_LINK_TIME_CODE_GENERATION)


	# /GR- Remove RTTI to miminize the executable size
	# /Zi generate program database (PDB symbol files even in release build)
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi /GR-")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
	if (PARAENGINE_COMPILE_LIB)
		# To turn size optimisations on again
		SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LARGEADDRESSAWARE /opt:ref /DEBUG")
		# add a stripped version of symbol files during release build
		SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LARGEADDRESSAWARE /PDBSTRIPPED:Release/ParaEngineClient.stripped.pdb")
	else (PARAENGINE_COMPILE_LIB)
		# To turn size optimisations on again
		SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /opt:ref /DEBUG /LARGEADDRESSAWARE")
		# add a stripped version of symbol files during release build
		SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LARGEADDRESSAWARE /PDBSTRIPPED:Release/ParaEngineClient.stripped.pdb")
	endif (PARAENGINE_COMPILE_LIB)
ENDIF()


if (PARAENGINE_COMPILE_LIB)
	SET(ParaEngineClient_Lib_FILES
		Engine/LibInterface.cpp
		ParaEngineClient.def
	)
	list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Lib_FILES})

	SET(CMAKE_DEBUG_POSTFIX "_d")

	add_library(ParaEngineClient SHARED ${ParaEngineClient_SRCS})

else (PARAENGINE_COMPILE_LIB)
	# add the executable
	add_executable(ParaEngineClient ${ParaEngineClient_SRCS})

	# When this property is set to true the executable when linked on Windows will be created with a WinMain() entry point instead of of just main().
	set_target_properties(ParaEngineClient PROPERTIES WIN32_EXECUTABLE true)

endif (PARAENGINE_COMPILE_LIB)


if (WIN32)
  set(EXTRA_LIBRARIES
			Ws2_32.lib
			winmm.lib
			wsock32.lib
			Dnsapi.lib
			Secur32.lib
			Crypt32.lib
			Wldap32.lib
			zlib
			NPLProfiler

			# following are client only
			comctl32.lib
			d3dxof.lib
			dxguid.lib
			d3d9.lib
			# DirectShowBaseClass		Strmiids.lib
			d3dx9.lib
			# PhysXLoader.lib
			dinput8.lib
			dxerr.lib
			Dbghelp.lib
			X3daudio.lib
			wsock32.lib
			VFW32.lib
			SHLWAPI.lib
			gdiplus.lib
			
  )
	if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
		# for 64bits libs
	else()
		list(APPEND EXTRA_LIBRARIES f_in_box.lib)
		list(APPEND EXTRA_LIBRARIES FreeImage.lib)
	endif()

	# this linker flag is need when using HOARD memory allocator (libhoard). "/FORCE:MULTIPLE"
	if (USE_HOARD)
		SET (CMAKE_EXE_LINKER_FLAGS "/FORCE:MULTIPLE")
		list(APPEND EXTRA_LIBRARIES "hoard")
	endif(USE_HOARD)

else (WIN32)
    if(APPLE)
        set(EXTRA_LIBRARIES
        		dl
        		resolv
        		z
        )
    else(APPLE)
        set(EXTRA_LIBRARIES
                rt
                dl
                resolv
                z
        )
    endif(APPLE)
endif (WIN32)

# Link the executable to the libraries.
target_link_libraries (ParaEngineClient
	sqlite libcurl lua jsoncpp tinyxpath luabind assimp ParaAntlr4
	${Boost_LIBRARIES} ${EXTRA_LIBRARIES}
	#jblib pcre pcrecpp
)


if (PARAENGINE_COMPILE_LIB)
else(PARAENGINE_COMPILE_LIB)
	SET_TARGET_PROPERTIES(ParaEngineClient PROPERTIES LINK_FLAGS /MANIFESTUAC:"level='requireAdministrator' uiAccess='false'")
endif(PARAENGINE_COMPILE_LIB)

# output the executable file to root ./bin directory
if (LINUX)
# set_target_properties(ParaEngineClient PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${SERVER_SOURCE_DIR}/bin")
endif (LINUX)

include(${PROJECT_SOURCE_DIR}/PEClientInstall.cmake)
include(${PROJECT_SOURCE_DIR}/doc/Document.cmake)
