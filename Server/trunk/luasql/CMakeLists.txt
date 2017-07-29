# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2013.1.31
# Note: based on luasql and official mysqlclient and postgresql.
# make sure the include and link directory are set correctly on linux and window.
# and then use cmake to compile
# One may also add PostgreSQL_ROOT="C:\Program Files (x86)\PostgreSQL\9.6" variable in your environment variable.

include(FindMySQL.cmake)


set(PostgreSQL_ADDITIONAL_VERSIONS "9.6")
find_package(PostgreSQL)

PROJECT(luasql)

# source files for luasql
	SET(luasql_SRCS
		src/luasql.c  src/luasql.h  src/sqlite3.def
		src/ls_sqlite3.c
		FindMySQL.cmake
	)

if(PostgreSQL_FOUND)
	list(APPEND luasql_SRCS src/ls_postgres.c src/postgres.def)
	include_directories("${PostgreSQL_INCLUDE_DIRS}")
	link_directories("${PostgreSQL_LIBRARY_DIRS}")
	list(APPEND LINK_LIBRARIES  ${PostgreSQL_LIBRARIES})
else(PostgreSQL_FOUND)
	message("Note: PostgreSQL not found, NPL luasql plugin ignored PostgreSQL support. ")
endif(PostgreSQL_FOUND)

if(MYSQL_FOUND)
	list(APPEND luasql_SRCS src/ls_mysql.c)
	if(PostgreSQL_FOUND)
		list(APPEND luasql_SRCS src/postgres_mysql.def)
	else(PostgreSQL_FOUND)
		list(APPEND luasql_SRCS src/mysql.def)
	endif(PostgreSQL_FOUND)

	include_directories("${MYSQL_INCLUDE_DIR}")
	list(APPEND LINK_LIBRARIES  ${MYSQL_LIBRARIES})
else(MYSQL_FOUND)
	message("Note: MySQL not found, NPL luasql plugin ignored MySQL support. ")
endif(MYSQL_FOUND)


IF(WIN32)
	add_definitions(-DLUA_BUILD_AS_DLL)
	add_definitions(-Dluasql_EXPORTS)
	add_definitions(-Dluasql_API=__declspec\(dllexport\))
ENDIF(WIN32)

include_directories("${lua_SOURCE_DIR}/src/")
include_directories("${sqlite_SOURCE_DIR}")


IF(MSVC)
	# statically link MSVC to reduce dependancies ONLY IN RELEASE MODE
	foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
	 	if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
	 	endif(${flag_var} MATCHES "/MD")
	endforeach(flag_var)

	# /GR- Remove RTTI to miminize the executable size
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GR-")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
ENDIF()

# the following will build lua as dll, the reason for this, is that we need its pdb files for debugging using decoda (a lua debugger).
# plus it also makes core game engine update more granular.
IF(WIN32)
	SET(CMAKE_DEBUG_POSTFIX "_d")

	# generate program database (PDB symbol files even in release build)
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
	# To turn size optimisations on again
	SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /opt:ref")
	# add a stripped version of symbol files during release build
	SET(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /PDBSTRIPPED:Release/luasql.stripped.pdb")

	ADD_LIBRARY(luasql SHARED ${luasql_SRCS})

	target_link_libraries (luasql lua sqlite ${LINK_LIBRARIES})

ELSE(WIN32)
	ADD_LIBRARY(luasql SHARED ${luasql_SRCS})

	target_link_libraries (luasql lua sqlite ${LINK_LIBRARIES})
ENDIF(WIN32)

set_target_properties(luasql PROPERTIES OUTPUT_NAME "luasql")
SET_TARGET_PROPERTIES(luasql PROPERTIES PREFIX "")

ADD_CUSTOM_COMMAND(TARGET luasql POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_BIN_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:luasql> ${OUTPUT_BIN_DIR}
)

IF(WIN32)
	if(MYSQL_FOUND)
		ADD_CUSTOM_COMMAND(TARGET luasql POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy ${MYSQL_DLL} ${OUTPUT_BIN_DIR}
		)
	endif(MYSQL_FOUND)
ENDIF(WIN32)

