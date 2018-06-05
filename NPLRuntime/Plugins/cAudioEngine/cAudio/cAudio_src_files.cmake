set (file_root_header_files
	Headers/cAudioCapture.h
	Headers/cAudioEffects.h
	Headers/cAudioManager.h
	Headers/cAudioSource.h
	Headers/cConsoleLogReceiver.h
	Headers/cEffect.h
	Headers/cEFXFunctions.h
	Headers/cFileLogReceiver.h
	Headers/cFileSource.h
	Headers/cFileSourceFactory.h
	Headers/cFilter.h
	Headers/cListener.h
	Headers/cLogger.h
	Headers/cMemoryOverride.h
	Headers/cMemorySource.h
	Headers/cMemoryTracker.h
	Headers/cMutex.h
	Headers/cPluginManager.h
	Headers/cRawAudioDecoderFactory.h
	Headers/cRawDecoder.h
	Headers/cStandardMemoryProvider.h
	Headers/cThread.h
	Headers/cWavAudioDecoderFactory.h
	Headers/cWavDecoder.h
	Headers/cOpenALAudioDeviceList.h
	Headers/cOpenALDeviceContext.h
)
set (file_root_include
	include/cAudio.h
	include/cAudioDefines.h
	include/cAudioMemory.h
	include/cAudioPlatform.h
	include/cAudioSleep.h
	include/cAudioString.h
	include/cSTLAllocator.h
	include/EAudioFormats.h
	include/IThread.h
	include/IAudioDeviceList.h
	include/IAudioCapture.h
	include/IAudioDecoder.h
	include/IAudioDecoderFactory.h
	include/IAudioEffects.h
	include/IAudioManager.h
	include/IAudioPlugin.h
	include/IAudioSource.h
	include/ICaptureEventHandler.h
	include/IDataSource.h
	include/IDataSourceFactory.h
	include/IEffect.h
	include/IEffectParameters.h
	include/IFilter.h
	include/IListener.h
	include/ILogger.h
	include/ILogReceiver.h
	include/IManagerEventHandler.h
	include/IMemoryProvider.h
	include/IPluginManager.h
	include/IRefCounted.h
	include/ISourceEventHandler.h
	include/IAudioDeviceContext.h
)
set (file_root_util
	Headers/cUtils.h
	include/cVector3.h
)
set (file_root_source_files
	src/cAudioCapture.cpp
	src/cAudioEffects.cpp
	src/cAudioManager.cpp
	src/cAudioMemory.cpp
	src/cAudioSleep.cpp
	src/cAudioSource.cpp
	src/cConsoleLogReceiver.cpp
	src/cEffect.cpp
	src/cFileLogReceiver.cpp
	src/cFileSource.cpp
	src/cFilter.cpp
	src/cListener.cpp
	src/cLogger.cpp
	src/cMemorySource.cpp
	src/cMemoryTracker.cpp
	src/cMutex.cpp
	src/cPluginManager.cpp
	src/cRawDecoder.cpp
	src/cStandardMemoryProvider.cpp
	src/cThread.cpp
	src/cWavAudioDecoderFactory.cpp
	src/cWavDecoder.cpp
	src/cAudio.cpp
	src/cOpenALAudioDeviceList.cpp
	src/cOpenALDeviceContext.cpp
)

if(CAUDIO_ENABLE_OGG)
	set (file_root_header_files
		${file_root_header_files}
		Headers/cOggAudioDecoderFactory.h
		Headers/cOggDecoder.h
	)

	set (file_root_source_files 
		${file_root_source_files}
		src/cOggAudioDecoderFactory.cpp
		src/cOggDecoder.cpp
	)
endif()
		
source_group("Header Files" FILES ${file_root_header_files})
source_group("Include" FILES ${file_root_include})
source_group("Util" FILES ${file_root_util})
source_group("Source Files" FILES ${file_root_source_files})


set (file_root
	${file_root_header_files}
	${file_root_include}
	${file_root_util}
	${file_root_source_files}
)


###############################
# ParaEngine related files
file (GLOB cAudioEngine_PEWrapper_SRCS ${PROJECT_SOURCE_DIR}/ParaEngineWrapper/*.cpp ${PROJECT_SOURCE_DIR}/ParaEngineWrapper/*.h ${PROJECT_SOURCE_DIR}/ParaEngineWrapper/*.def)
SOURCE_GROUP("ParaEngine Wrapper" FILES ${cAudioEngine_PEWrapper_SRCS})
list(APPEND file_root ${cAudioEngine_PEWrapper_SRCS})

###############################
# mp4 needs to be included in source
file (GLOB cAudioEngine_PluginMP3_SRCS ${PROJECT_SOURCE_DIR}/../Plugins/mp3Decoder/src/cMP3*.* ${PROJECT_SOURCE_DIR}/../Plugins/mp3Decoder/mpaudec/*.c ${PROJECT_SOURCE_DIR}/../Plugin/mp3Decoder/mpaudec/*.h)
SOURCE_GROUP("Plugin MP3" FILES ${cAudioEngine_PluginMP3_SRCS})
list(APPEND file_root ${cAudioEngine_PluginMP3_SRCS})