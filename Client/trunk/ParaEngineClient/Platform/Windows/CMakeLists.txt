

file (GLOB src_render Render/*.cpp Render/*.h)
source_group("Render" FILES ${src_render})

file (GLOB src_render_d3d9 Render/D3D9/*.cpp Render/D3D9/*.h)
source_group("Render\\D3D9" FILES ${src_render_d3d9})



add_executable(WindowsApplication 
    main.cpp
    WindowsApplication.h
    WindowsApplication.cpp
    ${src_render}
    ${src_render_d3d9}
)

target_link_libraries(WindowsApplication ParaEngineLibrary)
set_target_properties(WindowsApplication PROPERTIES WIN32_EXECUTABLE true)
