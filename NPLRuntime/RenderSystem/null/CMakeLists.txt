add_library(RenderSystemNull
    RenderContextNull.cpp
    RenderContextNull.h
    RenderDeviceNull.cpp
    RenderDeviceNull.h
)
target_include_directories(RenderSystemNull PUBLIC 
    ${PROJECT_SOURCE_DIR}/ParaEngine/
    ${CMAKE_CURRENT_SOURCE_DIR}
)