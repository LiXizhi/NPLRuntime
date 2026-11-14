# Unified Cross-Platform Robotics Configuration (MuJoCo 3.10.0 + ONNX Runtime 1.27.1)
# Supports:
#   - Emscripten WASM (delegates to WebRobot.cmake)
#   - Desktop 64-bit (Windows, Linux, macOS) via in-tree Server/trunk sources or prebuilt SDK roots

include(${CMAKE_CURRENT_LIST_DIR}/WebRobotDefaults.cmake)

option(NPLRUNTIME_ENABLE_ROBOTICS "Enable MuJoCo 3.10 + ONNX Runtime 1.27 robotics subsystem" OFF)
option(NPLRUNTIME_MUJOCO "Include MuJoCo 3.10 robot simulation (Desktop)" OFF)
option(NPLRUNTIME_ONNXRUNTIME "Include ONNX Runtime 1.27 policy inference (Desktop)" OFF)

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
            add_library(mujoco::mujoco UNKNOWN IMPORTED GLOBAL)
            set_target_properties(mujoco::mujoco PROPERTIES
                IMPORTED_LOCATION "${_MUJOCO_PREBUILT_LIB}"
                INTERFACE_INCLUDE_DIRECTORIES "${MUJOCO_ROOT}/include"
            )
            set(NPL_HAS_DESKTOP_MUJOCO TRUE)
            message(STATUS "Robotics: Using prebuilt MuJoCo from ${MUJOCO_ROOT}")
        elseif(EXISTS "${ROBOT_DEPENDENCY_ROOT}/mujoco-3.10.0/CMakeLists.txt")
            set(MUJOCO_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
            set(MUJOCO_BUILD_SIMULATE OFF CACHE BOOL "" FORCE)
            set(MUJOCO_BUILD_TESTS OFF CACHE BOOL "" FORCE)
            set(MUJOCO_BUILD_STUDIO OFF CACHE BOOL "" FORCE)
            set(MUJOCO_USE_FILAMENT OFF CACHE BOOL "" FORCE)
            set(MUJOCO_TEST_PYTHON_UTIL OFF CACHE BOOL "" FORCE)
            set(tinyxml2_INSTALL_PKGCONFIG OFF CACHE BOOL "Disable unused pkg-config files in multi-config builds" FORCE)

            set(_NPL_SAVED_CXX_STANDARD ${CMAKE_CXX_STANDARD})
            set(_NPL_SAVED_RUNTIME_OUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
            set(_NPL_SAVED_LIBRARY_OUT ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
            set(_NPL_SAVED_ARCHIVE_OUT ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
            if(DEFINED BUILD_SHARED_LIBS)
                set(_NPL_SAVED_BUILD_SHARED_DEFINED TRUE)
                set(_NPL_SAVED_BUILD_SHARED ${BUILD_SHARED_LIBS})
            else()
                set(_NPL_SAVED_BUILD_SHARED_DEFINED FALSE)
            endif()

            add_subdirectory("${ROBOT_DEPENDENCY_ROOT}/mujoco-3.10.0" "${CMAKE_BINARY_DIR}/mujoco-3.10.0" EXCLUDE_FROM_ALL)
            if(TARGET mujoco)
                set_target_properties(mujoco PROPERTIES DEBUG_POSTFIX "_d")
            endif()

            set(CMAKE_CXX_STANDARD ${_NPL_SAVED_CXX_STANDARD})
            set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${_NPL_SAVED_RUNTIME_OUT})
            set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${_NPL_SAVED_LIBRARY_OUT})
            set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${_NPL_SAVED_ARCHIVE_OUT})
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
            add_library(onnxruntime::onnxruntime UNKNOWN IMPORTED GLOBAL)
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

            if(MSVC)
                get_filename_component(_ORT_VC_ROOT "${CMAKE_CXX_COMPILER}" DIRECTORY)
                get_filename_component(_ORT_VC_ROOT "${_ORT_VC_ROOT}/../../../../../.." ABSOLUTE)
                set(_ORT_TOOLSET_FILE "${_ORT_VC_ROOT}/Auxiliary/Build/Microsoft.VCToolsVersion.v143.default.txt")
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
            endif()

            if(WIN32)
                set(_ORT_CONFIG_DIR "${_ORT_BUILD_ROOT}/$<CONFIG>/$<CONFIG>")
                set(_ORT_POSTFIX "$<$<CONFIG:Debug>:_d>")
                set(_ORT_BYPRODUCTS
                    "${_ORT_CONFIG_DIR}/onnxruntime${_ORT_POSTFIX}.dll"
                    "${_ORT_CONFIG_DIR}/onnxruntime${_ORT_POSTFIX}.lib"
                )
            elseif(APPLE)
                set(_ORT_BYPRODUCTS "${_ORT_BUILD_ROOT}/$<CONFIG>/libonnxruntime.dylib")
            else()
                set(_ORT_BYPRODUCTS "${_ORT_BUILD_ROOT}/$<CONFIG>/libonnxruntime.so")
            endif()

            ExternalProject_Add(onnxruntime_external
                SOURCE_DIR "${ONNXRUNTIME_SOURCE_ROOT}"
                BINARY_DIR "${_ORT_BUILD_ROOT}"
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
                    --build_shared_lib
                    --cmake_extra_defines onnxruntime_BUILD_UNIT_TESTS=OFF CMAKE_DEBUG_POSTFIX=_d
                    --cmake_path "${CMAKE_COMMAND}"
                    ${_ORT_EXTRA_ARGS}
                INSTALL_COMMAND ""
                BUILD_BYPRODUCTS ${_ORT_BYPRODUCTS}
                USES_TERMINAL_BUILD TRUE
            )

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
                    IMPORTED_LOCATION "${_ORT_BUILD_ROOT}/$<CONFIG>/libonnxruntime.dylib"
                    INTERFACE_INCLUDE_DIRECTORIES "${_ORT_INC_DIR}"
                )
            else()
                set_target_properties(onnxruntime::onnxruntime PROPERTIES
                    IMPORTED_LOCATION "${_ORT_BUILD_ROOT}/$<CONFIG>/libonnxruntime.so"
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
