



# Embed Resource
include_directories(../embed-resource)
add_subdirectory(../embed-resource ${PROJECT_BINARY_DIR}/../embed-resource)


##############################
file (GLOB ParaEngineClient_Engine_FILES ${ParaEngineClient_SOURCE_DIR}/Engine/ParaEngineServer.cpp)
SOURCE_GROUP("Engine" FILES ${ParaEngineClient_Engine_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Engine_FILES})




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


# OpenGL
ADD_DEFINITIONS(-DUSE_OPENGL_RENDERER)
find_package(OpenGL REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  )
set(GRAPHIC_LIBRARIES ${OPENGL_LIBRARIES}	)
find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIR})






set(EmbeddedResource_FILES)

##################################
#------------shader-------------##

set(DX2GLTOOL ${ParaEngineClient_SOURCE_DIR}/dxEffects2glEffects/bin/dx2gl)	


#Add shader files to the project.
message("Genrate Shaders.")


file (GLOB ParaEngineClient_SHADER_FILES ${ParaEngineClient_SOURCE_DIR}/shaders/fx/*.fx)

# custom build rules: effect FX files
foreach( src_file ${ParaEngineClient_SHADER_FILES} )
  
	get_filename_component(src_filename ${src_file} NAME_WE)
	set(out_file "${ParaEngineClient_SOURCE_DIR}/shaders/glsl/${src_filename}.fx.glsl")
	 message(${out_file})
	set(ParaEngineClient_SHADER_BIN_FILES ${ParaEngineClient_SHADER_BIN_FILES} ${out_file})
	add_custom_command(
		OUTPUT ${out_file}
		COMMAND "${DX2GLTOOL}" -glsl  ${src_file} ${out_file}
		DEPENDS ${src_file} dx2gl 
		COMMENT "dx2gl : ${src_file}"
		)
	embed_resources_abs(MyResource ${out_file})
	list(APPEND EmbeddedResource_FILES ${MyResource})
endforeach( src_file ${ParaEngineClient_SHADER_FILES} )

embed_resources_abs(MyResource  "${ParaEngineClient_SOURCE_DIR}/shaders/glsl/terrain_normal.fx.glsl")
list(APPEND ParaEngineClient_SHADER_FILES "${ParaEngineClient_SOURCE_DIR}/shaders/glsl/terrain_normal.fx.glsl")
list(APPEND EmbeddedResource_FILES ${MyResource})
embed_resources_abs(MyResource  "${ParaEngineClient_SOURCE_DIR}/shaders/glsl/skydome.fx.glsl")
list(APPEND ParaEngineClient_SHADER_FILES "${ParaEngineClient_SOURCE_DIR}/shaders/glsl/skydome.fx.glsl")
list(APPEND EmbeddedResource_FILES ${MyResource})
embed_resources_abs(MyResource  "${ParaEngineClient_SOURCE_DIR}/shaders/glsl/singleColorEffect.fx.glsl")
list(APPEND ParaEngineClient_SHADER_FILES "${ParaEngineClient_SOURCE_DIR}/shaders/glsl/singleColorEffect.fx.glsl")
list(APPEND EmbeddedResource_FILES ${MyResource})
embed_resources_abs(MyResource  "${ParaEngineClient_SOURCE_DIR}/shaders/glsl/simple_mesh_normal.fx.glsl")
list(APPEND ParaEngineClient_SHADER_FILES "${ParaEngineClient_SOURCE_DIR}/shaders/glsl/simple_mesh_normal.fx.glsl")
list(APPEND EmbeddedResource_FILES ${MyResource})
embed_resources_abs(MyResource  "${ParaEngineClient_SOURCE_DIR}/shaders/glsl/guiTextEffect.fx.glsl")
list(APPEND ParaEngineClient_SHADER_FILES "${ParaEngineClient_SOURCE_DIR}/shaders/glsl/guiTextEffect.fx.glsl")
list(APPEND EmbeddedResource_FILES ${MyResource})
embed_resources_abs(MyResource  "${ParaEngineClient_SOURCE_DIR}/shaders/glsl/guiEffect.fx.glsl")
list(APPEND ParaEngineClient_SHADER_FILES "${ParaEngineClient_SOURCE_DIR}/shaders/glsl/guiEffect.fx.glsl")
list(APPEND EmbeddedResource_FILES ${MyResource})

SOURCE_GROUP("Shader Files" FILES ${ParaEngineClient_SHADER_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_SHADER_FILES})


##############################
# embedded resource
list(APPEND EmbeddedResource_FILES ${ParaEngineClient_SOURCE_DIR}/ResourceEmbedded.cpp)
list(APPEND ParaEngineClient_SRCS ${EmbeddedResource_FILES})
SOURCE_GROUP("ResEmbedded" FILES ${EmbeddedResource_FILES})

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


ADD_DEFINITIONS(-DMAC_CLIENT)

set (External_Dir ${ParaEngineClient_SOURCE_DIR}/../externals)
file (GLOB ParaEngineClient_Platform_FILES ${ParaEngineClient_SOURCE_DIR}/platform/mac/*.*)
SOURCE_GROUP("Platform" FILES ${ParaEngineClient_Platform_FILES})
list(APPEND ParaEngineClient_Platform_FILES 
${ParaEngineClient_SOURCE_DIR}/platform/OpenGLWrapper.h
${ParaEngineClient_SOURCE_DIR}/platform/base/s3tc.cpp ${ParaEngineClient_SOURCE_DIR}/platform/base/s3tc.h
${ParaEngineClient_SOURCE_DIR}/platform/base/edtaa3func.cpp ${ParaEngineClient_SOURCE_DIR}/platform/base/edtaa3func.h
${ParaEngineClient_SOURCE_DIR}/platform/base/uthash.h
${External_Dir}/glad/src/glad.c)
include_directories("${External_Dir}/glfw-3.2.1/include")
include_directories("${External_Dir}/glad/include")
include_directories("${External_Dir}/jpeg/include/mac")
#nclude_directories("${External_Dir}/icon/include")

include_directories("${CURL_SOURCE_DIR}/include/")
include_directories("${CURL_BINARY_DIR}/include/curl/")

set(NPLRUNTIME_LINK_DIRECTORIES ${NPLRUNTIME_LINK_DIRECTORIES} 
	"${External_Dir}/jpeg/prebuilt/mac"
	)
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Platform_FILES})

set(GRAPHIC_LIBRARIES ${GRAPHIC_LIBRARIES} glfw libjpeg.a glEffectsParser)



file (GLOB Platform_Base_FILES ${ParaEngineClient_SOURCE_DIR}/platform/base/*.cpp)
SOURCE_GROUP("Platform_Base" FILES ${Platform_Base_FILES})
list(APPEND ParaEngineClient_SRCS ${Platform_Base_FILES})


set (External_Dir ${PROJECT_SOURCE_DIR}/../../Client/trunk/externals)
include_directories("${External_Dir}/jpeg/include/mac")



##############################

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_INSTALL OFF)

add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/externals/glfw-3.2.1 ${ParaEngineClient_BINARY_DIR}/glfw-3.2.1)
add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/externals/lpng1634 ${ParaEngineClient_BINARY_DIR}/lpng1634)
set(PNG_FOUND 1)
set(PNG_LIBRAY libpng)
set(PNG_LIBRARIES libpng)
set(PNG_INCLUDE_DIR ${CLIENT_SOURCE_DIR}/trunk/externals/lpng1634 ${ParaEngineClient_BINARY_DIR}/lpng1634)
set(PNG_SOURCE_DIR ${CLIENT_SOURCE_DIR}/trunk/externals/lpng1634 ${ParaEngineClient_BINARY_DIR}/lpng1634)

add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/externals/freetype-2.8.1 ${ParaEngineClient_BINARY_DIR}/freetype-2.8.1)
set(FREETYPE_LIBRARY freetype)
set(FREETYPE_LIBRARIES freetype)
set(FREETYPE_INCLUDE_DIR ${CLIENT_SOURCE_DIR}/trunk/externals/freetype-2.8.1/include ${ParaEngineClient_BINARY_DIR}/freetype-2.8.1/include)
set(FREETYPE_SOURCE_DIR ${CLIENT_SOURCE_DIR}/trunk/externals/freetype-2.8.1/src ${ParaEngineClient_BINARY_DIR}/freetype-2.8.1/src)
add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/externals/glew-2.1.0/build/cmake ${ParaEngineClient_BINARY_DIR}/glew-2.1.0)
set(GLEW_LIBRARY glew)
set(GLEW_LIBRARIES glew)
set(GLEW_INCLUDE_DIR ${CLIENT_SOURCE_DIR}/trunk/externals/glew-2.1.0/include ${ParaEngineClient_SOURCE_DIR}/glew-2.1.0/include)
set(GLEW_SOURCE_DIR ${CLIENT_SOURCE_DIR}/trunk/externals/glew-2.1.0/src ${ParaEngineClient_SOURCE_DIR}/glew-2.1.0/src)


##############################
add_subdirectory(${CLIENT_SOURCE_DIR}/trunk/ParaEngineClient/dxEffects2glEffects ${ParaEngineClient_BINARY_DIR}/dx2gl)
add_subdirectory(${CLIENT_SOURCE_DIR}/trunk/ParaEngineClient/glEffects ${ParaEngineClient_BINARY_DIR}/glEffects)

##############################
include_directories("${PNG_INCLUDE_DIR}")
include_directories("${FREETYPE_INCLUDE_DIR}")
include_directories("${GLEW_INCLUDE_DIR}")

set(GRAPHIC_LIBRARIES ${GRAPHIC_LIBRARIES} png_static freetype glew_s)


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



INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
include_directories("${lua_SOURCE_DIR}/src/")
include_directories("${luabind_SOURCE_DIR}")
include_directories("${sqlite_SOURCE_DIR}")
include_directories("${Boost_INCLUDE_DIRS}")
include_directories("${tinyxpath_SOURCE_DIR}")
include_directories("${jsoncpp_SOURCE_DIR}/include")

if(NPLRUNTIME_SUPPORT_FBX)
	include_directories("${Assimp_SOURCE_DIR}/include/")
	include_directories("${Assimp_BINARY_DIR}/include")
	ADD_DEFINITIONS(-DSUPPORT_FBX_MODEL_FILE)
endif()

ADD_DEFINITIONS(-DPLATFORM_MAC)
ADD_DEFINITIONS(-DBOOST_SIGNALS_NO_DEPRECATION_WARNING)
ADD_DEFINITIONS(-DTIXML_USE_STL)
if(NPLRUNTIME_STATIC_LIB)
	ADD_DEFINITIONS(-DNPLRUNTIME_STATICLIB)
	ADD_DEFINITIONS(-DSQLITE_STATICLIB)
	ADD_DEFINITIONS(-DCURL_STATICLIB)
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


# NOTE: This generate smaller and faster exe/dll, but no other libs should to it statically.
option(USE_LINK_TIME_CODE_GENERATION	"true to enable link time code gen in release build" OFF)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

# remove Run-Time-Type-Information (RTTI) to miminize the executable size
# set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fno-rtti")

# -Os to optimize for size. or -O3 to turn on all optimizations.
# -s option to strip debug info, -g to include debug info
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -s")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -s")

# suppress warnings
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wno-deprecated-declarations")


link_directories(${NPLRUNTIME_LINK_DIRECTORIES})

add_executable(ParaEngineClient MACOSX_BUNDLE ${ParaEngineClient_HEADERS} ${ParaEngineClient_SRCS})

set(EXTRA_LIBRARIES
		dl
		resolv
		z
		iconv
	${WRAPPER_LIB}
	${CURL_LIBRARIES}
	${GLUT_LIBRARY}
	${OPENGL_LIBRARY}
	pthread
)

if (NPLRUNTIME_SUPPORT_FBX)
	set(EXTRA_LIBRARIES ${EXTRA_LIBRARIES} assimp)
endif()
 
# for luajit2.0 
SET(CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000 ${CMAKE_EXE_LINKER_FLAGS}")

set(NPLRUNTIME_LINK_LIBRARIES sqlite liblua jsoncpp tinyxpath luabind
	${GRAPHIC_LIBRARIES}
	${EXTRA_LIBRARIES}
	${Boost_LIBRARIES}
)

# Link the executable to the libraries.
target_link_libraries(ParaEngineClient ${NPLRUNTIME_LINK_LIBRARIES})

set(NPLRUNTIME_LINK_LIBRARIES ParaEngineClient ${NPLRUNTIME_LINK_LIBRARIES})

# export these two variables to its parent scope just in case some app project reference NPLRuntime statically. 
set(NPLRUNTIME_LINK_LIBRARIES ${NPLRUNTIME_LINK_LIBRARIES} PARENT_SCOPE)
set(NPLRUNTIME_LINK_DIRECTORIES ${NPLRUNTIME_LINK_DIRECTORIES} PARENT_SCOPE)

ADD_CUSTOM_COMMAND(
   TARGET ParaEngineClient
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ParaEngineClient> ${OUTPUT_BIN_DIR}
)
