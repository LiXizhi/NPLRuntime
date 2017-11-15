# Author: LiXizhi
# Email: lixizhi@yeah.net
# Date: 2016.4.30
# Desc:
# Output variables:
# - ${NPLRUNTIME_LINK_LIBRARIES} contains link libraries, including the NPLRuntime itself.
# - ${NPLRUNTIME_LINK_DIRECTORIES} contains all link directories.

if(WIN32)
	include(cmake/win32.cmake)
elseif(APPLE)
	include(cmake/osx.cmake)
endif()

