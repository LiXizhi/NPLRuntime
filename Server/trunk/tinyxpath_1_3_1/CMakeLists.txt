# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2009.4.6
# Note: download tinyxpath src, and then use cmake to compile

PROJECT(tinyxpath)

# source files for tinyxpath
SET(tinyxpath_SRCS
action_store.cpp       htmlutil.cpp           lex_util.cpp
main.cpp               node_set.cpp           tinystr.cpp
tinyxml.cpp            tinyxmlerror.cpp       tinyxmlparser.cpp
tokenlist.cpp          xml_util.cpp           xpath_expression.cpp
xpath_processor.cpp    xpath_stack.cpp        xpath_static.cpp
xpath_stream.cpp       xpath_syntax.cpp 
tinyxml.h
)

ADD_DEFINITIONS(-DTIXML_USE_STL)

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
ENDIF()

ADD_LIBRARY(tinyxpath STATIC ${tinyxpath_SRCS})

