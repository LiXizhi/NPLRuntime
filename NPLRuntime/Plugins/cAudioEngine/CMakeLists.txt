#/**********************************************************\ 
#Original Author: Murat Sari (wolfmanfx)
#
#Created:    Feb 20, 2011
#License:    ZLib
#            
#\**********************************************************/
cmake_minimum_required(VERSION 2.8)

project(cAudio)

# Include necessary submodules
set(CMAKE_MODULE_PATH 
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake"
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Utils"
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Packages"
)



include(CMakeDependentOption)
include(PreprocessorUtils)
include(MacroLogFeature)

#####################################################################
# Set up the basic build environment
#####################################################################

if (CMAKE_BUILD_TYPE STREQUAL "")
	# CMake defaults to leaving CMAKE_BUILD_TYPE empty. This screws up
	# differentiation between debug and release builds.
	set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Choose the type of build, options are: None (CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

if (WIN32)
	# Create debug libraries with _d postfix
	set(CMAKE_DEBUG_POSTFIX "_d")
endif()


set(CAUDIO_RELEASE_PATH "/Release")
set(CAUDIO_RELWDBG_PATH "/RelWithDebInfo")
set(CAUDIO_MINSIZE_PATH "/MinSizeRel")
set(CAUDIO_DEBUG_PATH "/Debug")
set(CAUDIO_LIB_DIRECTORY "/lib")  
 
######################################################################
# Provide user options to customise the build process
######################################################################

#option(BUILD_ON_ANDROID "build on android" FALSE)
if (ANDROID)
	set(BUILD_ON_ANDROID TRUE)
endif()

# Customise what to build
# option(CAUDIO_STATIC "Static build" FALSE)
set(CAUDIO_STATIC ${AUDIO_STATICLIB})

option(CAUDIO_BUILD_SAMPLES "Build Samples" FALSE)
option(CAUDIO_BUILD_CSHARP_WRAPPER "Build CSharpWrapper - this wrapper is used with the provided *.cs files" FALSE)

#Custom settings 
option(CAUDIO_BUILD_EAX_PLUGIN "Build EAXLegacyPreset Plugin" FALSE)
option(CAUDIO_BUILD_MP3DECODER_PLUGIN "Build mp3Decoder Plugin" TRUE)
option(CAUDIO_ENABLE_OGG "Enable OGG decoder" TRUE)
option(CAUDIO_ENABLE_WAV "Enable RIFF/Wav decoder" TRUE)
option(CAUDIO_ENABLE_DEFAULT_FILESYSTEM "Enable default filesystem data source" TRUE)
option(CAUDIO_ENABLE_DEFAULT_FILE_LOGGER "Enable default file logger (html)" TRUE)
option(CAUDIO_ENABLE_DEFAULT_CONSOLE_LOGGER "Enable default console logger" TRUE)
option(CAUDIO_ENABLE_THREAD_SAFETY "Enable thread safety" TRUE)
option(CAUDIO_ENABLE_STD_MEMORY_ALLOCATOR "Enable Std Memory allocations (memalloc and free)" TRUE)
option(CAUDIO_ENABLE_REROUTE_STL_ALLOCATIONS "Reroute STL memory allocations" TRUE)
option(CAUDIO_ENABLE_MEMORYTRACKER "Enable memory leak tracker" FALSE)
option(CAUDIO_ENABLE_MEMORY_LOG_ALL_ALLOCATIONS "Enable log all memory allocations" FALSE)
option(CAUDIO_ENABLE_MEMORY_STATISTICS "Enable memory statistics generator" FALSE)
#if(APPLE)
#	option(CAUDIO_IOS_BUILD "Build for ios" FALSE)
#endif()
if (IOS)
	set(CAUDIO_IOS_BUILD TRUE)
else()
	set(CAUDIO_IOS_BUILD FALSE)
endif()

include(ConfigureBuild)

if (CAUDIO_IOS_BUILD)
  # Set build variables
  if (NOT CMAKE_OSX_SYSROOT)
	set(CMAKE_OSX_SYSROOT ${IOS_SDK})
  endif()
  set(CMAKE_OSX_DEPLOYMENT_TARGET "")
  set(CMAKE_EXE_LINKER_FLAGS "-framework Foundation -framework CoreGraphics -framework QuartzCore -framework UIKit -framework OpenGLES")
  set(XCODE_ATTRIBUTE_SDKROOT ${IOS_SDK})
  set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.YOUR_COMPANY.\${PRODUCT_NAME:rfc1034identifier}")

  # set(CMAKE_OSX_ARCHITECTURES $(ARCHS_STANDARD_32_BIT))
  #if (NOT CMAKE_OSX_ARCHITECTURES)
  #	set(CMAKE_OSX_ARCHITECTURES $(ARCHS_STANDARD_32_64_BIT))
  #endif()

  # add_definitions(-fno-regmove)
  # add_definitions(-falign-loops=16)
  remove_definitions(-msse)

  add_definitions(-DPLATFORM_IPHONE)
  add_definitions(-DPARA_TARGET_PLATFORM)
  add_definitions(-DPARA_PLATFORM_IOS)
  add_definitions(-DSTATIC_LIBRARY)
elseif (APPLE AND NOT CAUDIO_IOS_BUILD)

  # Set 10.14 as the base SDK by default
  set(XCODE_ATTRIBUTE_SDKROOT macosx)
  if (NOT CMAKE_OSX_SYSROOT)
    set(CMAKE_OSX_SYSROOT macosx)
  endif()	
  set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14)

  #if (NOT CMAKE_OSX_ARCHITECTURES)
  #  set(CMAKE_OSX_ARCHITECTURES $(ARCHS_STANDARD_32_64_BIT))
  #endif()
endif()

include(Dependencies)
include(InstallDependencies)

set(OPENAL_LIBTYPE "STATIC")

if (NOT IOS)
	add_subdirectory(DependenciesSource/openal-soft-1.19.0)
	set(OPENAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/DependenciesSource/openal-soft-1.19.0)
	set(OPENAL_INCLUDE_DIRS ${OPENAL_DIR}/include ${OPENAL_DIR}/include/AL)
endif()

if(CAUDIO_ENABLE_OGG)
	add_subdirectory(DependenciesSource/libogg-1.3.2)
	add_subdirectory(DependenciesSource/libvorbis-1.3.2)
endif()

add_subdirectory(cAudio)


if(CAUDIO_BUILD_EAX_PLUGIN)
	add_subdirectory(Plugins/EAXLegacyPreset)
endif()

if(CAUDIO_BUILD_MP3DECODER_PLUGIN)
	add_subdirectory(Plugins/mp3Decoder)
endif()

if(CAUDIO_BUILD_CSHARP_WRAPPER)
	add_subdirectory(cAudioCSharpWrapper)
endif()

if(CAUDIO_BUILD_SAMPLES)
	ADD_DEFINITIONS(-DCAUDIO_MEDIA_ROOT="${CMAKE_SOURCE_DIR}/Examples/Media/")
	add_subdirectory(Examples/Tutorial1_2DSound)
	add_subdirectory(Examples/Tutorial2_3DSound)	
	add_subdirectory(Examples/Tutorial3_MemoryPlayback)	
	add_subdirectory(Examples/Tutorial4_AudioCapture)	
	add_subdirectory(Examples/Tutorial5_AudioEffects)	
	add_subdirectory(Examples/Tutorial6_CustomEventHandler)	
	add_subdirectory(Examples/Tutorial7_CustomLogReceiver)	
endif()