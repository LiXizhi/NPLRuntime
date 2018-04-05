include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)


#SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
#SET(CMAKE_BUILD "@executable_path/../Frameworks")

ucm_add_dirs(src TO OSX_SRC RECURSIVE ADDITIONAL_EXT mm m)

ucm_add_dirs(res TO RESOURCE_FILES RECURSIVE ADDITIONAL_EXT *)
ucm_add_dirs(assets TO ASSETS_FILES RECURSIVE ADDITIONAL_EXT *)

set_source_files_properties(${RESOURCE_FILES} PROPERTICES MACOSX_PACKAGE_LOCATION Resources)

add_executable(Paracraft ${OSX_SRC} ${RESOURCE_FILES} ${ASSETS_FILES})

target_compile_definitions(Paracraft PUBLIC PLATFORM_MAC)

set_target_properties(Paracraft PROPERTIES 
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_ICON_FILE icon.icns
    RESOURCE "${RESOURCE_FILES}"
    INSTALL_RPATH "@executable_path/../Frameworks/"
    BUILD_WITH_INSTALL_RPATH TRUE
    )

find_library(COCOA_LIBRARY Cocoa required)
FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
target_link_libraries(Paracraft PRIVATE
    RenderSystemOpenGL
    ParaEngine
    ${COCOA_LIBRARY}
    ${OPENGL_LIBRARY}
)

set(CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000 ${CMAKE_EXE_LINKER_FLAGS}")


set(APP_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/Paracraft.app")
set(DYLIB_PATH "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/$<CONFIGURATION>")
set(DEST_FRAMEWORK_PATH "${APP_PATH}/Contents/Frameworks/")

set(LIBLUA_DYPATH "${DYLIB_PATH}/liblua.dylib")
set(LIBSQLITE_DYPATH "${DYLIB_PATH}/libsqlite.dylib")

add_custom_command(TARGET Paracraft POST_BUILD 
COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/assets" "${APP_PATH}/Contents/Resources/assets" 
COMMAND ${CMAKE_COMMAND} ARGS -E copy  "${LIBLUA_DYPATH}"     "${DEST_FRAMEWORK_PATH}/liblua.dylib"
COMMAND ${CMAKE_COMMAND} ARGS -E copy  "${LIBSQLITE_DYPATH}"  "${DEST_FRAMEWORK_PATH}/libsqlite.dylib"
MAIN_DEPENDENCY Paracraft
DEPENDS "${LIBLUA_DYPATH}"  "${LIBSQLITE_DYPATH}"
)



