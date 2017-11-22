# Options


option(NPLRUNTIME_LUAJIT20   "build luajit2.0.4" ON)
option(NPLRUNTIME_LUA51   "build lua5.1.5 with coco" ON)
option(NPLRUNTIME_SUPPORT_FBX "support FBX files (assimp)" OFF)
option(NPLRUNTIME_PHYSICS "include physics" OFF)




FIND_PACKAGE(CURL REQUIRED)
# making executable relocatable 
SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
SET(CMAKE_INSTALL_RPATH "$ORIGIN/") 


if (NPLRUNTIME_LUAJIT21) 
	add_subdirectory (${SERVER_SOURCE_DIR}LuaJIT-2.1 ${CLIENT_BINARY_DIR}/LuaJIT-2.1)
endif()
if (NPLRUNTIME_LUA51)
	add_subdirectory (${SERVER_SOURCE_DIR}lua-5.1.4 ${CLIENT_BINARY_DIR}/lua-5.1.4)
endif()
if(NPLRUNTIME_LUAJIT20)
	add_subdirectory (${SERVER_SOURCE_DIR}LuaJIT ${CLIENT_BINARY_DIR}/LuaJIT)
endif()

# only use the latest luabind on macosx/macos-sierra
add_subdirectory (${SERVER_SOURCE_DIR}luabind-0.9.2beta  ${CLIENT_BINARY_DIR}/luabind-0.9.2beta)
add_subdirectory (${SERVER_SOURCE_DIR}sqlite-3.6.23.1 ${CLIENT_BINARY_DIR}/sqlite-3.6.23.1)
add_subdirectory (${SERVER_SOURCE_DIR}jsoncpp-0.5.0 ${CLIENT_BINARY_DIR}/jsoncpp-0.5.0)
add_subdirectory (${SERVER_SOURCE_DIR}tinyxpath_1_3_1  ${CLIENT_BINARY_DIR}/tinyxpath_1_3_1)



if(NOT NPLRUNTIME_STATIC_LIB) 
    add_subdirectory (${SERVER_SOURCE_DIR}luasql ${CLIENT_BINARY_DIR}/luasql)
endif()

if(NPLRUNTIME_SUPPORT_FBX) 
	add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/externals/assimp-4.0.0 ${CLIENT_BINARY_DIR}/assimp-4.0.0)
endif()

# main NPL runtime using ParaEngineClient
add_subdirectory (${CLIENT_SOURCE_DIR}/../NPLRuntime/ParaEngineClient ${CLIENT_BINARY_DIR}/ParaEngineClient)

# expose the core include directory 
set (ParaEngineClient_SOURCE_DIR ${CLIENT_SOURCE_DIR}/trunk/ParaEngineClient)

if(NPLRUNTIME_PHYSICS)
	add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/externals/bullet3 ${CLIENT_BINARY_DIR}/bullet3)
	add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/PhysicsBT ${CLIENT_BINARY_DIR}/PhysicsBT)
endif()
	
if(PARAENGINE_CLIENT_DLL)
	if(NPLRUNTIME_STATIC_LIB) 
	else()
		# this app is for testing the client dll
		add_subdirectory (${CLIENT_SOURCE_DIR}/trunk/ParaEngineClientApp ${CLIENT_BINARY_DIR}/ParaEngineClientApp)
	endif()
endif()

	
# following are plugins that should be build after main runtime	
if(NPLRUNTIME_STATIC_LIB) 
else()	
	add_subdirectory (${SERVER_SOURCE_DIR}NPLMono/NPLMono2  ${CLIENT_BINARY_DIR}/NPLMono/NPLMono2)
endif()

# export these two variables to its parent scope just in case some app project reference NPLRuntime statically. 
set(NPLRUNTIME_LINK_LIBRARIES ${NPLRUNTIME_LINK_LIBRARIES} PARENT_SCOPE)
set(NPLRUNTIME_LINK_DIRECTORIES ${NPLRUNTIME_LINK_DIRECTORIES} PARENT_SCOPE)
