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

ADD_LIBRARY(tinyxpath STATIC ${tinyxpath_SRCS})
target_compile_definitions(tinyxpath PUBLIC TIXML_USE_STL)
target_include_directories(tinyxpath PUBLIC ${PROJECT_SOURCE_DIR})