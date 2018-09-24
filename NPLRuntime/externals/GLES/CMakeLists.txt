

	add_library(OpenGLES INTERFACE)
	target_include_directories(OpenGLES INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(OpenGLES INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/x86/libEGL.lib
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/x86/libGLESv2.lib
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/x86/libMaliEmulator.lib
    )