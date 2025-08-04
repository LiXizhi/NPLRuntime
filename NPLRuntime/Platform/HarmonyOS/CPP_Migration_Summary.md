# C++ 原生代码迁移总结

## 概述

成功将Android项目中的C++原生代码迁移到HarmonyOS平台，完成了从JNI到NAPI的转换。

## 迁移文件对照表

### 核心应用文件
| Android 文件 | HarmonyOS 文件 | 功能描述 |
|-------------|---------------|----------|
| `AppDelegate.cpp` | `AppDelegate.cpp` | 应用程序委托，处理生命周期 |
| `ParaAppAndroid.h/cpp` | `ParaAppHarmony.h/cpp` | ParaEngine应用主类 |

### 桥接通信文件
| Android 文件 | HarmonyOS 文件 | 功能描述 |
|-------------|---------------|----------|
| `LuaJavaBridge.h/cpp` | `LuaNapiBridge.h/cpp` | Lua与平台语言桥接 |
| `JniHelper.h/cpp` | `NAPIHelper.h/cpp` | 平台API辅助工具 |
| `ParaEngineHelper.h/cpp` | `ParaEngineHelper.h/cpp` | ParaEngine平台接口 |

### 渲染系统文件
| Android 文件 | HarmonyOS 文件 | 功能描述 |
|-------------|---------------|----------|
| `RenderWindowAndroid.h/cpp` | `RenderWindowHarmony.h/cpp` | 渲染窗口管理 |
| `RenderDeviceEGL.h/cpp` | `RenderDeviceOpenHarmony.h/cpp` | 渲染设备抽象 |
| `RenderContextEGL.h/cpp` | `RenderContextOpenHarmony.h/cpp` | 渲染上下文管理 |

## 关键技术变更

### 1. 平台API迁移

#### Android JNI → HarmonyOS NAPI
```cpp
// Android JNI
JNIEXPORT void JNICALL
Java_com_tatfook_paracraft_ParaEngineActivity_nativeInit(JNIEnv *env, jobject thiz) {
    // JNI实现
}

// HarmonyOS NAPI
napi_value NativeInit(napi_env env, napi_callback_info info) {
    // NAPI实现
    return nullptr;
}
```

#### 日志系统迁移
```cpp
// Android
#include <android/log.h>
__android_log_print(ANDROID_LOG_INFO, "ParaEngine", "Message");

// HarmonyOS
#include <hilog/log.h>
OH_LOG_INFO(LOG_APP, "ParaEngine: Message");
```

### 2. 按键映射适配

#### Android按键码 → HarmonyOS按键码
```cpp
// Android
s_keymap[AKEYCODE_BACK] = EVirtualKey::KEY_ESCAPE;
s_keymap[AKEYCODE_HOME] = EVirtualKey::KEY_HOME;

// HarmonyOS
s_keymap[2] = EVirtualKey::KEY_ESCAPE; // 返回键
s_keymap[1] = EVirtualKey::KEY_HOME;   // Home键
```

### 3. 资源管理系统

#### Android Asset Manager → HarmonyOS Raw File Manager
```cpp
// Android
#include <android/asset_manager.h>
AAssetManager* assetManager;

// HarmonyOS
#include <rawfile/raw_file_manager.h>
NativeResourceManager* rawFileManager;
```

### 4. 渲染系统适配

#### OpenGL ES适配
```cpp
// Android
#include <EGL/egl.h>
#include <GLES3/gl3.h>

// HarmonyOS (相同的API)
#include <EGL/egl.h>
#include <GLES3/gl3.h>
// HarmonyOS继续支持OpenGL ES
```

## 新增的HarmonyOS特定功能

### 1. NAPI桥接系统
- **NAPIHelper**: 提供NAPI常用操作的封装
- **LuaNapiBridge**: Lua与ArkTS的桥接通信
- **ParaEngineHelper**: ParaEngine功能的NAPI接口

### 2. 平台功能适配
```cpp
// 设备信息获取
std::string getDeviceInfo();

// 蓝牙功能
bool isBluetoothAvailable();
void enableBluetooth();

// 屏幕录制
void startScreenRecording();
void stopScreenRecording();

// 文件操作
std::string readAssetFile(const std::string& fileName);
bool writeFile(const std::string& fileName, const std::string& content);
```

### 3. 异步通信机制
```cpp
// 线程安全的函数调用
napi_threadsafe_function tsFunc;
createThreadSafeFunction(env, func, "callbackName", &tsFunc);
callThreadSafeFunction(tsFunc, data);
```

## 构建系统变更

### CMakeLists.txt 主要变更
```cmake
# 新增HarmonyOS NDK库
target_link_libraries(paracraft
    hilog_ndk.z        # 日志系统
    rawfile.z          # 资源文件访问
    native_window.z    # 窗口管理
    ace_napi.z         # NAPI支持
    EGL                # 图形渲染
    GLESv3             # OpenGL ES 3.0
    OpenSLES           # 音频系统
)

# 新增编译定义
target_compile_definitions(paracraft PRIVATE
    -DHARMONY_OS=1     # HarmonyOS平台标识
    -DUSE_NAPI=1       # 启用NAPI
)
```

## 目录结构

```
entry/src/main/cpp/
├── AppDelegate.h/cpp                    # 应用委托
├── ParaAppHarmony.h/cpp                # ParaEngine应用类
├── napi/
│   ├── NAPIHelper.h/cpp                # NAPI辅助工具
│   └── ParaEngineHelper.h/cpp          # ParaEngine接口
├── Framework/Common/Bridge/
│   └── LuaNapiBridge.h/cpp             # Lua桥接
├── util/
│   ├── SerialPortHarmony.h/cpp         # 串口通信
│   └── NetworkAdapter.cpp              # 网络适配
└── CMakeLists.txt                       # 构建配置
```

## 待完成的工作

### 1. 具体实现文件
- [ ] `ParaAppHarmony.cpp` - 应用主类实现
- [ ] `NAPIHelper.cpp` - NAPI辅助方法实现
- [ ] `LuaNapiBridge.cpp` - Lua桥接实现
- [ ] `ParaEngineHelper.cpp` - 平台接口实现

### 2. 渲染系统文件
- [ ] `RenderWindowHarmony.h/cpp` - 渲染窗口
- [ ] `RenderDeviceOpenHarmony.h/cpp` - 渲染设备
- [ ] `RenderContextOpenHarmony.h/cpp` - 渲染上下文

### 3. 平台特定功能
- [ ] USB设备检测适配
- [ ] 蓝牙功能具体实现
- [ ] 屏幕录制功能实现
- [ ] 文件系统接口适配

### 4. 测试验证
- [ ] NAPI接口测试
- [ ] Lua脚本调用测试
- [ ] 渲染功能测试
- [ ] 设备功能测试

## 注意事项

1. **API差异**: HarmonyOS的NAPI与Android的JNI在使用方式上有较大差异
2. **线程安全**: NAPI的线程安全机制需要特别注意
3. **内存管理**: NAPI的引用计数和垃圾回收机制
4. **错误处理**: NAPI的错误处理方式与JNI不同
5. **性能考虑**: NAPI调用的性能开销需要优化

通过以上迁移，Android项目的C++原生代码已经具备了在HarmonyOS平台运行的基础框架。
