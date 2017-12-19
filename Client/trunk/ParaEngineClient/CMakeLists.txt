

include(${PROJECT_SOURCE_DIR}/cmake/FindBoost.cmake)


include_directories(embed-resource)
add_subdirectory(embed-resource)

set (ParaEngineClient_SOURCE_DIR ${PROJECT_SOURCE_DIR}/ParaEngineClient/)

# The version number.
set (ParaEngineClient_VERSION_MAJOR 1)
set (ParaEngineClient_VERSION_MINOR 0)

# Sources
set(ParaEngineClient_SRCS)
# Libraries 
set(ParaEngineClient_LIBS)



set (External_Dir ${ParaEngineClient_SOURCE_DIR}/../externals)

# All Source
set(ParaEngineClient_SRCS) 



# SOURCE
# ==================================================================================
##############################
file (GLOB_RECURSE ParaEngineClient_Interface_FILES ${ParaEngineClient_SOURCE_DIR}/Framework/Interface/*.*)
SOURCE_GROUP("Framework\\Interface" FILES ${ParaEngineClient_Interface_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Interface_FILES})

##############################
file (GLOB_RECURSE ParaEngineClient_DirMonitor_FILES ${ParaEngineClient_SOURCE_DIR}/dir_monitor/*.*)
SOURCE_GROUP("Externals\\dir_monitor" FILES ${ParaEngineClient_DirMonitor_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_DirMonitor_FILES})

##############################
file (GLOB ParaEngineClient_Core_FILES ${ParaEngineClient_SOURCE_DIR}/Core/*.cpp)
SOURCE_GROUP("Core" FILES ${ParaEngineClient_Core_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Core_FILES})

#############################
file (GLOB ParaEngineClient_CoreHeader_FILES ${ParaEngineClient_SOURCE_DIR}/Core/*.h ${ParaEngineClient_SOURCE_DIR}/Core/*.hpp)
SOURCE_GROUP("Core\\Headers" FILES ${ParaEngineClient_CoreHeader_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_CoreHeader_FILES})


##############################
file (GLOB ParaEngineClient_BMaxModel_FILES ${ParaEngineClient_SOURCE_DIR}/BMaxModel/*.h ${ParaEngineClient_SOURCE_DIR}/BMaxModel/*.cpp)
SOURCE_GROUP("Engine\\BMaxModel" FILES ${ParaEngineClient_BMaxModel_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_BMaxModel_FILES})

##############################
file (GLOB ParaEngineClient_renderer_FILES ${ParaEngineClient_SOURCE_DIR}/renderer/*.h ${ParaEngineClient_SOURCE_DIR}/renderer/*.cpp)
SOURCE_GROUP("renderer" FILES ${ParaEngineClient_renderer_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_renderer_FILES})

##############################
file (GLOB ParaEngineClient_pb_FILES ${ParaEngineClient_SOURCE_DIR}/protocol/*.h ${ParaEngineClient_SOURCE_DIR}/protocol/*.cpp)
SOURCE_GROUP("protocol" FILES ${ParaEngineClient_pb_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_pb_FILES})

##############################
file (GLOB ParaEngineClient_ParaXModel_FILES ${ParaEngineClient_SOURCE_DIR}/ParaXModel/*.cpp ${ParaEngineClient_SOURCE_DIR}/ParaXModel/*.h ${ParaEngineClient_SOURCE_DIR}/ParaXModel/*.templates ${ParaEngineClient_SOURCE_DIR}/ParaXModel/*.inl)
SOURCE_GROUP("Engine\\ParaXModel" FILES ${ParaEngineClient_ParaXModel_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_ParaXModel_FILES})


##############################
file (GLOB ParaEngineClient_IO_FILES ${ParaEngineClient_SOURCE_DIR}/IO/*.cpp ${ParaEngineClient_SOURCE_DIR}/IO/*.h)
SOURCE_GROUP("IO" FILES ${ParaEngineClient_IO_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_IO_FILES})


##############################
file (GLOB ParaEngineClient_CurlLua_FILES ${ParaEngineClient_SOURCE_DIR}/curllua/*.*)
SOURCE_GROUP("Externals\\curllua" FILES ${ParaEngineClient_CurlLua_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_CurlLua_FILES})

##############################
file (GLOB ParaEngineClient_DebugTools_FILES ${ParaEngineClient_SOURCE_DIR}/debugtools/*.*)
SOURCE_GROUP("Externals\\debugtools" FILES ${ParaEngineClient_DebugTools_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_DebugTools_FILES})

##############################
file (GLOB ParaEngineClient_InfoCenter_FILES ${ParaEngineClient_SOURCE_DIR}/ic/*.h ${ParaEngineClient_SOURCE_DIR}/ic/*.cpp)
SOURCE_GROUP("Externals\\InfoCenter" FILES ${ParaEngineClient_InfoCenter_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_InfoCenter_FILES})


##############################
file (GLOB ParaEngineClient_TERRAIN_FILES ${ParaEngineClient_SOURCE_DIR}/terrain/*.h ${ParaEngineClient_SOURCE_DIR}/terrain/*.cpp)
SOURCE_GROUP("terrain" FILES ${ParaEngineClient_TERRAIN_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_TERRAIN_FILES})

##############################
file (GLOB ParaEngineClient_Block_FILES ${ParaEngineClient_SOURCE_DIR}/BlockEngine/*.cpp ${ParaEngineClient_SOURCE_DIR}/BlockEngine/*.h)
SOURCE_GROUP("BlockEngine" FILES ${ParaEngineClient_Block_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Block_FILES})

##############################
file (GLOB ParaEngineClient_Math_FILES ${ParaEngineClient_SOURCE_DIR}/math/*.cpp ${ParaEngineClient_SOURCE_DIR}/math/*.h ${ParaEngineClient_SOURCE_DIR}/math/*.inl)
SOURCE_GROUP("math" FILES ${ParaEngineClient_Math_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Math_FILES})

##############################
file (GLOB ParaEngineClient_NPL_FILES ${ParaEngineClient_SOURCE_DIR}/NPL/*.h ${ParaEngineClient_SOURCE_DIR}/NPL/*.cpp ${ParaEngineClient_SOURCE_DIR}/NPL/*.hpp ${ParaEngineClient_SOURCE_DIR}/NPL/*.txt ${ParaEngineClient_SOURCE_DIR}/NPL/*.xml)
SOURCE_GROUP("NPL" FILES ${ParaEngineClient_NPL_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_NPL_FILES})

##############################
file (GLOB ParaEngineClient_Scripting_FILES ${ParaEngineClient_SOURCE_DIR}/ParaScriptBindings/*.cpp ${ParaEngineClient_SOURCE_DIR}/ParaScriptBindings/*.h)
SOURCE_GROUP("ParaScriptBindings" FILES ${ParaEngineClient_Scripting_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Scripting_FILES})

##############################
file (GLOB ParaEngineClient_GUI_FILES ${ParaEngineClient_SOURCE_DIR}/2dengine/*.cpp ${ParaEngineClient_SOURCE_DIR}/2dengine/*.h)
SOURCE_GROUP("2dengine" FILES ${ParaEngineClient_GUI_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_GUI_FILES})

##############################
file (GLOB ParaEngineClient_painting_FILES ${ParaEngineClient_SOURCE_DIR}/PaintEngine/*.cpp ${ParaEngineClient_SOURCE_DIR}/PaintEngine/*.h)
SOURCE_GROUP("PaintEngine" FILES ${ParaEngineClient_painting_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_painting_FILES})

##############################
file (GLOB ParaEngineClient_3D_FILES ${ParaEngineClient_SOURCE_DIR}/3dengine/*.cpp ${ParaEngineClient_SOURCE_DIR}/3dengine/*.h ${ParaEngineClient_SOURCE_DIR}/3dengine/*.cpp)
SOURCE_GROUP("3dengine" FILES ${ParaEngineClient_3D_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_3D_FILES})

##############################
# util files
file (GLOB ParaEngineClient_UTIL_FILES ${ParaEngineClient_SOURCE_DIR}/util/*.h ${ParaEngineClient_SOURCE_DIR}/util/*.cpp ${ParaEngineClient_SOURCE_DIR}/util/*.hpp ${ParaEngineClient_SOURCE_DIR}/util/*.c)
SOURCE_GROUP("util" FILES ${ParaEngineClient_UTIL_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_UTIL_FILES})

##############################
# Documentation
file (GLOB ParaEngineClient_DOCUMENT_FILES ${ParaEngineClient_SOURCE_DIR}/doc/*.*)
SOURCE_GROUP("doc" FILES ${ParaEngineClient_DOCUMENT_FILES})
set_source_files_properties(${ParaEngineClient_DOCUMENT_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_DOCUMENT_FILES})

##############################
# WebSocket
file (GLOB ParaEngineClient_WebSocket_FILES ${ParaEngineClient_SOURCE_DIR}/WebSocket/*.cpp ${ParaEngineClient_SOURCE_DIR}/WebSocket/*.h)
SOURCE_GROUP("WebSocket" FILES ${ParaEngineClient_WebSocket_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_WebSocket_FILES})

##############################
file (GLOB ParaEngineClient_Engine_FILES ${ParaEngineClient_SOURCE_DIR}/Engine/*.cpp)
SOURCE_GROUP("Engine" FILES ${ParaEngineClient_Engine_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Engine_FILES})
##############################
file (GLOB ParaEngineClient_EngineHeader_FILES ${ParaEngineClient_SOURCE_DIR}/Engine/*.h)
SOURCE_GROUP("Engine\\headers" FILES ${ParaEngineClient_EngineHeader_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_EngineHeader_FILES})
##############################
file (GLOB ParaEngineClient_VoxelMesh_FILES ${ParaEngineClient_SOURCE_DIR}/VoxelMesh/*.*)
SOURCE_GROUP("Engine\\VoxelMesh" FILES ${ParaEngineClient_VoxelMesh_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_VoxelMesh_FILES})
##############################
file (GLOB ParaEngineClient_Cad_FILES ${ParaEngineClient_SOURCE_DIR}/CadModel/*.*)
SOURCE_GROUP("Externals\\CadModel" FILES ${ParaEngineClient_Cad_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Cad_FILES})
#####################################



file (GLOB ParaEngineClient_Platfowm_Windows_FILES ${ParaEngineClient_SOURCE_DIR}/Platform/Windows/*.*)
SOURCE_GROUP("Platform\\Windows" FILES ${ParaEngineClient_Platfowm_Windows_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Platfowm_Windows_FILES})
#####################################

file (GLOB ParaEngineClient_D3D9Render_FILES ${ParaEngineClient_SOURCE_DIR}/Platform/Windows/Render/D3D9/*.*)
SOURCE_GROUP("Platform\\Windows\\Render\\D3D9" FILES ${ParaEngineClient_D3D9Render_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_D3D9Render_FILES})
#####################################



# Resource file
# winmain.rc is the only buildable file, but the rest are all dependencies of it.
file (GLOB ParaEngineClient_RESOURCE_FILES ${ParaEngineClient_SOURCE_DIR}/res/*.*)
set_source_files_properties(${ParaEngineClient_RESOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
set(ParaEngineClient_RESOURCE_FILES     ${ParaEngineClient_SOURCE_DIR}/ParaEngineRes.rc 	${ParaEngineClient_RESOURCE_FILES}	)
SOURCE_GROUP("Externals\\res" FILES ${ParaEngineClient_RESOURCE_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_RESOURCE_FILES})
 
##############################
# CommonDirectX
file (GLOB ParaEngineClient_Common_FILES ${ParaEngineClient_SOURCE_DIR}/common/*.*)
SOURCE_GROUP("Externals\\CommonDirectX" FILES ${ParaEngineClient_Common_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Common_FILES})		

 ##############################
# Compiled Files
set_source_files_properties(${ParaEngineClient_SHADER_BIN_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_SHADER_BIN_FILES})
SOURCE_GROUP("Shader Files\\Compiled Files" FILES ${ParaEngineClient_SHADER_BIN_FILES})
##############################


##############################
# ParaXmodel.templates
embed_resources_abs(MyResource "${ParaEngineClient_SOURCE_DIR}/res/ParaXmodel.templates")
list(APPEND EmbeddedResource_FILES ${MyResource})

##############################
# cursor.tga
embed_resources_abs(MyResource "${ParaEngineClient_SOURCE_DIR}/res/cursor.tga")
list(APPEND EmbeddedResource_FILES ${MyResource})
##############################
# Shader Header Files
file (GLOB ParaEngineClient_SHADER_HEADER_FILES ${ParaEngineClient_SOURCE_DIR}/shaders/*.h)
set_source_files_properties(${ParaEngineClient_SHADER_HEADER_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)	
SOURCE_GROUP("Shader Files\\Headers Files" FILES ${ParaEngineClient_SHADER_HEADER_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_SHADER_HEADER_FILES})
##############################
# Shader
# find directx
include(${PROJECT_SOURCE_DIR}/cmake/FindDirectX.cmake)
if(NOT DIRECTX_FOUND)
	message( FATAL_ERROR "Not found DirectX SDK, CMake will exit.")
endif()
message(STATUS "DirectX SDK ROOT: "${DirectX_INCLUDE_DIRS} )

file (GLOB ParaEngineClient_SHADER_FILES ${ParaEngineClient_SOURCE_DIR}/shaders/*.fx)	
# custom build rules: effect FX files
foreach( src_file ${ParaEngineClient_SHADER_FILES} )
	get_filename_component(src_filename ${src_file} NAME_WE)
 	set(out_file "${ParaEngineClient_SOURCE_DIR}/shaders/${src_filename}.fxo")
 	set(ParaEngineClient_SHADER_BIN_FILES ${ParaEngineClient_SHADER_BIN_FILES} ${out_file})
 	add_custom_command(
	 	OUTPUT ${out_file}
	 	COMMAND "${DirectX_INCLUDE_DIRS}/../Utilities/bin/x86/fxc" /Tfx_2_0 /Gfp /nologo /Fo ${out_file} ${src_file}
	 	# uncomment the following line if you want to see the text output to debug shader instructions.
	 	# COMMAND fxc /Tfx_2_0 /Gfp /nologo /Fc "${ParaEngineClient_SOURCE_DIR}/shaders/${src_filename}.txt" ${src_file}
	 	DEPENDS ${src_file}
	 	COMMENT "fxc compile: ${src_file}"
	 	)
 	embed_resources_abs(MyResource "${ParaEngineClient_SOURCE_DIR}/shaders/${src_filename}.fxo")
 	list(APPEND EmbeddedResource_FILES ${MyResource})
endforeach( src_file ${ParaEngineClient_SHADER_FILES} )



##############################
# Shader Files
SOURCE_GROUP("Shader Files" FILES ${ParaEngineClient_SHADER_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_SHADER_FILES})


##############################
# embedded resource
list(APPEND EmbeddedResource_FILES ${ParaEngineClient_SOURCE_DIR}/ResourceEmbedded.cpp)
list(APPEND ParaEngineClient_SRCS ${EmbeddedResource_FILES})
SOURCE_GROUP("ResEmbedded" FILES ${EmbeddedResource_FILES})


# END OF SOURCE
# =====================================================================================


# INCLUDE DIRECTORIES
# ==================================================================================

##############################
# add the binary tree to the search path for include files
include_directories("${ParaEngineClient_SOURCE_DIR}")
include_directories("${ParaEngineClient_SOURCE_DIR}/Framework/Interface")
include_directories("${ParaEngineClient_SOURCE_DIR}/Core")
include_directories("${ParaEngineClient_SOURCE_DIR}/renderer")
include_directories("${ParaEngineClient_SOURCE_DIR}/3dengine")
include_directories("${ParaEngineClient_SOURCE_DIR}/Engine")
include_directories("${ParaEngineClient_SOURCE_DIR}/IO")
include_directories("${ParaEngineClient_SOURCE_DIR}/util")
include_directories("${ParaEngineClient_SOURCE_DIR}/math")
include_directories("${ParaEngineClient_SOURCE_DIR}/ParaScriptBindings")
include_directories("${ParaEngineClient_SOURCE_DIR}/NPL")
include_directories("${ParaEngineClient_SOURCE_DIR}/2dengine/Video")
include_directories("${ParaEngineClient_SOURCE_DIR}/common")
include_directories("${ParaEngineClient_SOURCE_DIR}/Platform/Windows/Render/D3D9")
include_directories("${ParaEngineClient_SOURCE_DIR}/Platform/Windows/")
include_directories("${Boost_INCLUDE_DIRS}")



if(NPLRUNTIME_SUPPORT_FBX)
	include_directories("${Assimp_SOURCE_DIR}/include/")
	include_directories("${Assimp_BINARY_DIR}/include")
	ADD_DEFINITIONS(-DSUPPORT_FBX_MODEL_FILE)
endif()


# END OF INCLUDE DIRECTORIES
# ==================================================================================


# LIBRARIES
# ==================================================================================

# All libraries
set(ParaEngineClient_LIBS)
# All link directories
set(ParaEngineClient_LINK_DIRS)



##############################
# Direct3D
include_directories(${DirectX_INCLUDE_DIRS})
list(APPEND ParaEngineClient_LINK_DIRS 	${DirectX_LIBRARY_PATHS})
list(APPEND ParaEngineClient_LIBS
			comctl32.lib
			d3dxof.lib
			dxguid.lib
			d3d9.lib
			d3dx9.lib
			dinput8.lib
			dxerr.lib
			Dbghelp.lib
			X3daudio.lib
			wsock32.lib
			VFW32.lib
			SHLWAPI.lib
			gdiplus.lib
			Psapi.lib
)
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	# for 64bits libs
elseif(WIN32)
	ADD_DEFINITIONS(-DUSE_FREEIMAGE) # for reading image files
	ADD_DEFINITIONS(-DUSE_GDI_COMPOSER) # for css skin, etc
	include_directories("${External_Dir}/FreeImage_3120/Dist")
	link_directories("${External_Dir}/FreeImage_3120/Dist")
	list(APPEND ParaEngineClient_LIBS FreeImage.lib)
	configure_file("${External_Dir}/FreeImage_3120/Dist/FreeImage.dll" "${OUTPUT_BIN_DIR}/FreeImage.dll" COPYONLY)
endif()



##############################
# BOOST
list(APPEND ParaEngineClient_LIBS ${Boost_LIBRARIES})



##############################
# assimp
if (NPLRUNTIME_SUPPORT_FBX)
list(APPEND ParaEngineClient_LIBS assimp)
endif()


##############################
# EXTRA_LIBRARIES
if(WIN32)
	list(APPEND ParaEngineClient_LIBS Ws2_32 winmm wsock32 Dnsapi Secur32 Crypt32 Wldap32 zlib libcurl
	 liblua jsoncpp tinyxpath luabind sqlite)
endif()
	

# END OF LIBRARIES
# ==================================================================================


# COMPILER SETTTINGS


ADD_DEFINITIONS(-D_WIN32_WINNT=0x0501)
ADD_DEFINITIONS(-DPARAENGINE_CLIENT)
ADD_DEFINITIONS(-DUSE_DIRECTX_RENDERER)


	#######################################
	# PCH: precompiled header under msvc
	if (MSVC)
    set_source_files_properties(${ParaEngineClient_SOURCE_DIR}/Core/ParaEngine.cxx
        PROPERTIES
        COMPILE_FLAGS "/YcParaEngine.h"
        )
    foreach( src_file ${ParaEngineClient_SRCS} )
        if(${src_file} MATCHES "cpp")
            set_source_files_properties(
                ${src_file}
                PROPERTIES
                COMPILE_FLAGS "/YuParaEngine.h"
                )
        endif(${src_file} MATCHES "cpp")
    endforeach( src_file ${ParaEngineClient_SRCS} )

    list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_SOURCE_DIR}/Core/ParaEngine.cxx)
endif (MSVC)

if(MSVC)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
endif()




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
    # SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GR-")
    
    # /Zi generate program database (PDB symbol files even in release build)
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
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
ELSE()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

    # remove Run-Time-Type-Information (RTTI) to miminize the executable size
    # set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-rtti")

    # -Os to optimize for size. or -O3 to turn on all optimizations.
    # -s option to strip debug info, -g to include debug info
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -s")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -s")

    # suppress warnings
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-deprecated-declarations")
ENDIF()



ADD_DEFINITIONS(-DBOOST_SIGNALS_NO_DEPRECATION_WARNING)
ADD_DEFINITIONS(-DTIXML_USE_STL)
if(NPLRUNTIME_STATIC_LIB)
	ADD_DEFINITIONS(-DNPLRUNTIME_STATICLIB)
	ADD_DEFINITIONS(-DSQLITE_STATICLIB)
	ADD_DEFINITIONS(-DCURL_STATICLIB)
	ADD_DEFINITIONS(-DPE_CORE_EXPORTING)
else()
	ADD_DEFINITIONS(-DPE_CORE_EXPORTING)
endif()
ADD_DEFINITIONS(-DNPLRUNTIME)

INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
CHECK_INCLUDE_FILE(
"unistd.h" HAVE_UNISTD_H)
IF(HAVE_UNISTD_H)
ADD_DEFINITIONS(-DHAVE_UNISTD_H)
ENDIF(HAVE_UNISTD_H)


# END OF COMPILER SETTTINGS
# ==================================================================================




# TARGET

link_directories(${ParaEngineClient_LINK_DIRS})
link_directories(${NPLRUNTIME_LINK_DIRECTORIES})

#########################
# Platform/ Win32
if(WIN32)

	ADD_DEFINITIONS(-DPLATFORM_WINDOWS)
	ADD_DEFINITIONS(-D_WIN32_WINNT=0x0501)
    ADD_DEFINITIONS(-DUSE_WINDOWS_SSPI)
    ADD_DEFINITIONS(-DSQLITE_STATICLIB)

	# Note: if true, we will produce dll instead of exe, only under win32
	if(PARAENGINE_CLIENT_DLL)
		set(PARAENGINE_COMPILE_LIB    true)
		ADD_DEFINITIONS(-DNPLRUNTIME_DLL)
	else()
		set(PARAENGINE_COMPILE_LIB    false)
	endif()

	if (PARAENGINE_COMPILE_LIB)
		set(CMAKE_DEBUG_POSTFIX "_d")
		if(NPLRUNTIME_STATIC_LIB)
			set(NPLRUNTIME_LIB_MODE "STATIC")
		else()
			set(NPLRUNTIME_LIB_MODE "SHARED")
		endif()
		add_library(ParaEngineClient ${NPLRUNTIME_LIB_MODE} ${ParaEngineClient_HEADERS} ${ParaEngineClient_SRCS})
	else ()
		# add the executable
		add_executable(ParaEngineClient ${ParaEngineClient_HEADERS} ${ParaEngineClient_SRCS})
		# When this property is set to true the executable when linked on Windows will be created with a WinMain() entry point instead of of just main().
		set_target_properties(ParaEngineClient PROPERTIES WIN32_EXECUTABLE true)
		set_target_properties(ParaEngineClient PROPERTIES OUTPUT_NAME "ParaEngineClient")
		set_target_properties(ParaEngineClient PROPERTIES DEBUG_OUTPUT_NAME "ParaEngineClient_d")

	endif ()

endif()




target_link_libraries(ParaEngineClient ${ParaEngineClient_LIBS})

#ADD_CUSTOM_COMMAND(
#	TARGET ParaEngineClient
#	POST_BUILD
#	COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
#	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ParaEngineClient> ${OUTPUT_BIN_DIR}
 #)



# END OF TARGET
# ==================================================================================

