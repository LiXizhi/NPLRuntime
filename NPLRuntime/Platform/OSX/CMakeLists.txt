# Title: Mac Cmake Lists
# Author(s): kkvskkkk, big
# CreateDate: 2018.11.02
# ModifyDate: 2021.10.09

include(${PROJECT_SOURCE_DIR}/cmake/ucm.cmake)

# set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
# set(CMAKE_BUILD "@executable_path/../Frameworks")

ucm_add_dirs(src TO OSX_SRC RECURSIVE ADDITIONAL_EXT mm m)
ucm_add_dirs(xib TO XIB_FILES RECURSIVE ADDITIONAL_EXT xib)

set(RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/res)
set(ASSETS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets)
set(INFO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist)
set(ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/icon.icns)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.14" CACHE STRING "Deployment target for OSX" FORCE)
set(CMAKE_XCODE_ATTRIBUTE_STRIP_STYLE "non-global" CACHE STRING "Non-Global Symbols" FORCE)
# set(IMAGES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Images.xcassets)

add_executable(Paracraft ${OSX_SRC}
    ${RESOURCE_FILES}
    ${ASSETS_FILES}
    ${IMAGES_FILE}
    ${XIB_FILES}
    ${ICON_FILE}
)

set_source_files_properties(${RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/res)
set_source_files_properties(${ASSETS_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/assets)
set_source_files_properties(${XIB_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/xib)
set_source_files_properties(${ICON_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION .)
# set_source_files_properties(${IMAGES_FILE} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/Images.cassets)

target_compile_definitions(Paracraft PUBLIC PLATFORM_MAC)

set(RESOURCE_LIST ${RESOURCE_FILES} ${ASSETS_FILES} ${XIB_FILES} ${ICON_FILE})

set_target_properties(Paracraft PROPERTIES 
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST ${INFO_FILE}
    # MACOSX_BUNDLE_ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/icon.icns
    XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Mac Developer"
    XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
    # XCODE_ATTRIBUTE_OTHER_CODE_SIGNING_FLAGS "--deep" [DO THIS MANUTALLY IN SETTINGS IF SANDBOX IS ENABLED!]
    XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
    XCODE_ATTRIBUTE_SKIP_INSTALL "No"
    # XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME "AppIcon"
    RESOURCE "${RESOURCE_LIST}"
    INSTALL_RPATH "@executable_path/../Frameworks/"
    XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS ${CMAKE_CURRENT_SOURCE_DIR}/Paracraft.entitlements
    BUILD_WITH_INSTALL_RPATH TRUE
)

target_include_directories(Paracraft PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

find_library(COCOA_LIBRARY Cocoa required)
find_library(OPENGL_LIBRARY OpenGL)
find_library(WebView WebKit)
# find_library(AUTHENTICATION_SERVICES AuthenticationServices)
find_library(AVFAudio AVFAudio)
find_library(IOKit IOKit)

target_link_libraries(Paracraft PUBLIC
    RenderSystemOpenGL
    ParaEngine
    ${COCOA_LIBRARY}
    ${OPENGL_LIBRARY}
    ${WebView}
    ${AVFAudio}
    # ${AUTHENTICATION_SERVICES}
    ${IOKit}
)

# no longer needed for luajit2.1, only needed or luajit 2.0
#set(CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000 ${CMAKE_EXE_LINKER_FLAGS}")

set(APP_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIGURATION>/Paracraft.app")
set(DYLIB_PATH "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/$<CONFIGURATION>")
set(DEST_FRAMEWORK_PATH "${APP_PATH}/Contents/Frameworks")

if (NOT SQLITE_STATICLIB)
    add_dependencies(Paracraft ParaSqlite)
	set(LIBSQLITE_DYPATH "${DYLIB_PATH}/libParaSqlite.dylib")
    set(LIBSQLITE_DYNAME "libParaSqlite.dylib")

	add_custom_command(TARGET Paracraft POST_BUILD
        COMMAND ${CMAKE_COMMAND} ARGS -E copy "${LIBSQLITE_DYPATH}" "${DEST_FRAMEWORK_PATH}/${LIBSQLITE_DYNAME}"
		MAIN_DEPENDENCY Paracraft
		DEPENDS "${LIBSQLITE_DYPATH}"
	)
endif ()

if (NPLRUNTIME_AUDIO AND NOT AUDIO_STATICLIB)
    add_dependencies(Paracraft cAudioEngine)

    set(LIBSAUDIO_DYPATH "${DYLIB_PATH}/libcAudioEngine.dylib")
    set(LIBSAUDIO_DYNAME "libcAudioEngine.dylib")

    add_custom_command(TARGET Paracraft POST_BUILD
        COMMAND ${CMAKE_COMMAND} ARGS -E copy "${LIBSAUDIO_DYPATH}" "${DEST_FRAMEWORK_PATH}/${LIBSAUDIO_DYNAME}"
        MAIN_DEPENDENCY Paracraft
        DEPENDS "${LIBSAUDIO_DYPATH}" cAudioEngine
    )
endif ()

if (NPLRUNTIME_PHYSICS AND NOT PHYSICS_STATICLIB)
    add_dependencies(Paracraft PhysicsBT)

    set(LIBSPHYSICS_DYPATH "${DYLIB_PATH}/libPhysicsBT.dylib")
    set(LIBSPHYSICS_DYNAME "libPhysicsBT.dylib")

    add_custom_command(TARGET Paracraft POST_BUILD
        COMMAND ${CMAKE_COMMAND} ARGS -E copy "${LIBSPHYSICS_DYPATH}" "${DEST_FRAMEWORK_PATH}/${LIBSPHYSICS_DYNAME}"
        MAIN_DEPENDENCY Paracraft
        DEPENDS "${LIBSPHYSICS_DYPATH}" PhysicsBT
    )
endif ()

# if (NOT LUA_STATICLIB)
# 	get_property(LUA_LIB_LIST GLOBAL PROPERTY "LUA_LIB_LIST" )

# 	foreach(luaLibName ${LUA_LIB_LIST})
# 		set(LIBLUA_DYPATH "${DYLIB_PATH}/lib${luaLibName}.dylib")
# 		add_custom_command(TARGET Paracraft POST_BUILD 
# 			COMMAND ${CMAKE_COMMAND} ARGS -E copy "${LIBLUA_DYPATH}" "${DEST_FRAMEWORK_PATH}/lib${luaLibName}.dylib"
# 			DEPENDS "${LIBLUA_DYPATH}"  
# 		)
# 	endforeach(luaLibName ${LUA_LIB_LIST})

# endif()
