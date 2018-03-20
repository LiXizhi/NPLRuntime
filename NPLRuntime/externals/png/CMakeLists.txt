
add_library(libpng STATIC
png.c
pngerror.c
pngget.c
pngmem.c
pngpread.c
pngread.c
pngrio.c
pngrtran.c
pngrutil.c
pngset.c
pngtrans.c
pngwio.c
pngwrite.c
pngwtran.c
pngwutil.c  
)
target_link_libraries(libpng PUBLIC zlib)
target_include_directories(libpng PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(libpng PRIVATE PNG_ARM_NEON_OPT=0)

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