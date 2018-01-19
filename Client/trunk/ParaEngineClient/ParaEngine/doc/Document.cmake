# Author: LiXizhi
# Company: ParaEngine.com
# Date: 2009.4.24
# Desc: Doxygen documentation project for ParaEngine. 
# Please edit STRIP_FROM_PATH in _ParaNPL doxygen config file, so that it point to ${ParaEngineClient_SOURCE_DIR}

#------- doxygen --------------
FIND_PACKAGE(Doxygen)

ADD_CUSTOM_TARGET(DOCUMENTATION
    COMMAND ${DOXYGEN} ${ParaEngineClient_SOURCE_DIR}/doc/_ParaNPL
    COMMAND ${DOXYGEN} ${ParaEngineClient_SOURCE_DIR}/doc/_ParaMain
    SOURCES ${ParaEngineClient_DOCUMENT_FILES})