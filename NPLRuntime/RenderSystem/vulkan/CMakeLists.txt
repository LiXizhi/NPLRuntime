add_library(RenderSystemVulkan
    VulkanLoader.h
    VulkanLoader.cpp
    RenderContextVulkan.h
    RenderContextVulkan.cpp
    RenderDeviceVulkan.h
    RenderDeviceVulkan.cpp
)

target_include_directories(RenderSystemVulkan PUBLIC
    ${PROJECT_SOURCE_DIR}/ParaEngine/Framework/Interface
    ${PROJECT_SOURCE_DIR}/ParaEngine/renderer
    ${PROJECT_SOURCE_DIR}/ParaEngine
    ${PROJECT_SOURCE_DIR}/RenderSystem/shader_compiler
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(RenderSystemVulkan
    PUBLIC RenderSystemRHICore ParaEngine
)

if(WIN32)
    target_compile_definitions(RenderSystemVulkan PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _HAS_STD_BYTE=0
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(RenderSystemVulkan PRIVATE dl)
endif()
