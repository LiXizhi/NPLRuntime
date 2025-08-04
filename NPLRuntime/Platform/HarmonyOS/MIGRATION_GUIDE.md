# Android 到 HarmonyOS 迁移指南

本文档详细说明了从Android项目迁移到HarmonyOS 5.0的完整过程和注意事项。

## 1. 项目结构对比

### Android 项目结构
```
AndroidStudio/
├── app/
│   ├── java/com/tatfook/paracraft/
│   ├── res/
│   ├── AndroidManifest.xml
│   └── build.gradle
├── paracraft-core/
├── build.gradle
└── settings.gradle
```

### HarmonyOS 项目结构
```
HarmonyOS/
├── entry/
│   ├── src/main/ets/
│   ├── src/main/resources/
│   ├── module.json5
│   └── build-profile.json5
├── paracraft_core/
├── AppScope/
├── build-profile.json5
└── hvigorfile.ts
```

## 2. 核心文件迁移对照表

| Android | HarmonyOS | 说明 |
|---------|-----------|------|
| `AndroidManifest.xml` | `module.json5` | 应用配置文件 |
| `build.gradle` | `build-profile.json5` | 构建配置 |
| `Activity.java` | `UIAbility.ets` | 页面组件 |
| `Service.java` | `ServiceExtensionAbility.ets` | 服务组件 |
| `Application.java` | - | HarmonyOS无需Application类 |

## 3. 代码迁移映射

### 3.1 Application类迁移
**Android (ParaEngineApplication.java)**
```java
public class ParaEngineApplication extends Application {
    @Override
    public void onCreate() {
        super.onCreate();
        this.initPluginWrapper();
    }
}
```

**HarmonyOS (EntryAbility.ets)**
```typescript
export default class EntryAbility extends UIAbility {
  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    // 初始化逻辑
  }
}
```

### 3.2 Activity迁移
**Android**
- `ParaEngineActivity` -> HarmonyOS `EntryAbility`
- `ParaEngineEditBoxActivity` -> HarmonyOS `@Component`

### 3.3 核心Bridge类迁移
**Android (ParaEngineLuaJavaBridge.java)**
```java
public class ParaEngineLuaJavaBridge {
    public static void callLuaFunction(String funcName, String params) {
        // JNI调用
    }
}
```

**HarmonyOS (ParaEngineLuaJavaBridge.ets)**
```typescript
export class ParaEngineLuaJavaBridge {
  static callLuaFunction(funcName: string, params: string): void {
    // NAPI调用
  }
}
```

## 4. 权限迁移

### Android权限 -> HarmonyOS权限
| Android | HarmonyOS |
|---------|-----------|
| `android.permission.INTERNET` | `ohos.permission.INTERNET` |
| `android.permission.WRITE_EXTERNAL_STORAGE` | `ohos.permission.WRITE_USER_STORAGE` |
| `android.permission.READ_EXTERNAL_STORAGE` | `ohos.permission.READ_USER_STORAGE` |
| `android.permission.CAMERA` | `ohos.permission.CAMERA` |
| `android.permission.RECORD_AUDIO` | `ohos.permission.MICROPHONE` |
| `android.permission.ACCESS_FINE_LOCATION` | `ohos.permission.LOCATION` |

## 5. 资源文件迁移

### 5.1 字符串资源
**Android (strings.xml)**
```xml
<string name="app_name">帕拉卡3D虚拟社区学习软件</string>
```

**HarmonyOS (string.json)**
```json
{
  "string": [
    {
      "name": "app_name",
      "value": "帕拉卡3D虚拟社区学习软件"
    }
  ]
}
```

### 5.2 布局资源
Android的XML布局需要转换为HarmonyOS的ArkTS声明式UI。

## 6. 原生代码迁移

### 6.1 JNI -> NAPI
**Android JNI**
```cpp
JNIEXPORT void JNICALL
Java_com_tatfook_paracraft_ParaEngineActivity_nativeInit(JNIEnv *env, jobject thiz) {
    // 原生代码
}
```

**HarmonyOS NAPI**
```cpp
static napi_value NativeInit(napi_env env, napi_callback_info info) {
    // 原生代码
    return nullptr;
}
```

### 6.2 CMakeLists.txt适配
- Android NDK -> HarmonyOS NDK
- 链接库从Android库改为HarmonyOS库

## 7. 产品变体配置

### Android Flavors
```gradle
productFlavors {
    tatfook {
        applicationId "com.tatfook.paracraft"
    }
    palaka {
        applicationId "com.palaka.paracraft"
    }
}
```

### HarmonyOS Products
```json5
{
  "app": {
    "products": [
      {
        "name": "tatfook",
        "bundleName": "com.tatfook.paracraft"
      },
      {
        "name": "palaka", 
        "bundleName": "com.palaka.paracraft"
      }
    ]
  }
}
```

## 8. 构建脚本迁移

### Android
```bash
./gradlew assembleDebug
./gradlew assembleRelease
```

### HarmonyOS
```bash
./build.sh build debug
./build.sh build release
```

## 9. 测试与调试

### 9.1 设备连接
- Android: `adb devices`
- HarmonyOS: `hdc devices`

### 9.2 日志查看
- Android: `adb logcat`
- HarmonyOS: `hdc hilog`

## 10. 发布流程

### Android
1. 生成signed APK
2. 上传到Google Play/各大应用商店

### HarmonyOS
1. 生成signed HAP
2. 上传到华为应用市场

## 11. 注意事项

### 11.1 API差异
- HarmonyOS的API与Android有较大差异
- 某些Android特有功能在HarmonyOS中没有对应实现
- 需要重新设计部分功能模块

### 11.2 UI适配
- HarmonyOS使用声明式UI框架
- 需要重写所有UI组件
- 适配不同设备尺寸和分辨率

### 11.3 性能优化
- HarmonyOS的渲染机制与Android不同
- 需要针对HarmonyOS进行性能优化
- 关注内存和CPU使用情况

## 12. 开发工具

### 推荐使用
- **DevEco Studio**: HarmonyOS官方IDE
- **HarmonyOS SDK**: 开发工具包
- **hdc**: 设备连接工具

### 调试工具
- **hilog**: 日志查看
- **profiler**: 性能分析
- **inspector**: UI调试

## 13. 学习资源

- [HarmonyOS官方文档](https://developer.harmonyos.com/)
- [ArkTS开发指南](https://developer.harmonyos.com/cn/docs/documentation/doc-guides/arkts-get-started-0000001504769321)
- [HarmonyOS迁移指南](https://developer.harmonyos.com/cn/docs/documentation/doc-guides/migration-guide-0000001430013392)

---

通过以上迁移指南，可以将现有的Android项目成功迁移到HarmonyOS平台。迁移过程中需要注意API差异和平台特性，确保应用在HarmonyOS上的功能完整性和性能表现。
