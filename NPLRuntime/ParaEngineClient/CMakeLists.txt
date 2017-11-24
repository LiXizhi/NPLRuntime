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



if(WIN32)
	include(cmake/win32.cmake)
elseif(APPLE)
	include(cmake/osx.cmake)
endif()

