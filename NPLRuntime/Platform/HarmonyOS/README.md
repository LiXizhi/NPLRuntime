# ParaCraft HarmonyOS Project

帕拉卡3D虚拟社区学习软件 HarmonyOS 版本

## 项目说明

这是从Android项目迁移到HarmonyOS 5.0的版本，保持了原有的功能特性：

- 3D虚拟社区学习平台
- 多渠道支持（塔夫斗、帕拉卡、教育版等）
- 原生C++引擎集成
- WebView组件支持
- 蓝牙和USB串口通信
- 音视频录制功能

## 项目结构

```
├── AppScope/                   # 应用级配置
├── entry/                      # 主模块
│   ├── src/main/
│   │   ├── ets/               # ArkTS源码
│   │   ├── cpp/               # Native C++代码
│   │   └── resources/         # 资源文件
│   └── build-profile.json5
├── paracraft_core/            # 核心共享模块
├── build-profile.json5        # 构建配置
├── hvigorfile.ts             # 构建脚本
└── package.json              # Node.js配置
```

## 开发环境

- HarmonyOS SDK API Level 12
- DevEco Studio 5.0+
- Node.js 16+

## 构建命令

```bash
# 安装依赖
npm install

# 调试构建
npm run build:debug

# 发布构建
npm run build:release

# 清理
npm run clean
```

## 主要迁移变更

### 1. 项目结构变更
- Android的`app`模块 -> HarmonyOS的`entry`模块
- Android的`paracraft-core`模块 -> HarmonyOS的`paracraft_core`模块
- `AndroidManifest.xml` -> `module.json5`
- Gradle配置 -> build-profile.json5

### 2. 核心组件迁移
- `ParaEngineActivity.java` -> `EntryAbility.ets` + `ParaEngineMain.ets`
- `ParaEngineEditBoxActivity.java` -> `ParaEngineEditBox.ets`
- `ParaEngineApplication.java` -> `EntryAbility.ets`
- `ParaEngineWebView.java` -> `ParaEngineWebView.ets`
- `ParaEngineLuaJavaBridge.java` -> `ParaEngineLuaJavaBridge.ets`
- `PlatformBridge.java` -> `PlatformBridge.ets`
- `ScreenRecorderService.java` -> `ScreenRecorderService.ets`

### 3. 代码迁移
- Java Activity -> ArkTS UIAbility
- Java Service -> ArkTS ServiceExtensionAbility
- Android View -> ArkTS Component
- JNI -> NAPI
- GLSurfaceView -> XComponent

### 4. 功能对应表
| Android组件 | HarmonyOS组件 | 功能说明 |
|------------|--------------|----------|
| `ParaEngineActivity` | `EntryAbility` + `ParaEngineMain` | 主Activity，包含3D渲染和生命周期管理 |
| `ParaEngineEditBoxActivity` | `ParaEngineEditBox` | 文本编辑页面 |
| `GLSurfaceView` | `XComponent` | 3D渲染组件 |
| `WebView` | `Web组件` | 网页显示组件 |
| `ScreenRecorderService` | `ScreenRecorderService` | 屏幕录制服务 |
| `BluetoothLeService` | `BluetoothManager` | 蓝牙管理 |

### 5. 权限变更
- `android.permission.*` -> `ohos.permission.*`
- 新增HarmonyOS特有权限配置
- 动态权限申请适配

### 4. 原生代码适配
- Android NDK -> HarmonyOS NDK
- OpenGL ES适配
- 音频系统适配

## 发布配置

支持多个产品风味：
- `tatfook`: 塔夫斗版本
- `palaka`: 帕拉卡版本  
- `papa`: 帕帕奇遇记版本
- `edu`: 教育版本

每个版本都有对应的：
- 应用名称
- 图标资源
- URL Scheme
- 渠道标识

## 注意事项

1. **签名配置**: 需要在`gradle.properties`中配置HarmonyOS签名信息
2. **权限申请**: 某些权限需要用户手动授权
3. **原生库适配**: C++代码需要针对HarmonyOS进行适配
4. **资源迁移**: 图片和布局资源需要按HarmonyOS规范重新整理

## 开发指南

1. 使用DevEco Studio打开项目
2. 配置HarmonyOS SDK
3. 连接HarmonyOS设备或模拟器
4. 点击Run按钮编译运行

## 联系方式

开发团队：塔夫斗科技
项目地址：https://github.com/NPLFoshan/NPLRuntime
