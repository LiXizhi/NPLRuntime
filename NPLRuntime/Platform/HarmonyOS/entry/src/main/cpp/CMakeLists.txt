# HarmonyOS Native C++ Configuration
# 对应Android项目的CMakeLists.txt

cmake_minimum_required(VERSION 3.4.1)
project(ParaCraftNative)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加编译参数
add_compile_options(-frtti -fexceptions -fsigned-char)

# 设置源文件目录
set(NPLRUNTIME_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../)
set(HARMONY_CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 包含头文件目录
include_directories(
    ${NPLRUNTIME_ROOT}
    ${NPLRUNTIME_ROOT}/Client/trunk/ParaEngineClient
    ${NPLRUNTIME_ROOT}/Server/trunk/NPLRuntime
    ${NPLRUNTIME_ROOT}/externals/luabind
    ${HARMONY_CPP_DIR}
    ${HARMONY_CPP_DIR}/napi
    ${HARMONY_CPP_DIR}/Framework/Common/Bridge
    ${HARMONY_CPP_DIR}/lua
)

# 添加HarmonyOS特定的源文件
set(HARMONY_SOURCES
    ${HARMONY_CPP_DIR}/AppDelegate.cpp
    ${HARMONY_CPP_DIR}/napi/NAPIHelper.cpp
    ${HARMONY_CPP_DIR}/napi/napi_main.cpp
    ${HARMONY_CPP_DIR}/napi/NAPI_Functions.cpp
    ${HARMONY_CPP_DIR}/Framework/Common/ParaEngineHelper.cpp
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

# 合并所有源文件
set(ALL_SOURCES ${HARMONY_SOURCES} ${PARAENGINE_SOURCES})

# 创建共享库
add_library(paracraft SHARED ${ALL_SOURCES})

# 链接HarmonyOS NDK库
target_link_libraries(paracraft
    hilog_ndk.z
    rawfile.z
    # native_window.z  # 暂时注释掉，库不存在
    EGL
    GLESv3
    OpenSLES
    ace_napi.z
)

# 设置编译定义
target_compile_definitions(paracraft PRIVATE
    -DHARMONY_OS=1
    -DNPLRUNTIME_OCE=1
    -DHB_HAVE_FREETYPE=ON
    -DUSE_NAPI=1
)
