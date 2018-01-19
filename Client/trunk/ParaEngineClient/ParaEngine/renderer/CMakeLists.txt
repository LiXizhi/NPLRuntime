
file (GLOB Src ${PROJECT_SOURCE_DIR}/renderer/*.cpp)
file (GLOB Headers ${PROJECT_SOURCE_DIR}/renderer/*.h)

add_library(Renderer ${Src} ${Headers})
#target_link_libraries(Renderer)
target_include_directories(Renderer PUBLIC ${PROJECT_SOURCE_DIR}/renderer/ )