# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2010.3.2
# Desc: The client exe application for ParaEngine. It is just a small exe that uses ParaEngineClient.dll 

project (ParaEngineClientApp)

# The version number.
set (ParaEngineClientApp_VERSION_MAJOR 1)
set (ParaEngineClientApp_VERSION_MINOR 0)

##############################  
# source code
SET(ParaEngineClientApp_SRCS
	ParaEngineClientApp.rc
	res/paraworld.ico
	resource.h
	config.h

	AutoUpdaterClient.cpp		AutoUpdaterClient.h
	AutoUpdaterApp.cpp   AutoUpdaterApp.h
	ParaEngineClientApp.cpp		ParaEngineClientApp.h
	InterprocessAppClient.cpp		InterprocessAppClient.h
	readme.txt version.txt 
)

##############################  
# installer files
file (GLOB ParaEngineClientApp_install_FILES ${PROJECT_SOURCE_DIR}/*.bat)
SOURCE_GROUP("Installer" FILES ${ParaEngineClientApp_install_FILES})
list(APPEND ParaEngineClientApp_SRCS ${ParaEngineClientApp_install_FILES})


##############################  
# web files
file (GLOB ParaEngineClientApp_js_FILES ${PROJECT_SOURCE_DIR}/js/*.js)
SOURCE_GROUP("Java Scripts" FILES ${ParaEngineClientApp_js_FILES})
list(APPEND ParaEngineClientApp_SRCS ${ParaEngineClientApp_js_FILES})

##############################  
# web files
file (GLOB ParaEngineClientApp_WEB_FILES RELATIVE ${PROJECT_SOURCE_DIR}
    haqi*.html
    )
list(APPEND ParaEngineClientApp_SRCS ${ParaEngineClientApp_WEB_FILES})

##############################  
# web files
file (GLOB ParaEngineClientApp_Haqi_FILES RELATIVE ${PROJECT_SOURCE_DIR}
    haqifiles/*.html
	intro*.html
    )
SOURCE_GROUP("Haqi Files" FILES ${ParaEngineClientApp_Haqi_FILES})    
list(APPEND ParaEngineClientApp_SRCS ${ParaEngineClientApp_Haqi_FILES})

##############################
# add the binary tree to the search path for include files
include_directories("${Boost_INCLUDE_DIRS}")
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${ParaEngineClient_SOURCE_DIR}/../../Client/trunk/ParaEngineClient")
include_directories("${ParaEngineClient_SOURCE_DIR}/../../Client/trunk/ParaEngineClient/Core")

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
	
	# /GR- Remove RTTI to miminize the executable size
	# /Zi generate program database (PDB symbol files even in release build)
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi /GR-")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
	# To turn size optimisations on again
	SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /opt:ref /DEBUG /LARGEADDRESSAWARE")
ENDIF()

# add the executable
add_executable(ParaEngineClientApp ${ParaEngineClientApp_SRCS})

# When this property is set to true the executable when linked on Windows will be created with a WinMain() entry point instead of of just main().
set_target_properties(ParaEngineClientApp PROPERTIES WIN32_EXECUTABLE true)
# set_target_properties(ParaEngineClientApp PROPERTIES LINK_FLAGS /MANIFESTUAC:"level='requireAdministrator' uiAccess='false'")
# Since we are installing to local app data dir from now on, we will only need medium integrity level to run the app, however protected mode IE only gives use low IL, in which we need to write to registry to promote this process to medium IL. 
# set_target_properties(ParaEngineClientApp PROPERTIES LINK_FLAGS /MANIFESTUAC:"level='highestAvailable' uiAccess='false'")

set_target_properties(ParaEngineClientApp PROPERTIES OUTPUT_NAME "ParaEngineClient")
set_target_properties(ParaEngineClientApp PROPERTIES DEBUG_OUTPUT_NAME "ParaEngineClient_d")


# Link the executable to the libraries. 
target_link_libraries (ParaEngineClientApp Psapi.lib ${Boost_LIBRARIES}) 

# Copy files Link the executable to the libraries. 
if(MSVC)

get_target_property(DEBUG_EXE_PATH ParaEngineClientApp DEBUG_LOCATION)
STRING(REGEX REPLACE "^.*[/\\]([^/\\]+)$" "\\1" DEBUG_EXE_NAME "${DEBUG_EXE_PATH}" )

get_target_property(RELEASE_EXE_PATH ParaEngineClientApp RELEASE_LOCATION)
STRING(REGEX REPLACE "^.*[/\\]([^/\\]+)$" "\\1" RELEASE_EXE_NAME "${RELEASE_EXE_PATH}" )

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set( PARAWORLD_BIN_DIR "${CLIENT_SOURCE_DIR}/../../ParaWorld/bin64/" )
else()
	set( PARAWORLD_BIN_DIR "${CLIENT_SOURCE_DIR}/../../ParaWorld/" )
endif()

ADD_CUSTOM_COMMAND(
   TARGET ParaEngineClientApp
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy ${DEBUG_EXE_PATH} ${PARAWORLD_BIN_DIR}${DEBUG_EXE_NAME}
   COMMAND ${CMAKE_COMMAND} -E copy ${RELEASE_EXE_PATH} ${PARAWORLD_BIN_DIR}${RELEASE_EXE_NAME}
)
endif(MSVC)