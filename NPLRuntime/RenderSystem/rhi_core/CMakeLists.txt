add_library(RenderSystemRHICore
    RHIStateTracker.h
    RHIStateTracker.cpp
    RHIDynamicRingBuffer.h
    RHIDynamicRingBuffer.cpp
    RHISoftwareS3TC.h
    RHISoftwareS3TC.cpp
    RHIBackendRegistry.h
    RHIBackendRegistry.cpp
)

target_include_directories(RenderSystemRHICore PUBLIC
    ${PROJECT_SOURCE_DIR}/ParaEngine/Framework/Interface
    ${PROJECT_SOURCE_DIR}/ParaEngine/renderer
    ${PROJECT_SOURCE_DIR}/ParaEngine
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if (WIN32 OR APPLE OR EMSCRIPTEN OR SDL OR ANDROID_STUDIO OR NPLRUNTIME_SERVER)
    target_compile_definitions(RenderSystemRHICore PUBLIC PARA_RHI_MULTI_BACKEND)
endif()
