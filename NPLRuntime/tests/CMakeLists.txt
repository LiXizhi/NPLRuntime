# Node has no IndexedDB. Isolate this test in a directory scope so the engine
# retains its browser preload cache while the smoke executable reads local data.
string(REPLACE "--use-preload-cache" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REPLACE "--use-preload-cache" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
add_executable(ParaRobotWasmSmoke
    WebRobotSmoke.cpp
    ../ParaEngine/ParaScriptBindings/MuJoCoSimulation.cpp
    ../ParaEngine/ParaScriptBindings/ONNXPolicySession.cpp
    ../ParaEngine/robotics/RobotCoordinateConverter.cpp)
target_include_directories(ParaRobotWasmSmoke PRIVATE
    ../ParaEngine/ParaScriptBindings ../ParaEngine/robotics)
target_compile_definitions(ParaRobotWasmSmoke PRIVATE USE_MUJOCO USE_ONNXRUNTIME)
target_link_libraries(ParaRobotWasmSmoke PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,mujoco>" onnxruntime_web)
target_link_options(ParaRobotWasmSmoke PRIVATE -msimd128 -sALLOW_MEMORY_GROWTH=1 -sSTACK_SIZE=16000000
    "SHELL:--preload-file ${ROBOT_TEST_ASSET_ROOT}@/microduck")
set_target_properties(ParaRobotWasmSmoke PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED YES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/robot-tests" SUFFIX ".js")
