# Unified Cross-Platform Robotics Configuration (MuJoCo 3.10.0 + ONNX Runtime 1.27.1)
# Supports:
#   - Emscripten WASM (delegates to WebRobot.cmake)
#   - Desktop 64-bit (Windows, macOS, Linux) via in-tree Server/trunk sources or prebuilt SDK roots

include(${CMAKE_CURRENT_LIST_DIR}/WebRobotDefaults.cmake)

if(NOT EMSCRIPTEN AND NOT NPLRUNTIME_SERVER AND NOT MAC_SERVER
   AND NOT ANDROID AND NOT IOS AND NOT HARMONY_OS AND NOT PICO_VR
   AND CMAKE_SIZEOF_VOID_P EQUAL 8 AND (WIN32 OR APPLE))
    set(_npl_default_enable_robotics ON)
else()
    set(_npl_default_enable_robotics OFF)
endif()

option(NPLRUNTIME_ENABLE_ROBOTICS "Enable MuJoCo 3.10 + ONNX Runtime 1.27 robotics subsystem" ${_npl_default_enable_robotics})
option(NPLRUNTIME_MUJOCO "Include MuJoCo 3.10 robot simulation (Desktop)" ${_npl_default_enable_robotics})
option(NPLRUNTIME_ONNXRUNTIME "Include ONNX Runtime 1.27 policy inference (Desktop)" ${_npl_default_enable_robotics})

if(NPLRUNTIME_ENABLE_ROBOTICS)
    if(EMSCRIPTEN)
        set(NPLRUNTIME_WEB_ROBOT ON CACHE BOOL "Build synchronous MuJoCo and ONNX robot APIs for Web" FORCE)
    else()
        set(NPLRUNTIME_MUJOCO ON CACHE BOOL "Include MuJoCo 3.10 robot simulation (Desktop)" FORCE)
        set(NPLRUNTIME_ONNXRUNTIME ON CACHE BOOL "Include ONNX Runtime 1.27 policy inference (Desktop)" FORCE)
    endif()
endif()

set(NPL_HAS_DESKTOP_MUJOCO FALSE)
set(NPL_HAS_DESKTOP_ONNXRUNTIME FALSE)

if(EMSCRIPTEN)
    if(NPLRUNTIME_WEB_ROBOT)
        include(${CMAKE_CURRENT_LIST_DIR}/WebRobot.cmake)
    endif()
