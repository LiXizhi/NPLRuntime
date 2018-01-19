

file (GLOB src_render Render/*.cpp Render/*.h)
source_group("Render" FILES ${src_render})

add_executable(WindowsApplication 
    main.cpp
    WindowsApplication.h
    WindowsApplication.cpp
    ${src_render}
    ${src_render_d3d9}
)

target_link_libraries(WindowsApplication ParaEngine RenderSystemD3D9 opengl32.lib glad_core glad_ext_wgl)
set_target_properties(WindowsApplication PROPERTIES WIN32_EXECUTABLE true)
