# Keep ONNX in its own CMake project: the engine sets global SDL/PNG/link flags
# that must not leak into dependency feature probes or compilation.
set(ONNXRUNTIME_WASM_LIBRARY "" CACHE FILEPATH "Optional prebuilt ONNX Runtime WASM static library; empty builds from source")
set(_robot_onnx_source "${ROBOT_DEPENDENCY_ROOT}/onnxruntime-1.27.1")
if(ONNXRUNTIME_WASM_LIBRARY)
    if(NOT EXISTS "${ONNXRUNTIME_WASM_LIBRARY}")
        message(FATAL_ERROR "ONNXRUNTIME_WASM_LIBRARY does not exist: ${ONNXRUNTIME_WASM_LIBRARY}")
    endif()
    set(_robot_onnx_library "${ONNXRUNTIME_WASM_LIBRARY}")
else()
    if(NOT CMAKE_TOOLCHAIN_FILE OR NOT EXISTS "${CMAKE_TOOLCHAIN_FILE}")
        message(FATAL_ERROR "Configure Web with emcmake so ONNX can reuse the Emscripten toolchain")
    endif()
    include(ExternalProject)
    set(NPLRUNTIME_WEB_ROBOT_BUILD_JOBS 4 CACHE STRING "Parallel jobs for the automatic ONNX WASM build")
    if(NOT NPLRUNTIME_WEB_ROBOT_BUILD_JOBS MATCHES "^[1-9][0-9]*$")
        message(FATAL_ERROR "NPLRUNTIME_WEB_ROBOT_BUILD_JOBS must be a positive integer")
    endif()
    set(_robot_onnx_build "${CMAKE_BINARY_DIR}/robot-onnx")
    set(_robot_onnx_library "${_robot_onnx_build}/libonnxruntime_webassembly.a")
    ExternalProject_Add(onnxruntime_web_build
        SOURCE_DIR "${_robot_onnx_source}/cmake"
        BINARY_DIR "${_robot_onnx_build}"
        PREFIX "${CMAKE_BINARY_DIR}/robot-onnx-project"
        DOWNLOAD_COMMAND ""
        UPDATE_COMMAND ""
        CMAKE_ARGS
            "-DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE}"
            "-DCMAKE_MAKE_PROGRAM:FILEPATH=${CMAKE_MAKE_PROGRAM}"
            "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
            "-DCMAKE_C_FLAGS:STRING=-fexceptions"
            "-DCMAKE_CXX_FLAGS:STRING=-fexceptions"
            "-DCMAKE_C_FLAGS_DEBUG:STRING=-O2 -g1"
            "-DCMAKE_CXX_FLAGS_DEBUG:STRING=-O2 -g1"
            -Donnxruntime_BUILD_UNIT_TESTS=OFF
            -Donnxruntime_BUILD_SHARED_LIB=OFF
            -Donnxruntime_BUILD_WEBASSEMBLY_STATIC_LIB=ON
            -Donnxruntime_ENABLE_WEBASSEMBLY_THREADS=OFF
            -Donnxruntime_ENABLE_WEBASSEMBLY_SIMD=ON
            -Donnxruntime_ENABLE_WEBASSEMBLY_EXCEPTION_CATCHING=ON
            -Donnxruntime_DISABLE_CONTRIB_OPS=ON
            -Donnxruntime_DISABLE_ML_OPS=ON
            -Donnxruntime_ENABLE_PYTHON=OFF
            -Donnxruntime_USE_XNNPACK=OFF
            -Donnxruntime_USE_JSEP=OFF
            -Donnxruntime_USE_WEBGPU=OFF
            -Donnxruntime_USE_WEBNN=OFF
            -Donnxruntime_COMPILE_WARNING_AS_ERROR=OFF
        BUILD_COMMAND "${CMAKE_COMMAND}" --build <BINARY_DIR>
            --config $<CONFIG> --target bundling_target --parallel ${NPLRUNTIME_WEB_ROBOT_BUILD_JOBS}
        BUILD_BYPRODUCTS "${_robot_onnx_library}"
        BUILD_ALWAYS TRUE
        INSTALL_COMMAND "")
    message(STATUS "Web robot: building ONNX from this checkout in ${_robot_onnx_build}")
endif()

add_library(onnxruntime_web STATIC IMPORTED GLOBAL)
set_target_properties(onnxruntime_web PROPERTIES
    IMPORTED_LOCATION "${_robot_onnx_library}"
    INTERFACE_INCLUDE_DIRECTORIES "${_robot_onnx_source}/include/onnxruntime/core/session")
if(TARGET onnxruntime_web_build)
    add_dependencies(onnxruntime_web onnxruntime_web_build)
endif()
