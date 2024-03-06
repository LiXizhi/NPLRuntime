# Author: LiXizhi
# Email: lixizhi@yeah.net
# Date: 2016.4.30
# Desc:
# Output variables:
# - ${NPLRUNTIME_LINK_LIBRARIES} contains link libraries, including the NPLRuntime itself.
# - ${NPLRUNTIME_LINK_DIRECTORIES} contains all link directories.



project (ParaEngineClient)

include_directories(../embed-resource)
add_subdirectory(../embed-resource ${PROJECT_BINARY_DIR}/../embed-resource)

set (ParaEngineClient_SOURCE_DIR ${PROJECT_SOURCE_DIR}/../../Client/trunk/ParaEngineClient)

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
file (GLOB ParaEngineClient_Framework_FILES ${ParaEngineClient_SOURCE_DIR}/Framework/InputSystem/*.*)
SOURCE_GROUP("Framework" FILES ${ParaEngineClient_Framework_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Framework_FILES})

##############################
file (GLOB_RECURSE ParaEngineClient_Framework_FILES ${ParaEngineClient_SOURCE_DIR}/Framework/Common/*.*)
SOURCE_GROUP("Framework" FILES ${ParaEngineClient_Framework_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Framework_FILES})

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
#AutoRigger
file (GLOB ParaEngineClient_AutoRigger_FILES ${ParaEngineClient_SOURCE_DIR}/AutoRigger/*.cpp ${ParaEngineClient_SOURCE_DIR}/AutoRigger/*.h)
SOURCE_GROUP("AutoRigger" FILES ${ParaEngineClient_AutoRigger_FILES})
list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_AutoRigger_FILES})


if(NPLRUNTIME_RENDERER STREQUAL "OPENGL")
# OPENGL Renderer Source

	file (GLOB ParaEngineClient_GLRenerer_FILES ${ParaEngineClient_SOURCE_DIR}/renderer/OpenGL/*.*)
	SOURCE_GROUP("GLRenderer" FILES ${ParaEngineClient_GLRenerer_FILES})
	list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_GLRenerer_FILES})
	include_directories("${ParaEngineClient_SOURCE_DIR}/renderer/OpenGL/")

	##################################
	#Shader
	set(EmbeddedResource_FILES)
	if(WIN32)
		set(DX2GLTOOL ${ParaEngineClient_SOURCE_DIR}/dxEffects2glEffects/bin/dx2gl.exe)
	endif()
	if(APPLE)
		set(DX2GLTOOL ${ParaEngineClient_SOURCE_DIR}/dxEffects2glEffects/bin/dx2gl)
	endif()
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

	# Other GL shader
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

	
elseif(NPLRUNTIME_RENDERER STREQUAL "DIRECTX")

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
	# ParaXmodel.templates
	embed_resources_abs(MyResource "${ParaEngineClient_SOURCE_DIR}/res/ParaXmodel.templates")
	list(APPEND EmbeddedResource_FILES ${MyResource})
	
	##############################
	# cursor.tga
	embed_resources_abs(MyResource "${ParaEngineClient_SOURCE_DIR}/res/cursor.tga")
	list(APPEND EmbeddedResource_FILES ${MyResource})

	##############################
	# paraworld.ico
	embed_resources_abs(MyResource "${ParaEngineClient_SOURCE_DIR}/res/paraworld.ico")
	list(APPEND EmbeddedResource_FILES ${MyResource})

	##############################
	# haqi.ico
	embed_resources_abs(MyResource "${ParaEngineClient_SOURCE_DIR}/res/haqi.ico")
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
	include(${PROJECT_SOURCE_DIR}/cmake/DirectX.cmake)
	if(NOT DIRECTX_FOUND)
		message( FATAL_ERROR "Not found DirectX SDK, CMake will exit.")
	endif()

	file (GLOB ParaEngineClient_SHADER_FILES ${ParaEngineClient_SOURCE_DIR}/shaders/*.fx)	
	# custom build rules: effect FX files
	foreach( src_file ${ParaEngineClient_SHADER_FILES} )
		get_filename_component(src_filename ${src_file} NAME_WE)
	 	set(out_file "${ParaEngineClient_SOURCE_DIR}/shaders/${src_filename}.fxo")
	 	set(ParaEngineClient_SHADER_BIN_FILES ${ParaEngineClient_SHADER_BIN_FILES} ${out_file})
	 	add_custom_command(
		 	OUTPUT ${out_file}
		 	COMMAND "${DIRECTX_INCLUDE_DIR}/../Utilities/bin/x86/fxc" /Tfx_2_0 /Gfp /nologo /Fo ${out_file} ${src_file}
		 	# uncomment the following line if you want to see the text output to debug shader instructions.
		 	# COMMAND fxc /Tfx_2_0 /Gfp /nologo /Fc "${ParaEngineClient_SOURCE_DIR}/shaders/${src_filename}.txt" ${src_file}
		 	DEPENDS ${src_file}
		 	COMMENT "fxc compile: ${src_file}"
		 	)
	 	embed_resources_abs(MyResource "${ParaEngineClient_SOURCE_DIR}/shaders/${src_filename}.fxo")
	 	list(APPEND EmbeddedResource_FILES ${MyResource})
	endforeach( src_file ${ParaEngineClient_SHADER_FILES} )


endif() # ENDIF  NPLRUNTIME_RENDERER STREQUAL "DIRECTX"


if(WIN32)
	if(NPLRUNTIME_RENDERER STREQUAL "OPENGL")
		##############################
		#OpenGLTest
		file (GLOB_RECURSE ParaEngineClient_OPENGLTEST_FILES ${ParaEngineClient_SOURCE_DIR}/OpenGLTest/*.*  ${ParaEngineClient_SOURCE_DIR}/Engine/guicon.cpp ${ParaEngineClient_SOURCE_DIR}/Engine/guicon.h ${ParaEngineClient_SOURCE_DIR}/Engine/OSWindows.h ${ParaEngineClient_SOURCE_DIR}/Engine/OSWindows.cpp)
		SOURCE_GROUP("OpenGLTest" FILES ${ParaEngineClient_OPENGLTEST_FILES})
		list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_OPENGLTEST_FILES})


		##############################
		# opengl under WIN32
		file (GLOB ParaEngineClient_Platform_FILES ${ParaEngineClient_SOURCE_DIR}/platform/win32/*.*)
		list(APPEND ParaEngineClient_Platform_FILES ${External_Dir}/glad/src/glad.c)
		SOURCE_GROUP("Platform" FILES ${ParaEngineClient_Platform_FILES})
		list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Platform_FILES})
	endif()
endif()


if(APPLE)

	##############################
	file (GLOB ParaEngineClient_Engine_FILES ${ParaEngineClient_SOURCE_DIR}/Engine/ParaEngineServer.cpp)
	SOURCE_GROUP("Engine" FILES ${ParaEngineClient_Engine_FILES})
	list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Engine_FILES})

	file (GLOB ParaEngineClient_Platform_FILES ${ParaEngineClient_SOURCE_DIR}/platform/mac/*.*)
	list(APPEND ParaEngineClient_Platform_FILES ${External_Dir}/glad/src/glad.c)
	SOURCE_GROUP("Platform" FILES ${ParaEngineClient_Platform_FILES})
	list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_Platform_FILES})
endif()



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
include_directories("${ParaEngineClient_SOURCE_DIR}/AutoRigger")
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
include_directories("${luabind_SOURCE_DIR}")
include_directories("${tinyxpath_SOURCE_DIR}")
include_directories("${lua_SOURCE_DIR}/src/")
include_directories("${jsoncpp_SOURCE_DIR}/include")
include_directories("${Boost_INCLUDE_DIRS}")

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

if(NPLRUNTIME_SUPPORT_FBX)
	include_directories("${Assimp_SOURCE_DIR}/include/")
	include_directories("${Assimp_BINARY_DIR}/include")
	ADD_DEFINITIONS(-DSUPPORT_FBX_MODEL_FILE)
endif()

option(NPLRUNTIME_WINXP OFF)
if(NPLRUNTIME_WINXP)
	ADD_DEFINITIONS(-DWINXP)
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
# ZLIB
if(WIN32)
	include_directories("${ZLIB_SOURCE_DIR}")
endif()

if(APPLE)
	
endif()

##############################
# CURL
if(WIN32)
	include_directories("${ZLIB_SOURCE_DIR}")
	include_directories("${CURL_SOURCE_DIR}/include/")
	include_directories("${CURL_BINARY_DIR}/include/curl/")
endif()

if(APPLE)
	include_directories(${CURL_INCLUDE_DIR})
	list(APPEND ParaEngineClient_LIBS ${CURL_LIBRARIES})
endif()

if(NPLRUNTIME_RENDERER STREQUAL "OPENGL")

	##############################
	# OpenGL
	find_package(OpenGL REQUIRED)
	include_directories( ${OPENGL_INCLUDE_DIRS}  )
	list(APPEND ParaEngineClient_LIBS ${OPENGL_LIBRARIES})

	##############################
	# GLFW
	option(GLFW_BUILD_DOCS OFF)
	option(GLFW_BUILD_EXAMPLES OFF)
	option(GLFW_BUILD_TESTS OFF)
	option(GLFW_INSTALL OFF)
	add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/externals/glfw-3.2.1 ${ParaEngineClient_BINARY_DIR}/glfw-3.2.1)

	##############################
	# PNG
	add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/externals/lpng1634 ${ParaEngineClient_BINARY_DIR}/lpng1634)
	set(PNG_FOUND 1)
	set(PNG_LIBRARY libpng)
	set(PNG_LIBRARIES libpng)
	set(PNG_INCLUDE_DIR ${CLIENT_SOURCE_DIR}/trunk/externals/lpng1634 ${ParaEngineClient_BINARY_DIR}/lpng1634)
	set(PNG_SOURCE_DIR ${CLIENT_SOURCE_DIR}/trunk/externals/lpng1634 ${ParaEngineClient_BINARY_DIR}/lpng1634)
	include_directories("${PNG_INCLUDE_DIR}")
	list(APPEND ParaEngineClient_LIBS png_static)


	##############################
	# Freetype
	add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/externals/freetype-2.8.1 ${ParaEngineClient_BINARY_DIR}/freetype-2.8.1)
	set(FREETYPE_LIBRARY freetype)
	set(FREETYPE_LIBRARIES freetype)
	set(FREETYPE_INCLUDE_DIR ${CLIENT_SOURCE_DIR}/trunk/externals/freetype-2.8.1/include ${ParaEngineClient_BINARY_DIR}/freetype-2.8.1/include)
	set(FREETYPE_SOURCE_DIR ${CLIENT_SOURCE_DIR}/trunk/externals/freetype-2.8.1/src ${ParaEngineClient_BINARY_DIR}/freetype-2.8.1/src)
	include_directories("${FREETYPE_INCLUDE_DIR}")
	list(APPEND ParaEngineClient_LIBS freetype)


	##############################
	# GLEW
	add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/externals/glew-2.1.0/build/cmake ${ParaEngineClient_BINARY_DIR}/glew-2.1.0)
	set(GLEW_LIBRARY glew)
	set(GLEW_LIBRARIES glew)
	set(GLEW_INCLUDE_DIR ${CLIENT_SOURCE_DIR}/trunk/externals/glew-2.1.0/include ${ParaEngineClient_BINARY_DIR}/glew-2.1.0/include)
	set(GLEW_SOURCE_DIR ${CLIENT_SOURCE_DIR}/trunk/externals/glew-2.1.0/src ${ParaEngineClient_BINARY_DIR}/glew-2.1.0/src)
	include_directories("${GLEW_INCLUDE_DIR}")
	list(APPEND ParaEngineClient_LIBS glew_s)


	##############################
	# GLFW
	include_directories("${External_Dir}/glfw-3.2.1/include")
	list(APPEND ParaEngineClient_LIBS glfw)

	##############################
	# GLUT
	if(APPLE)
		find_package(GLUT REQUIRED)
		include_directories(${GLUT_INCLUDE_DIR})
		list(APPEND ParaEngineClient_LIBS ${GLUT_LIBRARY})
	endif()



	##############################
	# GLAD
	include_directories("${External_Dir}/glad/include")

	##############################
	# Jpeg
	if(WIN32)
		include_directories("${External_Dir}/jpeg/include/win32")

		if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
			list(APPEND ParaEngineClient_LINK_DIRS 	"${External_Dir}/jpeg/prebuilt/win32/x64")
		else()
			list(APPEND ParaEngineClient_LINK_DIRS 	"${External_Dir}/jpeg/prebuilt/win32/x86")
		endif()

		
		list(APPEND ParaEngineClient_LIBS libjpeg.lib)
	endif()

	if(APPLE)
		include_directories("${External_Dir}/jpeg/include/mac")
		list(APPEND ParaEngineClient_LINK_DIRS 	"${External_Dir}/jpeg/prebuilt/mac")
		list(APPEND ParaEngineClient_LIBS libjpeg.a)
	endif()


	##############################
	# DX2GL
	add_subdirectory(${CLIENT_SOURCE_DIR}/trunk/ParaEngineClient/dxEffects2glEffects ${ParaEngineClient_BINARY_DIR}/dx2gl)

	##############################
	# GLEffectParser
	add_subdirectory(${CLIENT_SOURCE_DIR}/trunk/ParaEngineClient/glEffects ${ParaEngineClient_BINARY_DIR}/glEffects)
	list(APPEND ParaEngineClient_LIBS glEffectsParser)

	if(WIN32)	
		##############################
		# DirectX ?
		list(APPEND ParaEngineClient_LIBS gdiplus.lib dinput8.lib dxguid.lib)
	endif()


	#############################
	# Luajit2.0
	if(APPLE)
		# for luajit2.0 
		SET(CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000 ${CMAKE_EXE_LINKER_FLAGS}")
	endif()

elseif(NPLRUNTIME_RENDERER STREQUAL "DIRECTX")

	##############################
	# Direct3D
	include_directories(${DIRECTX_INCLUDE_DIR})
	list(APPEND ParaEngineClient_LINK_DIRS 	${DIRECTX_LIBRARY_DIR})
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


endif() # END OF NPLRUNTIME_RENDERER STREQUAL "OPENGL"

##############################
# iconv
option(NPLRUNTIME_ICONV "include iconv" ON)

if(NPLRUNTIME_ICONV)
	ADD_DEFINITIONS(-DUSE_ICONV)

	if(APPLE)
		list(APPEND ParaEngineClient_LIBS iconv)
	else()
		add_subdirectory(${SERVER_SOURCE_DIR}libiconv ${ParaEngineClient_BINARY_DIR}/libiconv)
		set(ICONV_INCLUDE_DIR ${SERVER_SOURCE_DIR}libiconv/include ${ParaEngineClient_BINARY_DIR}/libiconv/include)
		set(ICONV_SOURCE_DIR ${SERVER_SOURCE_DIR}libiconv/src ${ParaEngineClient_BINARY_DIR}/libiconv/src)
		include_directories("${ICONV_INCLUDE_DIR}")
		list(APPEND ParaEngineClient_LIBS libiconv)
	endif()
endif()


##############################
# FreeImage is only supported under win32, since win64 Freeimage dll is currently broken on windows 11 with vs 2022 or above. 
if (WIN32 AND (CMAKE_SIZEOF_VOID_P EQUAL 4))
	option(NPLRUNTIME_FREEIMAGE "include FreeImage" ON)

	if (NPLRUNTIME_FREEIMAGE)
		ADD_DEFINITIONS(-DUSE_FREEIMAGE) # for reading image files
		if(NPLRUNTIME_RENDERER STREQUAL "DIRECTX")
			ADD_DEFINITIONS(-DUSE_GDI_COMPOSER) # for css skin, etc
		endif()

		if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
			SET(FREEIMAGE_DASM_FLAGS "x64")
		else()
			SET(FREEIMAGE_DASM_FLAGS "x32")
		endif()

		if(WIN32)
			if (NPLRUNTIME_RENDERER STREQUAL "OPENGL")
				file (GLOB_RECURSE ParaEngineClient_FREEIMIAGE ${ParaEngineClient_SOURCE_DIR}/Engine/FreeImageMemIO.h ${ParaEngineClient_SOURCE_DIR}/Engine/FreeImageMemIO.cpp)
				SOURCE_GROUP("Engine" FILES ${ParaEngineClient_FREEIMIAGE})
				list(APPEND ParaEngineClient_SRCS ${ParaEngineClient_FREEIMIAGE})
			endif()
		endif()

		

		include_directories("${External_Dir}/FreeImage_3170/Dist/${FREEIMAGE_DASM_FLAGS}")
		link_directories("${External_Dir}/FreeImage_3170/Dist/${FREEIMAGE_DASM_FLAGS}")
		list(APPEND ParaEngineClient_LIBS FreeImage.lib)
		configure_file("${External_Dir}/FreeImage_3170/Dist/${FREEIMAGE_DASM_FLAGS}/FreeImage.dll" "${OUTPUT_BIN_DIR}/FreeImage.dll" COPYONLY)

	endif()
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
	sqlite jsoncpp tinyxpath luabind )
endif()

##############################
# AutoRigger 
list(APPEND ParaEngineClient_LIBS pcl_common pcl_features Pinocchio)
	
if(APPLE)
	list(APPEND ParaEngineClient_LIBS dl resolv z pthread sqlite jsoncpp tinyxpath luabind)
endif()

if(NPLRUNTIME_STATIC_LIB)
	if(NPLRUNTIME_LUAJIT20)
		list(APPEND ParaEngineClient_LIBS luajit20)
	elseif(NPLRUNTIME_LUAJIT21)
		list(APPEND ParaEngineClient_LIBS luajit21)
	else()
		list(APPEND ParaEngineClient_LIBS lua)
	endif()
else()
	# since luajit is ABI compatible with lua, we will always link with standard lua 
	list(APPEND ParaEngineClient_LIBS lua)
endif()

# END OF LIBRARIES
# ==================================================================================

# COMPILER SETTTINGS

ADD_DEFINITIONS(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

if(NPLRUNTIME_RENDERER STREQUAL "OPENGL")

	ADD_DEFINITIONS(-DUSE_OPENGL_RENDERER)


elseif(NPLRUNTIME_RENDERER STREQUAL "DIRECTX")


	ADD_DEFINITIONS(-D_WIN32_WINNT=0x0501)
	ADD_DEFINITIONS(-DPARAENGINE_CLIENT)
	ADD_DEFINITIONS(-DUSE_DIRECTX_RENDERER)


endif() # END OF NPLRUNTIME_RENDERER STREQUAL "OPENGL"

if(WIN32)

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

	# By default MSVC has a 2^16 limit on the number of sections in an object file, and this needs more than that.
	if(MSVC)
		set_source_files_properties(${ParaEngineClient_SOURCE_DIR}/AutoRigger/Matcher.cpp PROPERTIES COMPILE_FLAGS /bigobj)
	endif(MSVC)



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


endif() # END IF WIN32


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
			ADD_DEFINITIONS(-DUSE_BOOST_FILE_API)
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

#########################
# Platform/ Apple
if(APPLE)


	ADD_DEFINITIONS(-DMAC_CLIENT)
	ADD_DEFINITIONS(-DPLATFORM_MAC)

	add_executable(ParaEngineClient MACOSX_BUNDLE ${ParaEngineClient_HEADERS} ${ParaEngineClient_SRCS})

endif()

list(APPEND ParaEngineClient_LIBS ParaAntlr4)

target_link_libraries(ParaEngineClient ${ParaEngineClient_LIBS})

ADD_CUSTOM_COMMAND(
	TARGET ParaEngineClient
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ParaEngineClient> ${OUTPUT_BIN_DIR}
 )


set(NPLRUNTIME_LINK_LIBRARIES ParaEngineClient ${NPLRUNTIME_LINK_LIBRARIES} ${ParaEngineClient_LIBS})
# export these two variables to its parent scope just in case some app project reference NPLRuntime statically. 
set(NPLRUNTIME_LINK_LIBRARIES ${NPLRUNTIME_LINK_LIBRARIES} PARENT_SCOPE)
set(NPLRUNTIME_LINK_DIRECTORIES ${NPLRUNTIME_LINK_DIRECTORIES} PARENT_SCOPE)





# END OF TARGET
# ==================================================================================

