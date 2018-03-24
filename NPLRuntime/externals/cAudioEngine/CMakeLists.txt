add_library(cAudioEngine INTERFACE)

if(ANDROID) 
    target_link_libraries(cAudioEngine INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/android/armeabi/libcAudio.a
        ${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/android/armeabi/libopenal.a
        ${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/android/armeabi/libVorbis.a
        ${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/android/armeabi/libOgg.a
        ${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/android/armeabi/libcAp_mp3Decoder.a
        ${CMAKE_CURRENT_SOURCE_DIR}/prebuilt/android/armeabi/libcommon.a
        OpenSLES
    )
    target_compile_definitions(cAudioEngine INTERFACE STATIC_PLUGIN_CAUDIOENGINE)
endif()


