# Author: LiXizhi
# Email: lixizhi@yeah.net
# Date: 2009.4.10, 2016.2.21 shared source with client
# Desc: option modify PARAENGINE_COMPILE_LIB
project (ParaEngineServer)

include_directories(../embed-resource)
add_subdirectory(../embed-resource ${PROJECT_BINARY_DIR}/../embed-resource)

set (CLIENT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../Client/trunk/ParaEngineClient)

# The version number.
set (ParaEngineServer_VERSION_MAJOR 1)
set (ParaEngineServer_VERSION_MINOR 0)

set(EXTRA_LIBRARIES )
if (WIN32)
	# Note: if true, we will produce dll instead of exe, only under win32
	set(PARAENGINE_COMPILE_LIB    false)
endif (WIN32)

find_package(OpenGL REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  )

##############################
file (GLOB ParaEngineServer_Engine_FILES ${CLIENT_SOURCE_DIR}/Engine/ParaEngineServer.cpp ${CLIENT_SOURCE_DIR}/Engine/ParaXStaticBase.cpp)
SOURCE_GROUP("Engine" FILES ${ParaEngineServer_Engine_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_Engine_FILES})

##############################
#file (GLOB ParaEngineServer_EngineHeader_FILES ${CLIENT_SOURCE_DIR}/Engine/*.h)
#SOURCE_GROUP("Engine\\headers" FILES ${ParaEngineServer_EngineHeader_FILES})
#list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_EngineHeader_FILES})

