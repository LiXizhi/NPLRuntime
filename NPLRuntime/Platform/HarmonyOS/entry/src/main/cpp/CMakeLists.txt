# HarmonyOS Native C++ Configuration
# 对应Android项目的CMakeLists.txt
project(ParaCraftNative)
if(POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

# 为依赖插件的 POST_BUILD 拷贝提供输出目录
if(NOT DEFINED OUTPUT_BIN_DIR)
  set(OUTPUT_BIN_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
endif()

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加编译参数
add_compile_options(-frtti -fexceptions -fsigned-char)

# 设置源文件目录
set(NPLRUNTIME_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../)
set(HARMONY_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# ===== Boost 配置 =====
set(BOOST_ROOT ${NPLRUNTIME_ROOT}/externals/boost)
set(BOOST_PREBUILD_DIR ${BOOST_ROOT}/prebuild/harmonyos)

# 包含头文件目录
include_directories(
    ${NPLRUNTIME_ROOT}
    ${NPLRUNTIME_ROOT}/Client/trunk/ParaEngineClient
    ${NPLRUNTIME_ROOT}/Server/trunk/NPLRuntime
    ${NPLRUNTIME_ROOT}/ParaEngine
    ${NPLRUNTIME_ROOT}/ParaEngine/Core
    ${NPLRUNTIME_ROOT}/ParaEngine/3dengine
    ${NPLRUNTIME_ROOT}/ParaEngine/math
    ${NPLRUNTIME_ROOT}/ParaEngine/renderer
    ${NPLRUNTIME_ROOT}/ParaEngine/util
    ${NPLRUNTIME_ROOT}/ParaEngine/ParaScriptBindings
    ${NPLRUNTIME_ROOT}/ParaEngine/NPL
    ${BOOST_PREBUILD_DIR}/include
    ${HARMONY_CPP_DIR}
    ${HARMONY_CPP_DIR}/renderer
    ${HARMONY_CPP_DIR}/napi
    ${HARMONY_CPP_DIR}/Framework/Common/Bridge
    ${NPLRUNTIME_ROOT}/externals/LuaJIT2_1/include
    ${NPLRUNTIME_ROOT}/externals/luabind
    ${NPLRUNTIME_ROOT}/externals
    ${NPLRUNTIME_ROOT}/externals/freetype-2.8.1/include
    ${NPLRUNTIME_ROOT}/externals/dxEffectsParser/src
    ${NPLRUNTIME_ROOT}/externals/hlsl2glslfork/include
    ${NPLRUNTIME_ROOT}/externals/hlsl2glslfork/hlslang/GLSLCodeGen
    ${NPLRUNTIME_ROOT}/externals/hlsl2glslfork/hlslang
    ${NPLRUNTIME_ROOT}/externals/hlsl2glslfork/hlslang/MachineIndependent
    ${NPLRUNTIME_ROOT}/externals/hlsl2glslfork/hlslang/OSDependent/Unix
    ${NPLRUNTIME_ROOT}/externals/hlsl2glslfork/OGLCompilersDLL
)

# 添加HarmonyOS特定的源文件
file(GLOB_RECURSE HARMONY_SOURCES
    ${HARMONY_CPP_DIR}/*.cpp
)

# 添加ParaEngine核心源文件（排除Android特定文件）
file(GLOB_RECURSE PARAENGINE_SOURCES 
    "${NPLRUNTIME_ROOT}/Client/trunk/ParaEngineClient/*.cpp"
    "${NPLRUNTIME_ROOT}/Server/trunk/NPLRuntime/*.cpp"
)

# 排除Android特定的文件
list(FILTER PARAENGINE_SOURCES EXCLUDE REGEX ".*Android.*")
list(FILTER PARAENGINE_SOURCES EXCLUDE REGEX ".*JNI.*")
list(FILTER PARAENGINE_SOURCES EXCLUDE REGEX ".*jni.*")

# 合并所有源文件（仅平台源码）
set(ALL_SOURCES ${HARMONY_SOURCES})

# 创建共享库
add_library(paracraft SHARED ${ALL_SOURCES})

# ===== 确定 Boost 库目录（根据架构）=====
if(CMAKE_ANDROID_ARCH_ABI STREQUAL "arm64-v8a")
    set(BOOST_LIB_DIR ${BOOST_PREBUILD_DIR}/arm64-v8a)
elseif(CMAKE_ANDROID_ARCH_ABI STREQUAL "armeabi-v7a")
    set(BOOST_LIB_DIR ${BOOST_PREBUILD_DIR}/armeabi-v7a)
elseif(NOT DEFINED CMAKE_ANDROID_ARCH_ABI OR CMAKE_ANDROID_ARCH_ABI STREQUAL "")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|AARCH64)$")
        set(BOOST_LIB_DIR ${BOOST_PREBUILD_DIR}/arm64-v8a)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|ARM)$")
        set(BOOST_LIB_DIR ${BOOST_PREBUILD_DIR}/armeabi-v7a)
    endif()
endif()

if(NOT DEFINED BOOST_LIB_DIR)
    if(EXISTS ${BOOST_PREBUILD_DIR}/arm64-v8a)
        set(BOOST_LIB_DIR ${BOOST_PREBUILD_DIR}/arm64-v8a)
    elseif(EXISTS ${BOOST_PREBUILD_DIR}/armeabi-v7a)
        set(BOOST_LIB_DIR ${BOOST_PREBUILD_DIR}/armeabi-v7a)
    endif()
endif()

# 添加 Boost 库目录
if(DEFINED BOOST_LIB_DIR AND EXISTS ${BOOST_LIB_DIR})
    # 让目标在链接阶段显式使用 Boost 库搜索路径
    target_link_directories(paracraft PRIVATE ${BOOST_LIB_DIR})
    set(BOOST_LIB_NAMES thread date_time filesystem system chrono regex serialization iostreams log_setup wserialization)
    set(BOOST_LINK_LIBS)
    foreach(lib ${BOOST_LIB_NAMES})
        if(EXISTS ${BOOST_LIB_DIR}/libboost_${lib}.a OR EXISTS ${BOOST_LIB_DIR}/libboost_${lib}.so)
            list(APPEND BOOST_LINK_LIBS boost_${lib})
        endif()
    endforeach()
    if(BOOST_LINK_LIBS)
        message(STATUS "Linking Boost libs: ${BOOST_LINK_LIBS}")
    else()
        message(WARNING "No Boost libs found in ${BOOST_LIB_DIR}, skipping Boost link")
    endif()
else()
    message(WARNING "Boost libraries not found for architecture ${CMAKE_ANDROID_ARCH_ABI}. Please build Boost first.")
endif()

# 选择渲染器并引入核心工程
set(NPLRUNTIME_RENDERER OPENGL)
# 使用 LuaJIT 2.1
set(NPLRUNTIME_LUAJIT21 ON)

# 为 externals/curl 提供 ANDROID 分支兼容（仅限 externals）
if(DEFINED CMAKE_OHOS_ARCH_ABI AND NOT DEFINED CMAKE_ANDROID_ARCH_ABI)
    set(CMAKE_ANDROID_ARCH_ABI ${CMAKE_OHOS_ARCH_ABI})
endif()
set(PROJECT_SOURCE_DIR ${NPLRUNTIME_ROOT})
set(LINUX TRUE)

# 为 FindBoost 提供兼容目录结构（使用 HarmonyOS 预编译）
set(BOOST_COMPAT_ROOT ${CMAKE_BINARY_DIR}/boost_compat)
set(BOOST_COMPAT_LIBDIR ${BOOST_COMPAT_ROOT}/android-build/stage/${CMAKE_ANDROID_ARCH_ABI}/lib)
file(MAKE_DIRECTORY ${BOOST_COMPAT_ROOT})
file(MAKE_DIRECTORY ${BOOST_COMPAT_LIBDIR})
file(COPY ${BOOST_PREBUILD_DIR}/include/ DESTINATION ${BOOST_COMPAT_ROOT})
file(COPY ${BOOST_PREBUILD_DIR}/${CMAKE_ANDROID_ARCH_ABI}/ DESTINATION ${BOOST_COMPAT_LIBDIR})
set(BOOST_ROOT ${BOOST_COMPAT_ROOT})

# 设置 HARMONY_OS 标识供子目录使用
set(HARMONY_OS TRUE)
add_subdirectory(${NPLRUNTIME_ROOT}/externals ${CMAKE_CURRENT_BINARY_DIR}/externals)
add_subdirectory(${NPLRUNTIME_ROOT}/RenderSystem ${CMAKE_CURRENT_BINARY_DIR}/RenderSystem)
add_subdirectory(${NPLRUNTIME_ROOT}/Plugins/sqlite3 ${CMAKE_CURRENT_BINARY_DIR}/Plugins/sqlite3)
add_subdirectory(${NPLRUNTIME_ROOT}/ParaEngine ${CMAKE_CURRENT_BINARY_DIR}/ParaEngine)

# 通过链接 externals 的目标，让包含路径与依赖自动传播到 ParaEngine
if(TARGET ParaEngine)
  if(TARGET MachineIndependent)
    target_link_libraries(ParaEngine PUBLIC MachineIndependent)
  endif()
  if(TARGET GLSLCodeGen)
    target_link_libraries(ParaEngine PUBLIC GLSLCodeGen)
  endif()
  if(TARGET OSDependent)
    target_link_libraries(ParaEngine PUBLIC OSDependent)
  endif()
  if(TARGET OGLCompilersDLL)
    target_link_libraries(ParaEngine PUBLIC OGLCompilersDLL)
  endif()
  if(TARGET dxEffectsParser)
    target_link_libraries(ParaEngine PUBLIC dxEffectsParser)
  endif()
endif()

# 为 ParaEngine 目标补充 HarmonyOS 兼容宏，避免 Windows/DirectX 专有类型报错
if(TARGET ParaEngine)
    target_compile_definitions(ParaEngine PUBLIC
        HARMONY_OS=1
        EXCEPTION_POINTERS=void
        LPD3DXFILE=void*
        USE_OPENGL_RENDERER=1
        __int64=int64_t
        MAX_STRING_LENGTH=65535
    )
    set_target_properties(ParaEngine PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
endif()

# 链接核心库与HarmonyOS NDK库（参考Android）
target_link_libraries(paracraft PUBLIC
    RenderSystemOpenGL
    ParaEngine
    ParaSqlite
    EGL
    GLESv3
    OpenSLES
    ace_napi.z
    hilog_ndk.z
    rawfile.z
)

# 链接 Boost 库（如果已构建）
if(BOOST_LINK_LIBS)
    target_link_libraries(paracraft PUBLIC ${BOOST_LINK_LIBS})
endif()

# 设置编译定义
target_compile_definitions(paracraft PUBLIC
    -DHARMONY_OS=1
    -DNPLRUNTIME_OCE=1
    -DHB_HAVE_FREETYPE=ON
    -DUSE_NAPI=1
    -DUSE_OPENGL_RENDERER=1
    -DLINUX=1
    EXCEPTION_POINTERS=void
    PARAENGINE_MOBILE
    STATIC_PLUGIN_CAUDIOENGINE
)
target_include_directories(paracraft PRIVATE
    ${NPLRUNTIME_ROOT}/ParaEngine/util
)
# 全局兼容定义（影响子工程 ParaEngine 构建）
add_definitions(-DEXCEPTION_POINTERS=void)
add_definitions(-DLPD3DXFILE=void*)