else()
    if((NPLRUNTIME_MUJOCO OR NPLRUNTIME_ONNXRUNTIME) AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "Desktop MuJoCo 3.10.0 / ONNX Runtime 1.27.1 integration requires a 64-bit build")
    endif()

    get_filename_component(_npl_default_robot_root "${CMAKE_CURRENT_LIST_DIR}/../../Server/trunk" ABSOLUTE)
    set(ROBOT_DEPENDENCY_ROOT "${_npl_default_robot_root}" CACHE PATH "Root directory containing mujoco-3.10.0 and onnxruntime-1.27.1 sources")
    set(MUJOCO_ROOT "" CACHE PATH "Optional prebuilt MuJoCo 3.10 SDK root (include/ + lib/ or bin/)")
    set(ONNXRUNTIME_ROOT "" CACHE PATH "Optional prebuilt ONNX Runtime 1.27 SDK root (include/ + lib/)")
    set(ONNXRUNTIME_SOURCE_ROOT "${ROBOT_DEPENDENCY_ROOT}/onnxruntime-1.27.1" CACHE PATH "Path to ONNX Runtime 1.27.1 source checkout")

    # -------------------------------------------------------------------------
    # 1. Desktop MuJoCo 3.10.0
    # -------------------------------------------------------------------------
    if(NPLRUNTIME_MUJOCO)
        if(MUJOCO_ROOT AND EXISTS "${MUJOCO_ROOT}/include/mujoco/mujoco.h")
            find_library(_MUJOCO_PREBUILT_LIB NAMES mujoco mujoco310 PATHS "${MUJOCO_ROOT}/lib" "${MUJOCO_ROOT}/bin" NO_DEFAULT_PATH)
            if(NOT _MUJOCO_PREBUILT_LIB)
                message(FATAL_ERROR "MUJOCO_ROOT specified (${MUJOCO_ROOT}), but mujoco library was not found under lib/ or bin/")
            endif()
            add_library(mujoco::mujoco SHARED IMPORTED GLOBAL)
            set_target_properties(mujoco::mujoco PROPERTIES
                IMPORTED_LOCATION "${_MUJOCO_PREBUILT_LIB}"
                INTERFACE_INCLUDE_DIRECTORIES "${MUJOCO_ROOT}/include"
            )
            if(WIN32 AND EXISTS "${MUJOCO_ROOT}/bin/mujoco.dll")
                set_target_properties(mujoco::mujoco PROPERTIES
                    IMPORTED_IMPLIB "${_MUJOCO_PREBUILT_LIB}"
                    IMPORTED_LOCATION "${MUJOCO_ROOT}/bin/mujoco.dll"
                )
            elseif(WIN32 AND EXISTS "${MUJOCO_ROOT}/lib/mujoco.dll")
                set_target_properties(mujoco::mujoco PROPERTIES
                    IMPORTED_IMPLIB "${_MUJOCO_PREBUILT_LIB}"
                    IMPORTED_LOCATION "${MUJOCO_ROOT}/lib/mujoco.dll"
                )
            endif()
            set(NPL_HAS_DESKTOP_MUJOCO TRUE)
            message(STATUS "Robotics: Using prebuilt MuJoCo from ${MUJOCO_ROOT}")
        elseif(EXISTS "${ROBOT_DEPENDENCY_ROOT}/mujoco-3.10.0/CMakeLists.txt")
            set(MUJOCO_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
            set(MUJOCO_BUILD_SIMULATE OFF CACHE BOOL "" FORCE)
            set(MUJOCO_BUILD_TESTS OFF CACHE BOOL "" FORCE)
            set(MUJOCO_BUILD_STUDIO OFF CACHE BOOL "" FORCE)
            set(MUJOCO_USE_FILAMENT OFF CACHE BOOL "" FORCE)
            set(MUJOCO_TEST_PYTHON_UTIL OFF CACHE BOOL "" FORCE)
            set(MUJOCO_BUILD_MACOS_FRAMEWORKS OFF CACHE BOOL "" FORCE)
            set(tinyxml2_INSTALL_PKGCONFIG OFF CACHE BOOL "Disable unused pkg-config files in multi-config builds" FORCE)

            # Avoid passing -mavx to the arm64 slice during macOS universal (arm64;x86_64) builds.
            if(APPLE AND "arm64" IN_LIST CMAKE_OSX_ARCHITECTURES)
                set(MUJOCO_ENABLE_AVX OFF CACHE BOOL "" FORCE)
                set(MUJOCO_ENABLE_AVX_INTRINSICS OFF CACHE BOOL "" FORCE)
            endif()

            set(_NPL_SAVED_CXX_STANDARD ${CMAKE_CXX_STANDARD})
            set(_NPL_SAVED_RUNTIME_OUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
            set(_NPL_SAVED_LIBRARY_OUT ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
            set(_NPL_SAVED_ARCHIVE_OUT ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
            if(DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION)
                set(_NPL_SAVED_IPO_DEFINED TRUE)
                set(_NPL_SAVED_IPO ${CMAKE_INTERPROCEDURAL_OPTIMIZATION})
            else()
                set(_NPL_SAVED_IPO_DEFINED FALSE)
            endif()
            if(NOT WIN32)
                set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
            endif()
            if(DEFINED BUILD_SHARED_LIBS)
                set(_NPL_SAVED_BUILD_SHARED_DEFINED TRUE)
                set(_NPL_SAVED_BUILD_SHARED ${BUILD_SHARED_LIBS})
            else()
                set(_NPL_SAVED_BUILD_SHARED_DEFINED FALSE)
            endif()

            add_subdirectory("${ROBOT_DEPENDENCY_ROOT}/mujoco-3.10.0" "${CMAKE_BINARY_DIR}/mujoco-3.10.0" EXCLUDE_FROM_ALL)
            if(TARGET mujoco)
                set_target_properties(mujoco PROPERTIES DEBUG_POSTFIX "_d")
                if(NOT MSVC)
                    target_compile_options(mujoco PRIVATE -Wno-error)
                endif()
                if(WIN32)
                    set_target_properties(mujoco PROPERTIES
                        RUNTIME_OUTPUT_DIRECTORY "${_NPL_SAVED_RUNTIME_OUT}"
                        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${_NPL_SAVED_RUNTIME_OUT}"
                        RUNTIME_OUTPUT_DIRECTORY_RELEASE "${_NPL_SAVED_RUNTIME_OUT}"
                        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${_NPL_SAVED_RUNTIME_OUT}"
                        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${_NPL_SAVED_RUNTIME_OUT}"
                    )
                elseif(APPLE)
                    set_target_properties(mujoco PROPERTIES
                        MACOSX_RPATH TRUE
                    )
                endif()
            endif()

            set(CMAKE_CXX_STANDARD ${_NPL_SAVED_CXX_STANDARD})
            set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${_NPL_SAVED_RUNTIME_OUT})
            set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${_NPL_SAVED_LIBRARY_OUT})
            set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${_NPL_SAVED_ARCHIVE_OUT})
            if(_NPL_SAVED_IPO_DEFINED)
                set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ${_NPL_SAVED_IPO})
            else()
                unset(CMAKE_INTERPROCEDURAL_OPTIMIZATION)
            endif()
            if(_NPL_SAVED_BUILD_SHARED_DEFINED)
                set(BUILD_SHARED_LIBS ${_NPL_SAVED_BUILD_SHARED} CACHE BOOL "Build shared libraries" FORCE)
            else()
                unset(BUILD_SHARED_LIBS CACHE)
                unset(BUILD_SHARED_LIBS)
            endif()

            set(NPL_HAS_DESKTOP_MUJOCO TRUE)
            message(STATUS "Robotics: Building MuJoCo 3.10.0 from ${ROBOT_DEPENDENCY_ROOT}/mujoco-3.10.0")
        else()
            message(FATAL_ERROR "NPLRUNTIME_MUJOCO=ON, but neither MUJOCO_ROOT nor ${ROBOT_DEPENDENCY_ROOT}/mujoco-3.10.0/CMakeLists.txt exists")
        endif()
    endif()

    # -------------------------------------------------------------------------
    # 2. Desktop ONNX Runtime 1.27.1
    # -------------------------------------------------------------------------
    if(NPLRUNTIME_ONNXRUNTIME)
        if(ONNXRUNTIME_ROOT AND (EXISTS "${ONNXRUNTIME_ROOT}/include/onnxruntime_cxx_api.h" OR EXISTS "${ONNXRUNTIME_ROOT}/include/onnxruntime/core/session/onnxruntime_cxx_api.h"))
            if(EXISTS "${ONNXRUNTIME_ROOT}/include/onnxruntime_cxx_api.h")
                set(_ORT_INC_DIR "${ONNXRUNTIME_ROOT}/include")
            else()
                set(_ORT_INC_DIR "${ONNXRUNTIME_ROOT}/include/onnxruntime/core/session")
            endif()
            find_library(_ORT_PREBUILT_LIB NAMES onnxruntime PATHS "${ONNXRUNTIME_ROOT}/lib" "${ONNXRUNTIME_ROOT}/bin" NO_DEFAULT_PATH)
            if(NOT _ORT_PREBUILT_LIB)
                message(FATAL_ERROR "ONNXRUNTIME_ROOT specified (${ONNXRUNTIME_ROOT}), but onnxruntime library was not found under lib/ or bin/")
            endif()
            add_library(onnxruntime::onnxruntime SHARED IMPORTED GLOBAL)
            set_target_properties(onnxruntime::onnxruntime PROPERTIES
                IMPORTED_LOCATION "${_ORT_PREBUILT_LIB}"
                INTERFACE_INCLUDE_DIRECTORIES "${_ORT_INC_DIR}"
            )
            if(WIN32 AND EXISTS "${ONNXRUNTIME_ROOT}/lib/onnxruntime.dll")
                set_target_properties(onnxruntime::onnxruntime PROPERTIES
                    IMPORTED_IMPLIB "${_ORT_PREBUILT_LIB}"
                    IMPORTED_LOCATION "${ONNXRUNTIME_ROOT}/lib/onnxruntime.dll"
                )
            elseif(WIN32 AND EXISTS "${ONNXRUNTIME_ROOT}/bin/onnxruntime.dll")
                set_target_properties(onnxruntime::onnxruntime PROPERTIES
                    IMPORTED_IMPLIB "${_ORT_PREBUILT_LIB}"
                    IMPORTED_LOCATION "${ONNXRUNTIME_ROOT}/bin/onnxruntime.dll"
                )
            endif()
            set(NPL_HAS_DESKTOP_ONNXRUNTIME TRUE)
            message(STATUS "Robotics: Using prebuilt ONNX Runtime from ${ONNXRUNTIME_ROOT}")
        elseif(EXISTS "${ONNXRUNTIME_SOURCE_ROOT}/tools/ci_build/build.py")
            include(ExternalProject)
            find_package(Python3 REQUIRED COMPONENTS Interpreter)

            set(_ORT_INC_DIR "${ONNXRUNTIME_SOURCE_ROOT}/include/onnxruntime/core/session")
            if(NOT EXISTS "${_ORT_INC_DIR}/onnxruntime_cxx_api.h")
                message(FATAL_ERROR "ONNX Runtime public headers not found at ${_ORT_INC_DIR}")
            endif()

            set(_ORT_BUILD_ROOT "${CMAKE_BINARY_DIR}/onnxruntime-external")
            set(_ORT_EXTRA_ARGS)
            set(_ORT_EXTRA_CMAKE_DEFINES
                onnxruntime_BUILD_UNIT_TESTS=OFF
                CMAKE_DEBUG_POSTFIX=_d
            )

            if(MSVC)
                get_filename_component(_ORT_VC_ROOT "${CMAKE_CXX_COMPILER}" DIRECTORY)
                get_filename_component(_ORT_VC_ROOT "${_ORT_VC_ROOT}/../../../../../.." ABSOLUTE)
                if(CMAKE_GENERATOR_TOOLSET MATCHES "^v143" OR CMAKE_GENERATOR MATCHES "^Visual Studio 17")
                    set(_ORT_TOOLSET_FILE "${_ORT_VC_ROOT}/Auxiliary/Build/Microsoft.VCToolsVersion.v143.default.txt")
                else()
                    set(_ORT_TOOLSET_FILE "${_ORT_VC_ROOT}/Auxiliary/Build/Microsoft.VCToolsVersion.default.txt")
                endif()
                if(NOT EXISTS "${_ORT_TOOLSET_FILE}")
                    set(_ORT_TOOLSET_FILE "${_ORT_VC_ROOT}/Auxiliary/Build/Microsoft.VCToolsVersion.default.txt")
                endif()
                if(EXISTS "${_ORT_TOOLSET_FILE}")
                    file(STRINGS "${_ORT_TOOLSET_FILE}" _ORT_TOOLSET_VER LIMIT_COUNT 1)
                    list(APPEND _ORT_EXTRA_ARGS --msvc_toolset "${_ORT_TOOLSET_VER}")
                endif()
                if(CMAKE_GENERATOR MATCHES "^Visual Studio")
                    set(_ORT_GENERATOR "${CMAKE_GENERATOR}")
                else()
                    set(_ORT_GENERATOR "Visual Studio 17 2022")
                endif()
                list(APPEND _ORT_EXTRA_ARGS --enable_msvc_static_runtime --cmake_generator "${_ORT_GENERATOR}")
            elseif(APPLE)
                if(CMAKE_OSX_DEPLOYMENT_TARGET)
                    list(APPEND _ORT_EXTRA_ARGS --apple_deploy_target "${CMAKE_OSX_DEPLOYMENT_TARGET}")
                else()
                    list(APPEND _ORT_EXTRA_ARGS --apple_deploy_target "11.0")
                endif()
                if(CMAKE_OSX_ARCHITECTURES)
                    list(LENGTH CMAKE_OSX_ARCHITECTURES _ORT_OSX_ARCH_COUNT)
                    if(_ORT_OSX_ARCH_COUNT EQUAL 1)
                        list(APPEND _ORT_EXTRA_ARGS --osx_arch "${CMAKE_OSX_ARCHITECTURES}")
                    else()
                        # Pass valid host arch to --osx_arch and override CMAKE_OSX_ARCHITECTURES
                        # (escaped via LIST_SEPARATOR '|') so build.py produces a universal2 dylib.
                        if(CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "arm64")
                            list(APPEND _ORT_EXTRA_ARGS --osx_arch "arm64")
                        else()
                            list(APPEND _ORT_EXTRA_ARGS --osx_arch "x86_64")
                        endif()
                        string(REPLACE ";" "|" _ORT_OSX_ARCHS_PIPE "${CMAKE_OSX_ARCHITECTURES}")
                        list(APPEND _ORT_EXTRA_CMAKE_DEFINES "CMAKE_OSX_ARCHITECTURES=${_ORT_OSX_ARCHS_PIPE}")
                    endif()
                endif()
            endif()

            set(_ORT_POSTFIX "$<$<CONFIG:Debug>:_d>")
            if(WIN32)
                set(_ORT_CONFIG_DIR "${_ORT_BUILD_ROOT}/$<CONFIG>/$<CONFIG>")
                set(_ORT_BYPRODUCTS
                    "${_ORT_CONFIG_DIR}/onnxruntime${_ORT_POSTFIX}.dll"
                    "${_ORT_CONFIG_DIR}/onnxruntime${_ORT_POSTFIX}.lib"
                )
            elseif(APPLE)
                set(_ORT_BYPRODUCTS
                    "${_ORT_BUILD_ROOT}/$<CONFIG>/libonnxruntime${_ORT_POSTFIX}.1.dylib"
                    "${_ORT_BUILD_ROOT}/$<CONFIG>/libonnxruntime${_ORT_POSTFIX}.dylib"
                )
            else()
                set(_ORT_BYPRODUCTS
                    "${_ORT_BUILD_ROOT}/$<CONFIG>/libonnxruntime${_ORT_POSTFIX}.so"
                )
            endif()

            ExternalProject_Add(onnxruntime_external
                SOURCE_DIR "${ONNXRUNTIME_SOURCE_ROOT}"
                BINARY_DIR "${_ORT_BUILD_ROOT}"
                LIST_SEPARATOR |
                CONFIGURE_COMMAND ""
                BUILD_COMMAND
                    "${Python3_EXECUTABLE}" "${ONNXRUNTIME_SOURCE_ROOT}/tools/ci_build/build.py"
                    --build_dir "${_ORT_BUILD_ROOT}"
                    --config $<CONFIG>
                    --update
                    --build
                    --parallel
                    --skip_tests
                    --skip_submodule_sync
                    --disable_contrib_ops
                    --disable_ml_ops
                    --compile_no_warning_as_error
                    --build_shared_lib
                    --cmake_extra_defines ${_ORT_EXTRA_CMAKE_DEFINES}
                    --cmake_path "${CMAKE_COMMAND}"
                    ${_ORT_EXTRA_ARGS}
                INSTALL_COMMAND ""
                BUILD_BYPRODUCTS ${_ORT_BYPRODUCTS}
                USES_TERMINAL_BUILD TRUE
            )

            if(WIN32)
                ExternalProject_Add_Step(onnxruntime_external stage_runtime_dll
                    DEPENDEES build
                    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${_ORT_CONFIG_DIR}/onnxruntime${_ORT_POSTFIX}.dll"
                        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/onnxruntime${_ORT_POSTFIX}.dll"
                    COMMENT "Staging onnxruntime DLL into ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
                )
            endif()

            add_library(onnxruntime::onnxruntime SHARED IMPORTED GLOBAL)
            if(WIN32)
                set_target_properties(onnxruntime::onnxruntime PROPERTIES
                    IMPORTED_CONFIGURATIONS "Debug;Release;RelWithDebInfo;MinSizeRel"
                    IMPORTED_IMPLIB_DEBUG "${_ORT_BUILD_ROOT}/Debug/Debug/onnxruntime_d.lib"
                    IMPORTED_LOCATION_DEBUG "${_ORT_BUILD_ROOT}/Debug/Debug/onnxruntime_d.dll"
                    IMPORTED_IMPLIB_RELEASE "${_ORT_BUILD_ROOT}/Release/Release/onnxruntime.lib"
                    IMPORTED_LOCATION_RELEASE "${_ORT_BUILD_ROOT}/Release/Release/onnxruntime.dll"
                    IMPORTED_IMPLIB_RELWITHDEBINFO "${_ORT_BUILD_ROOT}/RelWithDebInfo/RelWithDebInfo/onnxruntime.lib"
                    IMPORTED_LOCATION_RELWITHDEBINFO "${_ORT_BUILD_ROOT}/RelWithDebInfo/RelWithDebInfo/onnxruntime.dll"
                    IMPORTED_IMPLIB_MINSIZEREL "${_ORT_BUILD_ROOT}/MinSizeRel/MinSizeRel/onnxruntime.lib"
                    IMPORTED_LOCATION_MINSIZEREL "${_ORT_BUILD_ROOT}/MinSizeRel/MinSizeRel/onnxruntime.dll"
                    INTERFACE_INCLUDE_DIRECTORIES "${_ORT_INC_DIR}"
                )
            elseif(APPLE)
                set_target_properties(onnxruntime::onnxruntime PROPERTIES
                    IMPORTED_CONFIGURATIONS "Debug;Release;RelWithDebInfo;MinSizeRel"
                    IMPORTED_LOCATION_DEBUG "${_ORT_BUILD_ROOT}/Debug/libonnxruntime_d.1.dylib"
                    IMPORTED_SONAME_DEBUG "@rpath/libonnxruntime_d.1.dylib"
                    IMPORTED_LOCATION_RELEASE "${_ORT_BUILD_ROOT}/Release/libonnxruntime.1.dylib"
                    IMPORTED_SONAME_RELEASE "@rpath/libonnxruntime.1.dylib"
                    IMPORTED_LOCATION_RELWITHDEBINFO "${_ORT_BUILD_ROOT}/RelWithDebInfo/libonnxruntime.1.dylib"
                    IMPORTED_SONAME_RELWITHDEBINFO "@rpath/libonnxruntime.1.dylib"
                    IMPORTED_LOCATION_MINSIZEREL "${_ORT_BUILD_ROOT}/MinSizeRel/libonnxruntime.1.dylib"
                    IMPORTED_SONAME_MINSIZEREL "@rpath/libonnxruntime.1.dylib"
                    INTERFACE_INCLUDE_DIRECTORIES "${_ORT_INC_DIR}"
                )
            else()
                set_target_properties(onnxruntime::onnxruntime PROPERTIES
                    IMPORTED_CONFIGURATIONS "Debug;Release;RelWithDebInfo;MinSizeRel"
                    IMPORTED_LOCATION_DEBUG "${_ORT_BUILD_ROOT}/Debug/libonnxruntime_d.so"
                    IMPORTED_LOCATION_RELEASE "${_ORT_BUILD_ROOT}/Release/libonnxruntime.so"
                    IMPORTED_LOCATION_RELWITHDEBINFO "${_ORT_BUILD_ROOT}/RelWithDebInfo/libonnxruntime.so"
                    IMPORTED_LOCATION_MINSIZEREL "${_ORT_BUILD_ROOT}/MinSizeRel/libonnxruntime.so"
                    INTERFACE_INCLUDE_DIRECTORIES "${_ORT_INC_DIR}"
                )
            endif()
            add_dependencies(onnxruntime::onnxruntime onnxruntime_external)
            set(NPL_HAS_DESKTOP_ONNXRUNTIME TRUE)
            message(STATUS "Robotics: Configured ONNX Runtime 1.27.1 external build from ${ONNXRUNTIME_SOURCE_ROOT}")
        else()
            message(FATAL_ERROR "NPLRUNTIME_ONNXRUNTIME=ON, but neither ONNXRUNTIME_ROOT nor ${ONNXRUNTIME_SOURCE_ROOT}/tools/ci_build/build.py exists")
        endif()
    endif()
endif()
