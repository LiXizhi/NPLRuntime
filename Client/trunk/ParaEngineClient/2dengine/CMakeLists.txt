
file (GLOB Src ${PROJECT_SOURCE_DIR}/2dengine/*.cpp)
file (GLOB Headers ${PROJECT_SOURCE_DIR}/2dengine/*.h)

add_library(2DEngine ${Src} ${Headers})
#target_link_libraries(2DEngine)
target_include_directories(2DEngine PUBLIC ${PROJECT_SOURCE_DIR}/2dengine/ )