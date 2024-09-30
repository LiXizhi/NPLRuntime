
cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(DarkNet)

# 设置编译器的源文件编码为 UTF-8
add_definitions(/source-charset:utf-8)
# 设置执行字符集为 UTF-8
add_definitions(/execution-charset:utf-8)

unset(Source_File_List)
unset(TARGET_INCLUDE_LIST)
unset(TARGET_LIB_LIST)
unset(TARGET_DEFINITIONS_LIST)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/utils Source_Utils_File_List)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/layers Source_Layers_File_List)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/models Source_Models_File_List)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/cores Source_Cores_File_List)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/apps Source_Apps_File_List)

list(APPEND Source_File_List ${Source_Utils_File_List})
list(APPEND Source_File_List ${Source_Layers_File_List})
list(APPEND Source_File_List ${Source_Models_File_List})
list(APPEND Source_File_List ${Source_Cores_File_List})
list(APPEND Source_File_List ${Source_Apps_File_List})

list(APPEND TARGET_INCLUDE_LIST ${CMAKE_CURRENT_SOURCE_DIR})
list(APPEND TARGET_INCLUDE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(TARGET_NAME "${APPLICATION_NAME}${PROJECT_NAME}")
add_library(${TARGET_NAME} STATIC ${Source_File_List})
target_include_directories(${TARGET_NAME} PRIVATE ${TARGET_INCLUDE_LIST})
target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${TARGET_NAME} PUBLIC ${TARGET_LIB_LIST})
target_compile_definitions(${TARGET_NAME} PUBLIC ${TARGET_DEFINITIONS_LIST})

set(TARGET_NAME "${APPLICATION_NAME}${PROJECT_NAME}Exe")
add_executable(${TARGET_NAME} main.cpp)
target_include_directories(${TARGET_NAME} PRIVATE ${TARGET_INCLUDE_LIST})
target_link_libraries(${TARGET_NAME} PUBLIC ${TARGET_LIB_LIST} "${APPLICATION_NAME}${PROJECT_NAME}")
target_compile_definitions(${TARGET_NAME} PUBLIC ${TARGET_DEFINITIONS_LIST})