##############################
file (GLOB_RECURSE ParaEngineServer_DirMonitor_FILES ${CLIENT_SOURCE_DIR}/dir_monitor/*.*)
SOURCE_GROUP("Externals\\dir_monitor" FILES ${ParaEngineServer_DirMonitor_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_DirMonitor_FILES})

##############################
file (GLOB ParaEngineServer_Core_FILES ${CLIENT_SOURCE_DIR}/Core/*.cpp)
SOURCE_GROUP("Core" FILES ${ParaEngineServer_Core_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_Core_FILES})

file (GLOB ParaEngineServer_CoreHeader_FILES ${CLIENT_SOURCE_DIR}/Core/*.h ${CLIENT_SOURCE_DIR}/Core/*.hpp)
SOURCE_GROUP("Core\\Headers" FILES ${ParaEngineServer_CoreHeader_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_CoreHeader_FILES})

##############################
#file (GLOB ParaEngineServer_MDXModel_FILES ${CLIENT_SOURCE_DIR}/mdxfile/*.*)
#SOURCE_GROUP("Engine\\mdxfile" FILES ${ParaEngineServer_MDXModel_FILES})
#list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_MDXModel_FILES})

##############################
file (GLOB ParaEngineServer_BMaxModel_FILES ${CLIENT_SOURCE_DIR}/BMaxModel/*.h ${CLIENT_SOURCE_DIR}/BMaxModel/*.cpp)
SOURCE_GROUP("Engine\\BMaxModel" FILES ${ParaEngineServer_BMaxModel_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_BMaxModel_FILES})

##############################
file (GLOB ParaEngineServer_renderer_FILES ${CLIENT_SOURCE_DIR}/renderer/*.h ${CLIENT_SOURCE_DIR}/renderer/*.cpp)
SOURCE_GROUP("renderer" FILES ${ParaEngineServer_renderer_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_renderer_FILES})

##############################
file (GLOB ParaEngineServer_pb_FILES ${CLIENT_SOURCE_DIR}/protocol/*.h ${CLIENT_SOURCE_DIR}/protocol/*.cpp)
SOURCE_GROUP("protocol" FILES ${ParaEngineServer_pb_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_pb_FILES}) 

##############################
file (GLOB ParaEngineServer_ParaXModel_FILES ${CLIENT_SOURCE_DIR}/ParaXModel/*.cpp ${CLIENT_SOURCE_DIR}/ParaXModel/*.h ${CLIENT_SOURCE_DIR}/ParaXModel/*.templates ${CLIENT_SOURCE_DIR}/ParaXModel/*.inl)
SOURCE_GROUP("Engine\\ParaXModel" FILES ${ParaEngineServer_ParaXModel_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_ParaXModel_FILES})

##############################
#file (GLOB ParaEngineServer_Physics_FILES ${CLIENT_SOURCE_DIR}/physics/*.*)
#SOURCE_GROUP("physics" FILES ${ParaEngineServer_Physics_FILES})
#list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_Physics_FILES})


##############################
file (GLOB ParaEngineServer_IO_FILES ${CLIENT_SOURCE_DIR}/IO/*.cpp ${CLIENT_SOURCE_DIR}/IO/*.h)
SOURCE_GROUP("IO" FILES ${ParaEngineServer_IO_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_IO_FILES})

##############################
#file (GLOB ParaEngineServer_CommonFramework_FILES ${CLIENT_SOURCE_DIR}/CommonFramework/*.*)
#SOURCE_GROUP("util\\CommonFramework" FILES ${ParaEngineServer_CommonFramework_FILES})
#list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_CommonFramework_FILES})

##############################
#file (GLOB ParaEngineServer_Common_FILES ${CLIENT_SOURCE_DIR}/common/*.*)
#SOURCE_GROUP("Externals\\CommonDirectX" FILES ${ParaEngineServer_Common_FILES})
#list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_Common_FILES})

set(EmbeddedResource_FILES)
# embedded resource
embed_resources_abs(MyResource "${CLIENT_SOURCE_DIR}/res/ParaXmodel.templates")
list(APPEND EmbeddedResource_FILES ${MyResource})

list(APPEND EmbeddedResource_FILES ${CLIENT_SOURCE_DIR}/ResourceEmbedded.cpp)
list(APPEND ParaEngineServer_SRCS ${EmbeddedResource_FILES})
SOURCE_GROUP("ResEmbedded" FILES ${EmbeddedResource_FILES})

##############################
file (GLOB ParaEngineServer_CurlLua_FILES ${CLIENT_SOURCE_DIR}/curllua/*.*)
SOURCE_GROUP("Externals\\curllua" FILES ${ParaEngineServer_CurlLua_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_CurlLua_FILES})

##############################
file (GLOB ParaEngineServer_DebugTools_FILES ${CLIENT_SOURCE_DIR}/debugtools/*.*)
SOURCE_GROUP("Externals\\debugtools" FILES ${ParaEngineServer_DebugTools_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_DebugTools_FILES})

##############################
file (GLOB ParaEngineServer_InfoCenter_FILES ${CLIENT_SOURCE_DIR}/ic/*.h ${CLIENT_SOURCE_DIR}/ic/*.cpp)
SOURCE_GROUP("Externals\\InfoCenter" FILES ${ParaEngineServer_InfoCenter_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_InfoCenter_FILES})


##############################
file (GLOB ParaEngineServer_TERRAIN_FILES ${CLIENT_SOURCE_DIR}/terrain/*.h ${CLIENT_SOURCE_DIR}/terrain/*.cpp)
SOURCE_GROUP("terrain" FILES ${ParaEngineServer_TERRAIN_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_TERRAIN_FILES})

##############################
file (GLOB ParaEngineServer_Block_FILES ${CLIENT_SOURCE_DIR}/BlockEngine/*.cpp ${CLIENT_SOURCE_DIR}/BlockEngine/*.h)
SOURCE_GROUP("BlockEngine" FILES ${ParaEngineServer_Block_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_Block_FILES})

##############################
file (GLOB ParaEngineServer_Math_FILES ${CLIENT_SOURCE_DIR}/math/*.cpp ${CLIENT_SOURCE_DIR}/math/*.h ${CLIENT_SOURCE_DIR}/math/*.inl)
SOURCE_GROUP("math" FILES ${ParaEngineServer_Math_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_Math_FILES})

##############################
file (GLOB ParaEngineServer_NPL_FILES ${CLIENT_SOURCE_DIR}/NPL/*.h ${CLIENT_SOURCE_DIR}/NPL/*.cpp ${CLIENT_SOURCE_DIR}/NPL/*.hpp ${CLIENT_SOURCE_DIR}/NPL/*.txt ${CLIENT_SOURCE_DIR}/NPL/*.xml)
SOURCE_GROUP("NPL" FILES ${ParaEngineServer_NPL_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_NPL_FILES})

##############################
file (GLOB ParaEngineServer_Scripting_FILES ${CLIENT_SOURCE_DIR}/ParaScriptBindings/*.cpp ${CLIENT_SOURCE_DIR}/ParaScriptBindings/*.h)
SOURCE_GROUP("ParaScriptBindings" FILES ${ParaEngineServer_Scripting_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_Scripting_FILES})

##############################
file (GLOB ParaEngineServer_GUI_FILES ${CLIENT_SOURCE_DIR}/2dengine/*.cpp ${CLIENT_SOURCE_DIR}/2dengine/*.h)
SOURCE_GROUP("2dengine" FILES ${ParaEngineServer_GUI_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_GUI_FILES})

##############################
file (GLOB ParaEngineServer_painting_FILES ${CLIENT_SOURCE_DIR}/PaintEngine/*.cpp ${CLIENT_SOURCE_DIR}/PaintEngine/*.h)
SOURCE_GROUP("PaintEngine" FILES ${ParaEngineServer_painting_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_painting_FILES})

##############################
file (GLOB ParaEngineServer_3D_FILES ${CLIENT_SOURCE_DIR}/3dengine/*.cpp ${CLIENT_SOURCE_DIR}/3dengine/*.h ${CLIENT_SOURCE_DIR}/3dengine/*.cpp)
SOURCE_GROUP("3dengine" FILES ${ParaEngineServer_3D_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_3D_FILES})

##############################
# util files
file (GLOB ParaEngineServer_UTIL_FILES ${CLIENT_SOURCE_DIR}/util/*.h ${CLIENT_SOURCE_DIR}/util/*.cpp ${CLIENT_SOURCE_DIR}/util/*.hpp ${CLIENT_SOURCE_DIR}/util/*.c)
SOURCE_GROUP("util" FILES ${ParaEngineServer_UTIL_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_UTIL_FILES})

##############################
# Documentation
file (GLOB ParaEngineClient_DOCUMENT_FILES ${CLIENT_SOURCE_DIR}/doc/*.*)
SOURCE_GROUP("doc" FILES ${ParaEngineClient_DOCUMENT_FILES})
set_source_files_properties(${ParaEngineClient_DOCUMENT_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)
list(APPEND ParaEngineServer_SRCS ${ParaEngineClient_DOCUMENT_FILES})

##############################
# WebSocket
file (GLOB ParaEngineServer_WebSocket_FILES ${CLIENT_SOURCE_DIR}/WebSocket/*.cpp ${CLIENT_SOURCE_DIR}/WebSocket/*.h ${CLIENT_SOURCE_DIR}/WebSocket/*.cpp)
SOURCE_GROUP("WebSocket" FILES ${ParaEngineServer_WebSocket_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineServer_WebSocket_FILES})

##############################
#AutoRigger
file (GLOB ParaEngineClient_AutoRigger_FILES ${CLIENT_SOURCE_DIR}/AutoRigger/*.cpp ${CLIENT_SOURCE_DIR}/AutoRigger/*.h)
SOURCE_GROUP("AutoRigger" FILES ${ParaEngineClient_AutoRigger_FILES})
list(APPEND ParaEngineServer_SRCS ${ParaEngineClient_AutoRigger_FILES})

#######################################
# PCH: precompiled header under msvc
if (MSVC)
	set_source_files_properties(${CLIENT_SOURCE_DIR}/Core/ParaEngine.cxx
		PROPERTIES
		COMPILE_FLAGS "/YcParaEngine.h"
		)
	foreach( src_file ${ParaEngineServer_SRCS} )
		if(${src_file} MATCHES "cpp")
			set_source_files_properties(
				${src_file}
				PROPERTIES
				COMPILE_FLAGS "/YuParaEngine.h"
				)
		endif(${src_file} MATCHES "cpp")
	endforeach( src_file ${ParaEngineServer_SRCS} )
	
	list(APPEND ParaEngineServer_SRCS ${CLIENT_SOURCE_DIR}/Core/ParaEngine.cxx)
endif (MSVC)


##############################
# add the binary tree to the search path for include files
include_directories("${CLIENT_SOURCE_DIR}")
include_directories("${CLIENT_SOURCE_DIR}/Core")
include_directories("${CLIENT_SOURCE_DIR}/renderer")
include_directories("${CLIENT_SOURCE_DIR}/3dengine")
include_directories("${CLIENT_SOURCE_DIR}/Engine")
include_directories("${CLIENT_SOURCE_DIR}/IO")
include_directories("${CLIENT_SOURCE_DIR}/util")
include_directories("${CLIENT_SOURCE_DIR}/math")
include_directories("${CLIENT_SOURCE_DIR}/ParaScriptBindings")
include_directories("${CLIENT_SOURCE_DIR}/NPL")
include_directories("${CLIENT_SOURCE_DIR}/2dengine/Video")
include_directories("${CLIENT_SOURCE_DIR}/common")
include_directories("${CLIENT_SOURCE_DIR}/AutoRigger")
include_directories("${sqlite_SOURCE_DIR}")
include_directories("${Boost_INCLUDE_DIRS}")
include_directories("${luabind_SOURCE_DIR}")
include_directories("${tinyxpath_SOURCE_DIR}")
include_directories("${lua_SOURCE_DIR}/src/")
include_directories("${jsoncpp_SOURCE_DIR}/include")

#AutoRigger
include_directories("${SERVER_SOURCE_DIR}/Eigen")
include_directories("${SERVER_SOURCE_DIR}/flann-1.8.4-src/src/cpp/")
include_directories("${SERVER_SOURCE_DIR}/Pinocchio/include")
include_directories("${SERVER_SOURCE_DIR}/pcl/common/include")
include_directories("${SERVER_SOURCE_DIR}/pcl/search/include")
include_directories("${SERVER_SOURCE_DIR}/pcl/octree/include")
include_directories("${SERVER_SOURCE_DIR}/pcl/surface/include")
include_directories("${SERVER_SOURCE_DIR}/pcl/kdtree/include")
include_directories("${SERVER_SOURCE_DIR}/pcl/features/include")

IF(WIN32)
	include_directories("${ZLIB_SOURCE_DIR}")
	include_directories("${CURL_SOURCE_DIR}/include/")
	include_directories("${CURL_BINARY_DIR}/include/curl/")
ELSE(WIN32)
	INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
ENDIF(WIN32)

## Readline support
include("${PROJECT_SOURCE_DIR}/../cmake/FindReadline.cmake")
# FIND_PACKAGE(Readline)
IF(READLINE_FOUND)
  ADD_DEFINITIONS("-DNPL_USE_READLINE")
  LIST(APPEND EXTRA_LIBRARIES ${Readline_LIBRARY})
  MESSAGE("NPL_USE_READLINE read line is found and used")
  INCLUDE_DIRECTORIES(${READLINE_INCLUDE_DIR})
ELSE()
	MESSAGE("warning: NPL_USE_READLINE NOT FOUND please apt-get install libreadline6 libreadline6-dev")
ENDIF()

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
ADD_DEFINITIONS(-DPARAENGINE_SERVER)
ADD_DEFINITIONS(-DTIXML_USE_STL)
ADD_DEFINITIONS(-DPE_CORE_EXPORTING)

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

ENDIF()

# NOTE: This generate smaller and faster exe/dll, but no other libs should to it statically.
option(USE_LINK_TIME_CODE_GENERATION	"true to enable link time code gen in release build" OFF)

IF(MSVC)
	if(USE_LINK_TIME_CODE_GENERATION)
		# /GL Link time code generation. Note turn this off if you have plugins that links to ParaEngineServer.
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
		SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /LARGEADDRESSAWARE /PDBSTRIPPED:Release/ParaEngineServer.stripped.pdb")
	else (PARAENGINE_COMPILE_LIB)
		# To turn size optimisations on again
		SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /opt:ref /DEBUG /LARGEADDRESSAWARE")
		# add a stripped version of symbol files during release build
		SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LARGEADDRESSAWARE /PDBSTRIPPED:Release/ParaEngineServer.stripped.pdb")
	endif (PARAENGINE_COMPILE_LIB)
ELSE()

	# set debug
	# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11")


	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

	# remove Run-Time-Type-Information (RTTI) to miminize the executable size
	# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-rtti")

	IF(APPLE)
		# -Os to optimize for size. or -O3 to turn on all optimizations.
		# no -s option since it has the same effect as gcc -s if not a single -g is provided
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
	ELSE()
		# -Os to optimize for size. or -O3 to turn on all optimizations.
		# -s option to strip debug info, -g to include debug info
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -s")
		set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -s")
	ENDIF()

	# suppress warnings
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-deprecated-declarations")
ENDIF()


if (PARAENGINE_COMPILE_LIB)
	SET(CMAKE_DEBUG_POSTFIX "_d")
	add_library(ParaEngineServer SHARED ${ParaEngineServer_HEADERS} ${ParaEngineServer_SRCS})

else (PARAENGINE_COMPILE_LIB)
	# add the executable
	add_executable(ParaEngineServer ${ParaEngineServer_HEADERS} ${ParaEngineServer_SRCS})
	# When this property is set to true the executable when linked on Windows will be created with a WinMain() entry point instead of of just main().
	# set_target_properties(ParaEngineServer PROPERTIES WIN32_EXECUTABLE true)
if (WIN32)
	set_target_properties(ParaEngineServer PROPERTIES OUTPUT_NAME "ParaEngineServer")
	set_target_properties(ParaEngineServer PROPERTIES DEBUG_OUTPUT_NAME "ParaEngineServer_d")
endif()

endif (PARAENGINE_COMPILE_LIB)


if(APPLE)
	# for luajit 2.0 & mac
    # SET(CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000 ${CMAKE_EXE_LINKER_FLAGS}")
endif()

if(APPLE)
    set(LUA_LIBRARY luajit21)
else()
    set(LUA_LIBRARY lua)
endif()

##############################
# iconv
option(NPLRUNTIME_ICONV "include iconv" ON)

if(NPLRUNTIME_ICONV)
	ADD_DEFINITIONS(-DUSE_ICONV)


	if(APPLE)
		list(APPEND EXTRA_LIBRARIES iconv)
	#elseif (LINUX OR UNIX)
	#	list(APPEND EXTRA_LIBRARIES libiconv)
	else()
		add_subdirectory(${SERVER_SOURCE_DIR}libiconv ${NPLRuntime_BINARY_DIR}/libiconv)
		set(ICONV_INCLUDE_DIR ${SERVER_SOURCE_DIR}libiconv/include ${NPLRuntime_BINARY_DIR}/libiconv/include)
		set(ICONV_SOURCE_DIR ${SERVER_SOURCE_DIR}libiconv/src ${NPLRuntime_BINARY_DIR}/libiconv/src)
		include_directories("${ICONV_INCLUDE_DIR}")
		list(APPEND EXTRA_LIBRARIES libiconv)
	endif()
endif()

if (WIN32)
  set(EXTRA_LIBRARIES ${EXTRA_LIBRARIES}
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
	if(LINUX)
		set(EXTRA_LIBRARIES ${EXTRA_LIBRARIES}
				rt
				dl
				resolv
				z
			${CURL_LIBRARIES}
			pthread
		)
	else(LINUX)
		set(EXTRA_LIBRARIES ${EXTRA_LIBRARIES}
				dl
				resolv
				z
			${CURL_LIBRARIES}
			pthread
		)
	endif(LINUX)
	if(APPLE)
	    ## this is for file monitor API in MAC OSX
		set(EXTRA_LIBRARIES ${EXTRA_LIBRARIES} "-framework CoreFoundation" "-framework CoreServices")
	endif(APPLE)
endif (WIN32)


# Link the executable to the libraries.
target_link_libraries (ParaEngineServer
    ${Boost_LIBRARIES} ${LUA_LIBRARY}
	sqlite jsoncpp tinyxpath luabind
	pcl_common pcl_features Pinocchio
	${OPENGL_LIBRARIES}
	${EXTRA_LIBRARIES}
)

ADD_CUSTOM_COMMAND(
   TARGET ParaEngineServer
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ParaEngineServer> ${OUTPUT_BIN_DIR}
)
