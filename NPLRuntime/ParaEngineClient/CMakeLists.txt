# Author: LiXizhi
# Email: lixizhi@yeah.net
# Date: 2016.4.30
# Desc:
project (ParaEngineClient)

set (ParaEngineClient_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../Client/trunk/ParaEngineClient)

# The version number.
set (ParaEngineClient_VERSION_MAJOR 1)
set (ParaEngineClient_VERSION_MINOR 0)

if (WIN32)
	# Note: if true, we will produce dll instead of exe, only under win32
	set(PARAENGINE_COMPILE_LIB    false)
	include(${PROJECT_SOURCE_DIR}/cmake/DirectX.cmake)
endif (WIN32)

if (DIRECTX_FOUND)
	ADD_DEFINITIONS(-DPARAENGINE_CLIENT)
	ADD_DEFINITIONS(-D_WIN32_WINNT=0x0501)

	include_directories( ${DIRECTX_INCLUDE_DIR} )
	link_directories (${DIRECTX_LIBRARY_DIR})
	set(GRAPHIC_LIBRARIES
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

else()
	find_package(OpenGL REQUIRED)
	include_directories( ${OPENGL_INCLUDE_DIRS}  )
	set(GRAPHIC_LIBRARIES ${OPENGL_LIBRARIES} )

	##############################
	file (GLOB ParaEngineClient_Engine_FILES ${ParaEngineClient_SOURCE_DIR}/Engine/ParaEngineServer.cpp)
	SOURCE_GROUP("Engine" FILES ${ParaEngineClient_Engine_FILES})
	list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Engine_FILES})
endif()


##############################
#file (GLOB ParaEngineClient_EngineHeader_FILES ${ParaEngineClient_SOURCE_DIR}/Engine/*.h)
#SOURCE_GROUP("Engine\\headers" FILES ${ParaEngineClient_EngineHeader_FILES})
#list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_EngineHeader_FILES})

