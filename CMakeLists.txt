
cmake_minimum_required (VERSION 3.20.0)
project (ParaCraft)

set(CMAKE_CXX_STANDARD 11)

set(SERVER_SOURCE_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/Server/trunk/)

#BOOST
if("$ENV{BOOST_ROOT}" STREQUAL "")
	set(BOOST_ROOT "D:/workspace/c/boost_1_78_0")
	message(WARNING "You can define a global environment variable of BOOST_ROOT that specify the boost root dir")
else()
	message("BOOST_ROOT env is at: $ENV{BOOST_ROOT}")
	set(BOOST_ROOT "$ENV{BOOST_ROOT}")
	string(REPLACE "\\" "/" BOOST_ROOT ${BOOST_ROOT})
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8 )
	if(IS_DIRECTORY ${BOOST_ROOT}/stage/lib64)
		set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib64)	
	endif()
else()
	if(IS_DIRECTORY ${BOOST_ROOT}/stage/lib32)
		set(BOOST_LIBRARYDIR ${BOOST_ROOT}/stage/lib32)
	endif()
endif()

set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS   ON)

if (MSVC)
    if ("$ENV{GITHUB_WORKFLOW}" STRGREATER "")
        set(Boost_USE_STATIC_RUNTIME OFF)
    else ()
        set(Boost_USE_STATIC_RUNTIME ON)
    endif()
else()
	set(Boost_USE_STATIC_RUNTIME OFF)
endif()

# Add more boost components here
find_package( Boost 1.78.0 REQUIRED COMPONENTS thread date_time filesystem system chrono regex serialization iostreams) 


#ZLIB
################################
add_subdirectory (${SERVER_SOURCE_DIR}zlib-1.2.11 zlib-1.2.11)
set(ZLIB_FOUND 1)
set(ZLIB_LIBRARY zlib)
set(ZLIB_LIBRARIES zlib)
set(ZLIB_INCLUDE_DIR ${SERVER_SOURCE_DIR}zlib-1.2.11 zlib-1.2.11)
set(ZLIB_SOURCE_DIR ${ZLIB_INCLUDE_DIR})

#CURL
option(BUILD_CURL_EXE OFF)
option(BUILD_CURL_TESTS OFF)
add_subdirectory (${SERVER_SOURCE_DIR}curl-7.47.1 curl-7.47.1)

#LUAJIT
add_subdirectory (${SERVER_SOURCE_DIR}LuaJIT LuaJIT)

add_subdirectory (${SERVER_SOURCE_DIR}luabind-0.9 luabind-0.9)
add_subdirectory (${SERVER_SOURCE_DIR}sqlite-3.6.23.1 sqlite-3.6.23.1)
add_subdirectory (${SERVER_SOURCE_DIR}jsoncpp-0.5.0 jsoncpp-0.5.0)
add_subdirectory (${SERVER_SOURCE_DIR}tinyxpath_1_3_1  tinyxpath_1_3_1)
add_subdirectory (${SERVER_SOURCE_DIR}/pcl/common common)
add_subdirectory (${SERVER_SOURCE_DIR}/pcl/features features)
add_subdirectory (${SERVER_SOURCE_DIR}/Pinocchio Pinocchio)

add_subdirectory (${SERVER_SOURCE_DIR}luasql ${CLIENT_BINARY_DIR}/luasql)
