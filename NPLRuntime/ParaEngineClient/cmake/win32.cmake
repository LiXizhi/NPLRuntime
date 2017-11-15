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



# Note: if true, we will produce dll instead of exe, only under win32
if(PARAENGINE_CLIENT_DLL)
	set(PARAENGINE_COMPILE_LIB    true)
	ADD_DEFINITIONS(-DNPLRUNTIME_DLL)
else()
	set(PARAENGINE_COMPILE_LIB    false)
endif()


if(NPLRUNTIME_RENDERER STREQUAL "OPENGL")
    include(cmake/win32_gl.cmake)
elseif(NPLRUNTIME_RENDERER STREQUAL "DIRECTX")
    include(cmake/win32_dx.cmake)
endif()



