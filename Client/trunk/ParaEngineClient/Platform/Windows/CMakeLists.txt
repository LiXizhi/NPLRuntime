
add_executable(WindowsApplication 
    main.cpp
    WindowsApplication.h
    WindowsApplication.cpp
    WindowsRenderWindow.cpp
    WindowsRenderWindow.h
)

target_link_libraries(WindowsApplication ParaEngineLibrary)
set_target_properties(WindowsApplication PROPERTIES WIN32_EXECUTABLE true)
