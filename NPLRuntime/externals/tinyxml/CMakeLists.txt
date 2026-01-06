add_library(tinyxml tinyxml.cpp tinyxml.h tinyxmlerror.cpp tinyxmlparser.cpp)
target_include_directories(tinyxml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(tinyxml PUBLIC TIXML_USE_STL)

IF(MSVC)
	# Statically link MSVC runtime for all configurations 
	# Debug: /MTd (MultiThreadedDebug), Release: /MT (MultiThreaded)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	
	# /GR- Remove RTTI to miminize the executable size
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GR-")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
ENDIF()