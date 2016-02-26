option (USE_MSVC_PCH				"Use precompiled headers in MSVC." ON)

#########################################################################################

macro( MSVC_PCH_SUPPORT ProjectName )
if (MSVC)
	if (USE_MSVC_PCH)
	
		set_source_files_properties(${PROJECT_SOURCE_DIR}/Core/ParaEngine.cxx
			PROPERTIES
			COMPILE_FLAGS "/YcParaEngine.h"
			)
		foreach( src_file ${${ProjectName}_SRCS} )
			if(${src_file} MATCHES "cpp")
				set_source_files_properties(
					${src_file}
					PROPERTIES
					COMPILE_FLAGS "/YuParaEngine.h"
					)
			endif(${src_file} MATCHES "cpp")
		endforeach( src_file ${${ProjectName}_SRCS} )
		
		list(APPEND ${ProjectName}_SRCS ${PROJECT_SOURCE_DIR}/Core/ParaEngine.cxx)

	endif(USE_MSVC_PCH)
endif (MSVC)
endmacro (MSVC_PCH_SUPPORT)