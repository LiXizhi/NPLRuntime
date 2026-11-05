include(ExternalProject)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

if(NOT EXISTS "${ONNXRUNTIME_SOURCE_ROOT}/tools/ci_build/build.py")
	message(FATAL_ERROR "ONNX Runtime build driver not found at ${ONNXRUNTIME_SOURCE_ROOT}")
endif()

set(_ONNXRUNTIME_INCLUDE_DIR "${ONNXRUNTIME_SOURCE_ROOT}/include/onnxruntime/core/session")
if(NOT EXISTS "${_ONNXRUNTIME_INCLUDE_DIR}/onnxruntime_cxx_api.h")
	message(FATAL_ERROR "ONNX Runtime public headers not found at ${_ONNXRUNTIME_INCLUDE_DIR}")
endif()

get_filename_component(_ONNXRUNTIME_VC_ROOT "${CMAKE_CXX_COMPILER}" DIRECTORY)
get_filename_component(_ONNXRUNTIME_VC_ROOT "${_ONNXRUNTIME_VC_ROOT}/../../../../../.." ABSOLUTE)
set(_ONNXRUNTIME_TOOLSET_VERSION_FILE "${_ONNXRUNTIME_VC_ROOT}/Auxiliary/Build/Microsoft.VCToolsVersion.v143.default.txt")
if(NOT EXISTS "${_ONNXRUNTIME_TOOLSET_VERSION_FILE}")
	message(FATAL_ERROR "Visual Studio v143 toolset metadata not found at ${_ONNXRUNTIME_TOOLSET_VERSION_FILE}")
endif()
file(STRINGS "${_ONNXRUNTIME_TOOLSET_VERSION_FILE}" _ONNXRUNTIME_TOOLSET_VERSION LIMIT_COUNT 1)

set(_ONNXRUNTIME_GENERATOR "Visual Studio 17 2022")

set(_ONNXRUNTIME_BUILD_ROOT "${PROJECT_BINARY_DIR}/onnxruntime-external")
set(_ONNXRUNTIME_CONFIG_DIR "${_ONNXRUNTIME_BUILD_ROOT}/$<CONFIG>/$<CONFIG>")
set(_ONNXRUNTIME_POSTFIX "$<$<CONFIG:Debug>:_d>")
set(_ONNXRUNTIME_DLL "${_ONNXRUNTIME_CONFIG_DIR}/onnxruntime${_ONNXRUNTIME_POSTFIX}.dll")
set(_ONNXRUNTIME_LIB "${_ONNXRUNTIME_CONFIG_DIR}/onnxruntime${_ONNXRUNTIME_POSTFIX}.lib")

ExternalProject_Add(onnxruntime_external
	SOURCE_DIR "${ONNXRUNTIME_SOURCE_ROOT}"
	BINARY_DIR "${_ONNXRUNTIME_BUILD_ROOT}"
	CONFIGURE_COMMAND ""
	BUILD_COMMAND
		"${Python3_EXECUTABLE}" "${ONNXRUNTIME_SOURCE_ROOT}/tools/ci_build/build.py"
		--build_dir "${_ONNXRUNTIME_BUILD_ROOT}"
		--config $<CONFIG>
		--update
		--build
		--parallel
		--skip_tests
		--skip_submodule_sync
		--build_shared_lib
		--enable_msvc_static_runtime
		--cmake_extra_defines onnxruntime_BUILD_UNIT_TESTS=OFF CMAKE_DEBUG_POSTFIX=_d
		--cmake_path "${CMAKE_COMMAND}"
		--cmake_generator "${_ONNXRUNTIME_GENERATOR}"
		--msvc_toolset "${_ONNXRUNTIME_TOOLSET_VERSION}"
	INSTALL_COMMAND ""
	BUILD_ALWAYS TRUE
	BUILD_BYPRODUCTS "${_ONNXRUNTIME_DLL}" "${_ONNXRUNTIME_LIB}"
	USES_TERMINAL_BUILD TRUE
)

add_library(onnxruntime::onnxruntime SHARED IMPORTED GLOBAL)
set_target_properties(onnxruntime::onnxruntime PROPERTIES
	IMPORTED_CONFIGURATIONS "Debug;Release;RelWithDebInfo;MinSizeRel"
	IMPORTED_IMPLIB_DEBUG "${_ONNXRUNTIME_BUILD_ROOT}/Debug/Debug/onnxruntime_d.lib"
	IMPORTED_LOCATION_DEBUG "${_ONNXRUNTIME_BUILD_ROOT}/Debug/Debug/onnxruntime_d.dll"
	IMPORTED_IMPLIB_RELEASE "${_ONNXRUNTIME_BUILD_ROOT}/Release/Release/onnxruntime.lib"
	IMPORTED_LOCATION_RELEASE "${_ONNXRUNTIME_BUILD_ROOT}/Release/Release/onnxruntime.dll"
	IMPORTED_IMPLIB_RELWITHDEBINFO "${_ONNXRUNTIME_BUILD_ROOT}/RelWithDebInfo/RelWithDebInfo/onnxruntime.lib"
	IMPORTED_LOCATION_RELWITHDEBINFO "${_ONNXRUNTIME_BUILD_ROOT}/RelWithDebInfo/RelWithDebInfo/onnxruntime.dll"
	IMPORTED_IMPLIB_MINSIZEREL "${_ONNXRUNTIME_BUILD_ROOT}/MinSizeRel/MinSizeRel/onnxruntime.lib"
	IMPORTED_LOCATION_MINSIZEREL "${_ONNXRUNTIME_BUILD_ROOT}/MinSizeRel/MinSizeRel/onnxruntime.dll"
	INTERFACE_INCLUDE_DIRECTORIES "${_ONNXRUNTIME_INCLUDE_DIR}"
)
add_dependencies(onnxruntime::onnxruntime onnxruntime_external)