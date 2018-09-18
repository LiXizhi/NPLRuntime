cmake_minimum_required(VERSION 2.4.4)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)

project(libiconv C)

set(VERSION "1.15")

set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation directory for executables")
set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Installation directory for libraries")
set(INSTALL_INC_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH "Installation directory for headers")
set(INSTALL_MAN_DIR "${CMAKE_INSTALL_PREFIX}/share/man" CACHE PATH "Installation directory for manual pages")
set(INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_PREFIX}/share/pkgconfig" CACHE PATH "Installation directory for pkgconfig (.pc) files")

enable_testing()

set(CMAKE_REQUIRED_DEFINITIONS) # clear variable




set(ICONV_PUBLIC_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/iconv.h
)

set(ICONV_PRIVATE_HDRS)
file (GLOB ICONV_PRIVATE_HDRS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
list(APPEND ICONV_PRIVATE_HDRS ${ICONV_PRIVATE_HDRS_FILES})

set(ICONV_SRCS) 
file (GLOB ICONV_SRCS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
list(APPEND ICONV_SRCS ${ICONV_SRCS_FILES})


add_library(libiconv STATIC ${ICONV_SRCS} ${ICONV_PUBLIC_HDRS} ${ICONV_PRIVATE_HDRS})

target_include_directories(libiconv PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include  ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(MSVC)
	target_compile_definitions(libiconv PRIVATE _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
endif()


IF(ANDROID)
	set_target_properties(libiconv PROPERTIES ANDROID_STL_TYPE none)
ENDIF()

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
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GR-")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")




ENDIF()

if (APPLE)
	target_compile_definitions(libiconv PRIVATE LIBDIR='\"c\"')
elseif(UNIX)
	target_compile_definitions(libiconv PRIVATE LIBDIR='\"c\"')
endif()