##############################
file (GLOB_RECURSE ParaEngineClient_DirMonitor_FILES ${ParaEngineClient_SOURCE_DIR}/dir_monitor/*.*)
SOURCE_GROUP("Externals\\dir_monitor" FILES ${ParaEngineClient_DirMonitor_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_DirMonitor_FILES})

##############################
file (GLOB ParaEngineClient_Core_FILES ${ParaEngineClient_SOURCE_DIR}/Core/*.cpp)
SOURCE_GROUP("Core" FILES ${ParaEngineClient_Core_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Core_FILES})

file (GLOB ParaEngineClient_CoreHeader_FILES ${ParaEngineClient_SOURCE_DIR}/Core/*.h ${ParaEngineClient_SOURCE_DIR}/Core/*.hpp)
SOURCE_GROUP("Core\\Headers" FILES ${ParaEngineClient_CoreHeader_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_CoreHeader_FILES})

##############################
#file (GLOB ParaEngineClient_MDXModel_FILES ${ParaEngineClient_SOURCE_DIR}/mdxfile/*.*)
#SOURCE_GROUP("Engine\\mdxfile" FILES ${ParaEngineClient_MDXModel_FILES})
#list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_MDXModel_FILES})

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
#file (GLOB ParaEngineClient_Physics_FILES ${ParaEngineClient_SOURCE_DIR}/physics/*.*)
#SOURCE_GROUP("physics" FILES ${ParaEngineClient_Physics_FILES})
#list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Physics_FILES})


##############################
file (GLOB ParaEngineClient_IO_FILES ${ParaEngineClient_SOURCE_DIR}/IO/*.cpp ${ParaEngineClient_SOURCE_DIR}/IO/*.h)
SOURCE_GROUP("IO" FILES ${ParaEngineClient_IO_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_IO_FILES})

if(APPLE)
	file (GLOB ParaEngineClient_Platform_FILES ${ParaEngineClient_SOURCE_DIR}/platform/mac/*.cpp)
	SOURCE_GROUP("Platform" FILES ${ParaEngineClient_Platform_FILES})
	list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Platform_FILES})


	set (Ex_Dir ${PROJECT_SOURCE_DIR}/../../Client/trunk/externals)


	include_directories("${Ex_Dir}/png/include/mac")
	include_directories("${Ex_Dir}/jpeg/include/mac")
	include_directories("${Ex_Dir}/freetype2/include/mac")

endif(APPLE)

if(DIRECTX_FOUND)
	#####################################
	##--------- resource file ---------##
	# winmain.rc is the only buildable file, but the rest are all dependencies of it.
	file (GLOB ParaEngineClient_RESOURCE_FILES ${ParaEngineClient_SOURCE_DIR}/res/*.*)
	set_source_files_properties(${ParaEngineClient_RESOURCE_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
	set(ParaEngineClient_RESOURCE_FILES     ${ParaEngineClient_SOURCE_DIR}/ParaEngineRes.rc 	${ParaEngineClient_RESOURCE_FILES}	)
	SOURCE_GROUP("Externals\\res" FILES ${ParaEngineClient_RESOURCE_FILES})
	list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_RESOURCE_FILES})

	##############################
	#file (GLOB ParaEngineClient_CommonFramework_FILES ${ParaEngineClient_SOURCE_DIR}/CommonFramework/*.*)
	#SOURCE_GROUP("util\\CommonFramework" FILES ${ParaEngineClient_CommonFramework_FILES})
	#list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_CommonFramework_FILES})

	##############################
	file (GLOB ParaEngineClient_Common_FILES ${ParaEngineClient_SOURCE_DIR}/common/*.*)
	SOURCE_GROUP("Externals\\CommonDirectX" FILES ${ParaEngineClient_Common_FILES})
	list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Common_FILES})

	###################################
	##------------shader-------------##
	# Add shader files to the project.
	file (GLOB ParaEngineClient_SHADER_FILES ${ParaEngineClient_SOURCE_DIR}/shaders/*.fx)

	# custom build rules: effect FX files
	foreach( src_file ${ParaEngineClient_SHADER_FILES} )
		get_filename_component(src_filename ${src_file} NAME_WE)
		set(ParaEngineClient_SHADER_BIN_FILES ${ParaEngineClient_SHADER_BIN_FILES} "${ParaEngineClient_SOURCE_DIR}/shaders/${src_filename}.fxo")
		add_custom_command(
			OUTPUT "${ParaEngineClient_SOURCE_DIR}/shaders/${src_filename}.fxo"
			COMMAND "${DIRECTX_INCLUDE_DIR}/../Utilities/bin/x86/fxc" /Tfx_2_0 /Gfp /nologo /Fo "${ParaEngineClient_SOURCE_DIR}/shaders/${src_filename}.fxo" ${src_file}
			# uncomment the following line if you want to see the text output to debug shader instructions.
			# COMMAND fxc /Tfx_2_0 /Gfp /nologo /Fc "${ParaEngineClient_SOURCE_DIR}/shaders/${src_filename}.txt" ${src_file}
			DEPENDS ${src_file}
			COMMENT "fxc compile: ${src_file}"
			)
	endforeach( src_file ${ParaEngineClient_SHADER_FILES} )
	set_source_files_properties(${ParaEngineClient_SHADER_BIN_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
	list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_SHADER_BIN_FILES})
	SOURCE_GROUP("Shader Files\\Compiled Files" FILES ${ParaEngineClient_SHADER_BIN_FILES})

	# header files
	file (GLOB ParaEngineClient_SHADER_HEADER_FILES ${ParaEngineClient_SOURCE_DIR}/shaders/*.h)
	set_source_files_properties(${ParaEngineClient_SHADER_HEADER_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
	SOURCE_GROUP("Shader Files" FILES ${ParaEngineClient_SHADER_FILES})
	SOURCE_GROUP("Shader Files\\Headers Files" FILES ${ParaEngineClient_SHADER_HEADER_FILES})

	list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_SHADER_FILES} ${ParaEngineClient_SHADER_HEADER_FILES})

endif(DIRECTX_FOUND)

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
# add the binary tree to the search path for include files
include_directories("${ParaEngineClient_SOURCE_DIR}")
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
include_directories("${sqlite_SOURCE_DIR}")
include_directories("${Boost_INCLUDE_DIRS}")
include_directories("${luabind_SOURCE_DIR}")
include_directories("${tinyxpath_SOURCE_DIR}")
include_directories("${lua_SOURCE_DIR}/src/")
include_directories("${jsoncpp_SOURCE_DIR}/include")
IF(WIN32)
	include_directories("${ZLIB_SOURCE_DIR}")
	include_directories("${CURL_SOURCE_DIR}/include/")
	include_directories("${CURL_BINARY_DIR}/include/curl/")
ELSE(WIN32)
	INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
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
ADD_DEFINITIONS(-DTIXML_USE_STL)
ADD_DEFINITIONS(-DPE_CORE_EXPORTING)
ADD_DEFINITIONS(-DNPLRUNTIME)

INCLUDE (${CMAKE_ROOT}/Modules/CheckIncludeFile.cmake)
CHECK_INCLUDE_FILE(
  "unistd.h" HAVE_UNISTD_H)
IF(HAVE_UNISTD_H)
  ADD_DEFINITIONS(-DHAVE_UNISTD_H)
ENDIF(HAVE_UNISTD_H)

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


if (PARAENGINE_COMPILE_LIB)
	SET(CMAKE_DEBUG_POSTFIX "_d")
	add_library(ParaEngineClient SHARED ${ParaEngineClient_HEADERS} ${ParaEngineClient_SRCS})

else (PARAENGINE_COMPILE_LIB)
	# add the executable
	add_executable(ParaEngineClient ${ParaEngineClient_HEADERS} ${ParaEngineClient_SRCS})
	if(DIRECTX_FOUND)
		# When this property is set to true the executable when linked on Windows will be created with a WinMain() entry point instead of of just main().
		set_target_properties(ParaEngineClient PROPERTIES WIN32_EXECUTABLE true)
	endif(DIRECTX_FOUND)
if (WIN32)
	set_target_properties(ParaEngineClient PROPERTIES OUTPUT_NAME "ParaEngineClient")
	set_target_properties(ParaEngineClient PROPERTIES DEBUG_OUTPUT_NAME "ParaEngineClient_d")
endif()

endif (PARAENGINE_COMPILE_LIB)


if (WIN32)
  set(EXTRA_LIBRARIES
			Ws2_32
			winmm
			wsock32
			Dnsapi
			Secur32
			Crypt32
			Wldap32
			zlib
			libcurl
  )
else (WIN32)
	if(APPLE)
		set(EXTRA_LIBRARIES
				dl
				resolv
				z
			${CURL_LIBRARIES}
			pthread
		)
	else(APPLE)
		set(EXTRA_LIBRARIES
				rt
				dl
				resolv
				z
			${CURL_LIBRARIES}
			pthread
		)
	endif(APPLE)
endif (WIN32)

# Link the executable to the libraries.
target_link_libraries (ParaEngineClient
	sqlite lua jsoncpp tinyxpath luabind
	${GRAPHIC_LIBRARIES}
	${EXTRA_LIBRARIES}
	${Boost_LIBRARIES}
)

ADD_CUSTOM_COMMAND(
   TARGET ParaEngineClient
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ParaEngineClient> ${OUTPUT_BIN_DIR}
)
