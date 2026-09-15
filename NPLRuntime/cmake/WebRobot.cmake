if(NOT EMSCRIPTEN OR NOT EMSCRIPTEN_SINGLE_THREAD)
    message(FATAL_ERROR "Web robot integration currently requires single-thread Emscripten")
endif()
get_filename_component(_robot_branch_root "${CMAKE_CURRENT_LIST_DIR}/../../Server/trunk" REALPATH)
set(ROBOT_DEPENDENCY_ROOT "${_robot_branch_root}" CACHE PATH "Versioned dependency sources inside this WASM checkout")
get_filename_component(_robot_configured_root "${ROBOT_DEPENDENCY_ROOT}" REALPATH)
if(NOT _robot_configured_root STREQUAL _robot_branch_root)
    message(FATAL_ERROR "Robot C++ dependencies must be inside this WASM checkout: ${_robot_branch_root}")
endif()
set(ONNXRUNTIME_WASM_LIBRARY "" CACHE FILEPATH "Bundled ONNX Runtime WASM static library")
if(NOT EXISTS "${ROBOT_DEPENDENCY_ROOT}/mujoco-3.10.0/CMakeLists.txt" OR NOT EXISTS "${ONNXRUNTIME_WASM_LIBRARY}")
    message(FATAL_ERROR "Prepare the pinned MuJoCo/ONNX WASM dependencies with the robot CLI build")
endif()
set(MUJOCO_WASM_THREADS OFF CACHE BOOL "" FORCE)
set(MUJOCO_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MUJOCO_BUILD_TESTS_WASM OFF CACHE BOOL "" FORCE)
set(MUJOCO_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(MUJOCO_BUILD_SIMULATE OFF CACHE BOOL "" FORCE)
set(MUJOCO_BUILD_STUDIO OFF CACHE BOOL "" FORCE)
set(MUJOCO_USE_FILAMENT OFF CACHE BOOL "" FORCE)
add_subdirectory("${ROBOT_DEPENDENCY_ROOT}/mujoco-3.10.0" "${CMAKE_BINARY_DIR}/mujoco" EXCLUDE_FROM_ALL)
# cp_old puts Emscripten link settings in global compile flags; MuJoCo's
# -Werror must not turn those harmless driver diagnostics into errors.
target_compile_options(mujoco PRIVATE -Wno-unused-command-line-argument)
add_library(onnxruntime_web STATIC IMPORTED GLOBAL)
set_target_properties(onnxruntime_web PROPERTIES
    IMPORTED_LOCATION "${ONNXRUNTIME_WASM_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${ROBOT_DEPENDENCY_ROOT}/onnxruntime-1.27.1/include/onnxruntime/core/session")
set(ROBOT_TEST_ASSET_ROOT "" CACHE PATH "MicroDuck walking assets for the WASM smoke test")
if(EXISTS "${ROBOT_TEST_ASSET_ROOT}/mujoco/scene_ball.xml")
    add_executable(ParaRobotWasmSmoke
        "${CMAKE_CURRENT_LIST_DIR}/../tests/WebRobotSmoke.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/../ParaEngine/ParaScriptBindings/MuJoCoSimulation.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/../ParaEngine/ParaScriptBindings/ONNXPolicySession.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/../ParaEngine/robotics/RobotCoordinateConverter.cpp")
    target_include_directories(ParaRobotWasmSmoke PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../ParaEngine/ParaScriptBindings" "${CMAKE_CURRENT_LIST_DIR}/../ParaEngine/robotics")
    target_compile_definitions(ParaRobotWasmSmoke PRIVATE USE_MUJOCO USE_ONNXRUNTIME)
    target_link_libraries(ParaRobotWasmSmoke PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,mujoco>" onnxruntime_web)
    target_link_options(ParaRobotWasmSmoke PRIVATE -msimd128 -sALLOW_MEMORY_GROWTH=1 -sSTACK_SIZE=16000000
        "SHELL:--preload-file ${ROBOT_TEST_ASSET_ROOT}@/microduck")
    set_target_properties(ParaRobotWasmSmoke PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/robot-tests" SUFFIX ".js")
endif()
