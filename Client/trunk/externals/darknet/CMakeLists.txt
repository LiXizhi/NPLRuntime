set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(darknet)

IF(MSVC)
	# Statically link MSVC runtime for all configurations 
	# Debug: /MTd (MultiThreadedDebug), Release: /MT (MultiThreaded)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

	# enable multiprocessor build option /MP, this will greatly increase compile speed
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MP")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MP")
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP")
	SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MP")

	# /GR- Remove RTTI to miminize the executable size
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GR-")
	SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
ENDIF()

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/utils Source_Utils_File_List)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/layers Source_Layers_File_List)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/models Source_Models_File_List)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/cores Source_Cores_File_List)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/apps Source_Apps_File_List)

list(APPEND Source_File_List ${Source_Utils_File_List})
list(APPEND Source_File_List ${Source_Layers_File_List})
list(APPEND Source_File_List ${Source_Models_File_List})
list(APPEND Source_File_List ${Source_Cores_File_List})
list(APPEND Source_File_List ${Source_Apps_File_List})

include_directories("${lua_SOURCE_DIR}/src/")
add_library(${PROJECT_NAME} STATIC ${Source_File_List} LuaDarkNet.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include src src/include)
if (APPVEYOR)
	target_compile_definitions(${PROJECT_NAME} PUBLIC __APPVEYOR__)
endif()
