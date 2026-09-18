cmake_minimum_required(VERSION 3.16)
project(NPLRuntimeMuJoCoSmokeTest LANGUAGES C CXX)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
	message(FATAL_ERROR "MuJoCo smoke tests require a 64-bit build")
endif()

set(MUJOCO_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(MUJOCO_BUILD_SIMULATE OFF CACHE BOOL "" FORCE)
set(MUJOCO_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MUJOCO_TEST_PYTHON_UTIL OFF CACHE BOOL "" FORCE)

add_subdirectory(../../Server/trunk/mujoco-3.10.0 mujoco)

add_executable(MuJoCoSimulationSmokeTest
	MuJoCoSimulationSmokeTest.cpp
	../../Client/trunk/ParaEngineClient/ParaScriptBindings/MuJoCoSimulation.cpp
	../../Client/trunk/ParaEngineClient/robotics/RobotCoordinateConverter.cpp
	../../Client/trunk/ParaEngineClient/robotics/MujocoRobotSimulation.cpp
)
target_compile_definitions(MuJoCoSimulationSmokeTest PRIVATE USE_MUJOCO)
target_include_directories(MuJoCoSimulationSmokeTest PRIVATE
	../../Client/trunk/ParaEngineClient/ParaScriptBindings
	../../Client/trunk/ParaEngineClient/robotics
)
target_link_libraries(MuJoCoSimulationSmokeTest PRIVATE mujoco::mujoco)
target_compile_features(MuJoCoSimulationSmokeTest PRIVATE cxx_std_20)
add_custom_command(
	TARGET MuJoCoSimulationSmokeTest
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
		$<TARGET_FILE:mujoco>
		$<TARGET_FILE_DIR:MuJoCoSimulationSmokeTest>
)

enable_testing()
add_test(
	NAME MuJoCoSimulationSmokeTest
	COMMAND MuJoCoSimulationSmokeTest ${CMAKE_CURRENT_SOURCE_DIR}/minimal_hinge.xml
)