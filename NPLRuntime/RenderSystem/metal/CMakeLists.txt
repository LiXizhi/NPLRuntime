add_library(RenderSystemMetal
    RenderContextMetal.h
    RenderDeviceMetal.h
    RenderDeviceMetal.mm
)

target_include_directories(RenderSystemMetal PUBLIC
    ${PROJECT_SOURCE_DIR}/ParaEngine/Framework/Interface
    ${PROJECT_SOURCE_DIR}/ParaEngine/renderer
    ${PROJECT_SOURCE_DIR}/ParaEngine
    ${PROJECT_SOURCE_DIR}/RenderSystem/shader_compiler
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_options(RenderSystemMetal PRIVATE -fno-objc-arc)

find_library(METAL_LIBRARY Metal REQUIRED)
find_library(QUARTZCORE_LIBRARY QuartzCore REQUIRED)
find_library(FOUNDATION_LIBRARY Foundation REQUIRED)

target_link_libraries(RenderSystemMetal
    PUBLIC RenderSystemRHICore ParaEngine
    PRIVATE ${METAL_LIBRARY} ${QUARTZCORE_LIBRARY} ${FOUNDATION_LIBRARY}
)

if(IOS)
    find_library(UIKIT_LIBRARY UIKit REQUIRED)
    target_link_libraries(RenderSystemMetal PRIVATE ${UIKIT_LIBRARY})
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    target_link_libraries(RenderSystemMetal PRIVATE ${COCOA_LIBRARY})
endif()
